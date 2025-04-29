#include "framework.h"
#include "Activity.h"
#include "PPTAIHelper.h"
#include "PuyoyoyoAI.h"
#include "template.h"
#include "Windows.h"
#include "algorithm"
#include "deque"
#include "iostream"
#include "vector"
#include "chrono"
#include "fstream"
#include "sstream"
#include "string"
#include "future"
#include "utility"

#define BEAM_WIDTH 15
#define BEAM_DEPTH 3

#define SEARCH_WIDTH 3
#define MAXIMUM_DEPTH 2

extern HDC hMemDC;
extern HWND hNewWnd;
extern HDC overlayDC;
extern HWND overlayWnd;
extern WCHAR buf[256];
extern int color;
extern int place;
extern bool assist;

static PPTAIHelper PPT;
static int cursorX, cursorY;
static int nowPuyo[] = { 0,0 };
static int nextPuyo[] = { 0,0 };
static int nextNextPuyo[] = { 0,0 };
static int saveNext[] = { 0,0 };
static int saveNextNext[] = { 0,0 };
static int scene = 0;
static int puyoNum = 0;
static double Time = 0;
static unsigned __int64 Field[6] = { 0 };
static unsigned __int64 copyField[6] = { 0 };
static unsigned __int64 tmpField[6] = { 0 };
static unsigned __int64 tmpField1[6] = { 0 };
static unsigned __int64 tmpField2[6] = { 0 };
static int copyFloor[6] = { 0 };
static int tmpFloor[6] = { 0 };
static int tmpFloor1[6] = { 0 };
static int tmpFloor2[6] = { 0 };
static int fieldFloor[6] = { 0 };

static int num = 0;
static int conv[4] = { 0 };
static __int8 appearance = 0;

const __int8 colorBonus[5] = { 0,3,6,12,24 };
const __int8 linkBonus[8] = { 0,2,3,4,5,6,7,10 };
const __int8 idealHeight[6] = { 1, 0, 0, 0, 0, 1 };

static unsigned int frameCount = 0;


// 盤面のサイズ
const int ROWS = 12;
const int COLS = 6;

// 方向ベクトル（上下左右）
const int directions[4][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };

static BYTE R = 0;
std::deque<int> queue;//0-Left,1-Right,2-z,3-x
std::deque<int> dropQueue;
//std::vector<std::vector<std::vector<int>>> ScoreQueue;
static void ImportField();
static void MakeTemplate();
static void Search();

std::string WCHARArrayToString(const WCHAR* wideCharArray) {
    // 変換するためのサイズを取得する
    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wideCharArray, -1, nullptr, 0, nullptr, nullptr);
    if (sizeNeeded <= 0) {
        // エラー処理、必要に応じて例外をスローするなど
        return "";
    }

    // 変換先のバッファを確保する
    std::string multiByteString(sizeNeeded - 1, '\0'); // -1 は NULL 終端文字分

    // 実際に変換を行う
    WideCharToMultiByte(CP_UTF8, 0, wideCharArray, -1, &multiByteString[0], sizeNeeded, nullptr, nullptr);

    return multiByteString;
}

static void LogMessage(WCHAR* format, ...)
{
    // ログファイルのパス
    const std::string logFilePath = "application.log";

    // std::ofstreamを使用してファイルを開く（ファイルが存在しない場合は自動的に作成される）
    std::ofstream logFile(logFilePath, std::ios::app); // 追記モードで開く

    // ファイルが正常に開けたかチェック
    if (logFile.is_open())
    {
        va_list args;
        va_start(args, format);
        WCHAR buf[256];
        int len = _vsntprintf_s(buf, _ARRAYSIZE(buf), _TRUNCATE, format, args);
        va_end(args);

        std::string result = WCHARArrayToString(buf);

        // メッセージを書き込む
        logFile << result << std::endl;
        // ファイルを閉じる
        logFile.close();
    }
    else
    {
        std::cerr << "Failed to open log file: " << logFilePath << std::endl;
    }
}

class UnionFind {
public:
    UnionFind(int size) : parent(size), rank(size, 0) {
        for (int i = 0; i < size; ++i) parent[i] = i;
    }

    int find(int x) {
        if (parent[x] != x) parent[x] = find(parent[x]);
        return parent[x];
    }

    void unite(int x, int y) {
        int rootX = find(x);
        int rootY = find(y);
        if (rootX != rootY) {
            if (rank[rootX] > rank[rootY]) parent[rootY] = rootX;
            else if (rank[rootX] < rank[rootY]) parent[rootX] = rootY;
            else {
                parent[rootY] = rootX;
                ++rank[rootX];
            }
        }
    }

private:
    std::vector<int> parent;
    std::vector<int> rank;
};


// 座標を1次元インデックスに変換
static int toIndex(int r, int c) {
    return r * COLS + c;
}

// 盤面に連鎖があるかどうかを検出する関数
static bool detectChain(unsigned __int64* fieldPointer, std::deque<int>& dis, __int64* erase, __int8* eraseCount) {
    unsigned __int64 copyFi[6] = { 0 };

    for (int i = 0; i < 6; i++)
    {
        copyFi[i] = *(fieldPointer + i);
        *(erase + i) = 0;
        if (i != 5) *(eraseCount + i) = 0;
    }
    UnionFind uf(ROWS * COLS);
    std::vector<std::vector<bool>> visited(ROWS, std::vector<bool>(COLS, false));
    bool chainDetected = false;

    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            if (((copyFi[c] >> (r * 3)) & 0b111) != 0 && ((copyFi[c] >> (r * 3)) & 0b111) != 6) {
                for (const auto& dir : directions) {
                    int nr = r + dir[0];
                    int nc = c + dir[1];
                    if (nr >= 0 && nr < ROWS && nc >= 0 && nc < COLS)
                    {
                        int color = (copyFi[c] >> (r * 3)) & 0b111;
                        int newColor = (copyFi[nc] >> (nr * 3)) & 0b111;
                        if (color == newColor) {
                            uf.unite(toIndex(r, c), toIndex(nr, nc));
                        }
                    }
                }
            }
        }
    }

    // 連鎖のサイズを確認
    std::vector<int> componentSize(ROWS * COLS, 0);
    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            if (((copyFi[c] >> (r * 3)) & 0b111) != 0 && ((copyFi[c] >> (r * 3)) & 0b111) != 6) {
                int root = uf.find(toIndex(r, c));
                componentSize[root]++;
                if (componentSize[root] >= 4) {
                    chainDetected = true;
                    dis.push_back(root);
                    //break;
                }
            }
        }
        //if (chainDetected) break;
    }

    while (dis.size())
    {
        int root = dis[0];
        for (int nr = 0; nr < ROWS; nr++)
        {
            for (int nc = 0; nc < COLS; nc++)
            {
                if (uf.find(toIndex(nr, nc)) == root)
                {
                    int color = copyFi[nc] >> (nr * 3) & 0b111;
                    *(erase + nc) |= (long long)0b111 << (nr * 3);
                    if (color == 0 || color >= 6) break;
                    ++*(eraseCount + color - 1);

                }
            }
        }
        dis.erase(dis.begin());
    }

    return chainDetected;
}

void DrawString(HDC hdc, int x, int y, WCHAR* format, ...) {
    va_list args;
    va_start(args, format);
    WCHAR buf[256];
    int len = _vsntprintf_s(buf, _ARRAYSIZE(buf), _TRUNCATE, format, args);
    va_end(args);
    TextOut(hdc, x, y, buf, len);
}
static void newDrawString(WCHAR* format, ...) {
    va_list args;
    va_start(args, format);
    WCHAR hoge[256];
    int len = _vsntprintf_s(hoge, _ARRAYSIZE(hoge), _TRUNCATE, format, args);
    va_end(args);
    wcscpy_s(buf, sizeof(buf) / sizeof(WCHAR), hoge);
    InvalidateRect(hNewWnd, nullptr, FALSE);
}

/*
static void PressZKeyRepeatedly()
{
    static int mode = 0;
    if (mode == 0)
    {
        PPT.InputKey(VK_RIGHT, false, true);
        mode = 1;
    }
    else
    {
        PPT.InputKey(VK_RIGHT, false, false);
        mode = 0;
    }
}*/

static void fall(unsigned __int64* pointer)
{
    for (int i = 0; i < 6; i++)
    {
        unsigned __int64 row = *(pointer + i);
        unsigned __int64 tmp = 0;
        int num = 0;
        for (int j = 0; j < 13; j++)
        {
            if (*(pointer + i) >> (num * 3) == 0) break;
            tmp = 0;
            int color = (*(pointer + i) >> (j * 3)) & 0b111;
            if (!color)
            {
                tmp = row & (((unsigned __int64)0b1 << (num * 3 + 1)) - 1);
                row &= ~(((unsigned __int64)0b1 << (num * 3 + 1)) - 1);
                row >>= 3;
                row |= tmp;
                num--;
            }
            num++;
        }
        *(pointer + i) = row;
    }
}

static int ChainStart(unsigned __int64* pointer, int chain, std::deque<int>& dis, __int64* erase, char* eraseCount)
{
    if (!detectChain(pointer, dis, erase, eraseCount)) return 0;
    for (int i = 0; i < 6; i++)
    {
        *(pointer + i) &= ~*(erase + i);
    }

    int puyo = 0;
    int colB = 0;
    int linkB = 0;
    for (int i = 0; i < 5; i++)
    {
        puyo += *(eraseCount + i);
        if (*(eraseCount + i) != 0)
        {
            colB++;
            *(eraseCount + i) -= 4;
            if (*(eraseCount + i) >= 7) *(eraseCount + i) = 7;
            if (*(eraseCount + i) < 0) *(eraseCount + i) = 0;
            linkB += linkBonus[*(eraseCount + i)];
        }
    }
    if (colB != 0) colB--;
    colB = colorBonus[colB];
    int chainBonus = 0;
    if (chain != 0)
    {
        if (chain >= 1 && chain <= 5)
        {
            chainBonus = 0b100 << chain;
        }
        else
        {
            chainBonus = -64 + chain * 32;
        }
    }
    int bonus = chainBonus + colB + linkB;
    if (bonus == 0) bonus = 1;
    return puyo * bonus;
}

static void GetStarted(unsigned __int64* fieldPointer, int* floorPointer, __int8 childPuyo, __int8 shaftPuyo, int place)
{
    if (place < 12)
    {
        if (*(floorPointer + place) >= 12) return;
        if (place < 6)
        {
            *(fieldPointer + place) |= (unsigned __int64)shaftPuyo << *(floorPointer + place) * 3;
            *(floorPointer + place) += 1;
            //puyoNum++;
            if (*(floorPointer + place) >= 12) return;
            *(fieldPointer + place) |= (unsigned __int64)childPuyo << (*(floorPointer + place)) * 3;
            *(floorPointer + place) += 1;
            //puyoNum++;
        }
        else
        {
            place -= 6;
            *(fieldPointer + place) |= (unsigned __int64)childPuyo << *(floorPointer + place) * 3;
            *(floorPointer + place) += 1;
            //puyoNum++;
            if (*(floorPointer + place) >= 12) return;
            *(fieldPointer + place) |= (unsigned __int64)shaftPuyo << (*(floorPointer + place)) * 3;
            *(floorPointer + place) += 1;
            //puyoNum++;
        }
    }
    else
    {
        place -= 12;
        if (place < 5)
        {
            if (*(floorPointer + place) < 12)
            {
                *(fieldPointer + place) |= (unsigned __int64)childPuyo << *(floorPointer + place) * 3;
                *(floorPointer + place) += 1;
                //puyoNum++;
            }
            if (*(floorPointer + place + 1) < 12)
            {
                *(fieldPointer + place + 1) |= (unsigned __int64)shaftPuyo << *(floorPointer + place + 1) * 3;
                *(floorPointer + place + 1) += 1;
                //puyoNum++;
            }
        }
        else
        {
            place -= 5;
            if (*(floorPointer + place) < 12)
            {
                *(fieldPointer + place) |= (unsigned __int64)shaftPuyo << *(floorPointer + place) * 3;
                *(floorPointer + place) += 1;
                //puyoNum++;
            }
            if (*(floorPointer + place + 1) < 12)
            {
                *(fieldPointer + place + 1) |= (unsigned __int64)childPuyo << *(floorPointer + place + 1) * 3;
                *(floorPointer + place + 1) += 1;
                //puyoNum++;
            }
        }
    }
}
static void drop(int place)
{
    queue.clear();
    if (place < 12)
    {
        int num = place;
        if (num >= 6) num -= 6;
        switch (num)
        {
        case 0:
        {
            queue.push_back(0);
            if (place == num + 6)
            {
                queue.push_back(3);
                queue.push_back(3);
            }
            break;
        }
        case 1:
        {
            queue.push_back(4);
            if (place == num + 6)
            {
                queue.push_back(3);
                queue.push_back(3);
            }
            break;
        }
        case 2:
        {
            if (place == num + 6)
            {
                queue.push_back(2);
                queue.push_back(2);
            }
            break;
        }
        case 3:
        {
            queue.push_back(5);
            if (place == num + 6)
            {
                queue.push_back(2);
                queue.push_back(2);
            }
            break;
        }
        case 4:
        {
            queue.push_back(1);
            queue.push_back(3);
            if (place == num + 6)
            {
                queue.push_back(3);
            }
            else
            {
                queue.push_back(2);
            }
            break;
        }
        case 5:
        {
            queue.push_back(1);
            if (place == num + 6)
            {
                queue.push_back(2);
                queue.push_back(2);
            }
            break;
        }
        default:
            break;
        }
    }
    else
    {
        int num = place - 12;
        switch (num)
        {
        case 0:
        {
            queue.push_back(0);
            queue.push_back(2);
            break;
        }
        case 1:
        {
            queue.push_back(2);
            break;
        }
        case 2:
        {
            queue.push_back(5);
            queue.push_back(2);
            break;
        }
        case 3:
        {
            queue.push_back(1);
            queue.push_back(3);
            queue.push_back(2);
            queue.push_back(2);
            break;
        }
        case 4:
        {
            queue.push_back(1);
            queue.push_back(2);
            break;
        }
        case 5:
        {
            queue.push_back(0);
            queue.push_back(3);
            break;
        }
        case 6:
        {
            queue.push_back(4);
            queue.push_back(3);
            break;
        }
        case 7:
        {
            queue.push_back(3);
            break;
        }
        case 8:
        {
            queue.push_back(5);
            queue.push_back(3);
            break;
        }
        case 9:
        {
            queue.push_back(1);
            queue.push_back(3);
            break;
        }
        default:
            break;
        }
    }
}

static void DrawRGBAt(int x, int y)
{
    BYTE r, g, b;
    PPT.GetPixelColor(x, y, r, g, b);
    WCHAR text[] = L"(%d, %d) : R=%02x G=%02x B=%02x";
    DrawString(hMemDC, x, y, text, x, y, r, g, b);
}

static void Operations()
{
    static int puyoMode = 0;
    static int frame = 0;
    if (puyoMode)
    {
        int action = queue.front(); // 前方要素を取得
        queue.erase(queue.begin());

        switch (action)
        {
        case 0:
            PPT.InputKey(VK_LEFT, true, true);
            frame = 1;
            break;
        case 1:
            PPT.InputKey(VK_RIGHT, true, true);
            frame = 1;
            break;
        case 2:
            PPT.InputKey('Z', false, true);
            break;
        case 3:
            PPT.InputKey('X', false, true);
            break;
        case 4:
            PPT.InputKey(VK_LEFT, false, true);
            break;
        case 5:
            PPT.InputKey(VK_RIGHT, false, true);
            break;
        default:
            break;
        }
        puyoMode = 0;
    }
    else
    {
        if (frame)
        {
            PPT.InputKey(VK_LEFT, true, false);
            PPT.InputKey(VK_RIGHT, true, false);
            frame--;
        }
        if (!queue.empty()) puyoMode = 1;
        PPT.InputKey('Z', false, false);
        PPT.InputKey('X', false, false);
        PPT.InputKey(VK_LEFT, false, false);
        PPT.InputKey(VK_RIGHT, false, false);
        /*if (frame > 0)
        {
            /*if ((frame == 4 || frame == 2) && !queue.empty() && queue.front() != 6) PPT.InputKey(queue.front() * -2 + 94, false, true);
            if ((frame == 3 || frame == 1) && !queue.empty() && queue.front() != 6) PPT.InputKey(queue.front() * -2 + 94, false, false);
            if ((frame == 3 || frame == 1) && !queue.empty()) queue.erase(queue.begin());
            if (frame == 1)
            {
                PPT.InputKey(VK_LEFT, true, false);
                PPT.InputKey(VK_RIGHT, true, false);
            }
            frame--;
        }
        else
        {
            if (!queue.empty()) puyoMode = 1;
            PPT.InputKey('Z', false, false);
            PPT.InputKey('X', false, false);
            PPT.InputKey(VK_LEFT, false, false);
            PPT.InputKey(VK_RIGHT, false, false);
        }*/
    }
}

static int JudgementColor(int x, int y) {
    //       |    RGB      | color
    //empty  |             |   0
    //red    | (c2,7f,7d)  |   1
    //yerrow | (dc,77,1d)  |   2
    //green  | (34,bd,2e)  |   3
    //blue   | (48,68,d1)  |   4
    //purple | (b0,7c,d3)  |   5
    //ojama  | (7f,67,6b)  |   6

    BYTE r, g, b;
    PPT.GetPixelColor(x, y, r, g, b);
    static int color = 0;
    static int difference = 0;

    if (r >= g)
    {
        if (b >= r)
        {
            difference = b - g;
        }
        else if (g >= b)
        {
            difference = r - b;
        }
        else
        {
            difference = r - g;
        }
    }
    else
    {
        if (b >= g)
        {
            difference = b - r;
        }
        else if (r >= b)
        {
            difference = g - b;
        }
        else
        {
            difference = g - r;
        }
    }

    if (difference <= 0x19 && r >= 0x70)
    {
        color = 6;
        return color;
    }

    if (r >= 0x96)
    {
        if (b >= 0x96)
        {
            color = 5;
        }
        else if (b <= 0x3f)
        {
            color = 2;
        }
        else
        {
            color = 1;
        }
    }
    else if (b >= 0x96)
    {
        color = 4;
    }
    else if (g >= 0x96)
    {
        color = 3;
    }
    else
    {
        color = 0;
    }
    return color;
}

static void MoveCursor()
{
    if (GetAsyncKeyState(VK_LEFT) != 0)
    {
        cursorX--;
        if (cursorX < 0) cursorX = 0;
    }
    if (GetAsyncKeyState(VK_RIGHT) != 0)
    {
        cursorX++;
        if (cursorX > PPT.GetCaptureWidth() - 1) cursorX = PPT.GetCaptureWidth() - 1;
    }
    if (GetAsyncKeyState(VK_UP) != 0)
    {
        cursorY--;
        if (cursorY < 0) cursorY = 0;
    }
    if (GetAsyncKeyState(VK_DOWN) != 0)
    {
        cursorY++;
        if (cursorY > PPT.GetCaptureHeight() - 1) cursorY = PPT.GetCaptureHeight() - 1;
    }
}

static void SetCaptureBound()
{
    static bool Key = false, KeyOld = false;

    // This if statement passes only the first frame when you press the T key.
    //このif文はTキーを押した最初の1フレームだけ通ります。
    static int mode = 0;
    static int A = 1;
    KeyOld = Key;
    Key = GetAsyncKeyState('T') != 0;
    if (Key && !KeyOld)
    {
        //PPT.SetCapturePosition();
        //PPT.SetCaptureSize();
        if (mode == 0)
        {
            PPT.InputKey(VK_DOWN, true, true);
            mode = 1;
            A = 1;
        }
        else
        {
            mode = 0;
        }
    }
    if (mode == 0 && A)
    {
        A = 0;
        PPT.InputKey(VK_DOWN, true, false);
    }
    else if (mode == 0)
    {
        PPT.InputKey(VK_DOWN, false, false);
    }
}

static void CaptureOnlyTheFrame()
{
    static bool Key = false, KeyOld = false;

    // This if statement passes only the first frame when you press the R key.
    //このif文はRキーを押した最初の1フレームだけ通ります。
    KeyOld = Key;
    Key = GetAsyncKeyState('R') != 0;
    if (Key && !KeyOld)
    {
        PPT.CaptureScreen();
    }
}

static void RaiseFlag(int color)
{
    if ((appearance >> (color - 1) & 1) == 0)
    {
        appearance |= 1 << (color - 1);
        conv[num] = color;
        num++;
    }
}

static void CheckArrays()
{
    for (int i = 0; i < 6; i++)
    {
        copyField[i] = Field[i];
        copyFloor[i] = fieldFloor[i];
    }
}

static void ChangePuyo()
{
    static bool Key = false, KeyOld = false;

    // This if statement passes only the first frame when you press the A key.
    //このif文はAキーを押した最初の1フレームだけ通ります。
    KeyOld = Key;
    Key = GetAsyncKeyState('A') != 0;
    if (Key && !KeyOld)
    {
        appearance = 0;
        num = 0;
        for (int i = 0; i < 4; i++)
        {
            conv[i] = 0;
        }
        ImportField();
        for (int i = 0; i < 2; i++)
        {
            nextPuyo[i] = JudgementColor(251, 65 + i * 20);
            nextNextPuyo[i] = JudgementColor(263, 110 + i * 16);
            saveNext[i] = nextPuyo[i];
            saveNextNext[i] = nextNextPuyo[i];
            RaiseFlag(nextPuyo[i]);
            RaiseFlag(nextNextPuyo[i]);
        }
        for (int i = 0; i < 6; i++)
        {
            copyFloor[i] = 0;
            copyField[i] = 0;
        }
        CheckArrays();
    }
}
static void Auto()
{
    static bool Key = false, KeyOld = false;
    KeyOld = Key;
    Key = GetAsyncKeyState('M') != 0;
    if (Key && !KeyOld)
    {
        dropQueue.clear();
        queue.clear();
        if (scene == 1) MakeTemplate();
        if (scene == 2) Search();
    }
}
static void CaptureEveryFrame()
{
    PPT.CaptureScreen();
}

static void ImportField()
{
    puyoNum = 0;
    for (int i = 0; i < 6; i++)
    {
        Field[i] = 0b0;
        for (int j = 0; j < 12; j++)
        {
            __int8 color = JudgementColor(104 + i * 21, 288 - j * 20);
            if (color == 0)
            {
                fieldFloor[i] = j;

                break;
            }
            else
            {
                Field[i] |= (__int64)color << (3 * j);
            }
            fieldFloor[i] = 12;
        }
        puyoNum += fieldFloor[i];
    }
}

static int TemplateEvaluation(unsigned __int64* Pointer, unsigned __int8 Template)
{
    int score = 0;
    unsigned __int8 finish = 0;
    for (int i = T_WIDTH - 1; i >= 0; i--)
    {
        if (finish == 63) break;
        __int8 ix = i % 6;
        if (ix >= 6) break;
        __int8 iy = (T_WIDTH - i + ix) / 6 - 1;
        __int8 iColor = (*(Pointer + ix) >> (iy * 3)) & 0b111;
        __int8 iLabel = ((__int64)LABEL[Template][ix] >> iy * 4) & 0b1111;
        if (iColor != 0)
        {
            for (int j = i; j < T_WIDTH; j++)
            {
                __int8 jx = j % 6;
                if (jx >= 6) break;
                __int8 jy = (T_WIDTH - j + jx) / 6 - 1;
                __int8 jColor = (*(Pointer + jx) >> (jy * 3)) & 0b111;
                __int8 jLabel = ((__int64)LABEL[Template][jx] >> jy * 4) & 0b1111;
                int value = (WeightsTable[Template][iLabel] + WeightsTable[Template][jLabel]) / 2;
                __int8 di, dj;
                if (jLabel >= iLabel)
                {
                    if (iLabel <= 8)
                    {
                        if (((TruthTable[Template][iLabel] >> jLabel * 2) & 0b11) == 2) value *= -1;
                        if (((TruthTable[Template][iLabel] >> jLabel * 2) & 0b11) == 0) value = 0;
                    }
                    else
                    {
                        di = 16 - iLabel;
                        dj = 15 - jLabel;
                        if (((TruthTable[Template][di] >> dj * 2) & 0b11) == 2) value *= -1;
                        if (((TruthTable[Template][di] >> dj * 2) & 0b11) == 0) value = 0;
                    }
                }
                else
                {
                    if (jLabel <= 8)
                    {
                        if (((TruthTable[Template][jLabel] >> iLabel * 2) & 0b11) == 2) value *= -1;
                        if (((TruthTable[Template][jLabel] >> iLabel * 2) & 0b11) == 0) value = 0;
                    }
                    else
                    {
                        di = 15 - iLabel;
                        dj = 16 - jLabel;
                        if (((TruthTable[Template][dj] >> di * 2) & 0b11) == 2) value *= -1;
                        if (((TruthTable[Template][dj] >> di * 2) & 0b11) == 0) value = 0;
                    }
                }
                if (iColor == jColor)
                {
                    if (value < 0)
                        return 0;
                    score += value;
                }
                else if (jColor != 0)
                {
                    if (value > 0)
                        return 0;
                    score -= value;
                }
            }
        }
        else
        {
            finish |= 1 << ix;
        }
    }
    return score + adjustment[Template];
}

static void SetTmp()
{
    for (int i = 0; i < 6; i++)
    {
        tmpFloor[i] = copyFloor[i];
        tmpField[i] = copyField[i];
    }
}
static void SetTmp1()
{
    for (int i = 0; i < 6; i++)
    {
        tmpFloor1[i] = copyFloor[i];
        tmpField1[i] = copyField[i];
    }
}
static void SetTmp2()
{
    for (int i = 0; i < 6; i++)
    {
        tmpFloor2[i] = copyFloor[i];
        tmpField2[i] = copyField[i];
    }
}

static void Xorshift(unsigned int* num)
{
    *num ^= *num << 13;
    *num ^= *num >> 10;
    *num ^= *num << 5;
    *num ^= *num >> 1;
}
static int bit_pos(int x) {
    int pos = (~x) & (x + 1);
    int bitPos = 0;
    while (pos) {
        pos >>= 1;
        bitPos++;
    }

    return bitPos;
}
static const WCHAR* getColor(__int8 num) {
    switch (num) {
    case 1: return L"赤";
    case 2: return L"黄";
    case 3: return L"緑";
    case 4: return L"青";
    case 5: return L"紫";
    default: return L"黒";
    }
}

static void IntToWCHAR(int num, __int8 col1, __int8 col2,int value) {
    WCHAR base[64];  // 文字列の長さを考慮して適切なサイズ
    int Column = 0;

    // col1, col2 の範囲チェック
    if (col1 < 1 || col1 > 5 || col2 < 1 || col2 > 5) {
        return; // 範囲外の場合の処理
    }
    color = (col1 - 1) << 3 | (col2 - 1);
    place = num;
    if (num <= 11) {
        wcscpy_s(base, _ARRAYSIZE(base), L"%d列目に%ls(上),%ls(下)");
        Column = num % 6 + 1;
        newDrawString(base, Column, getColor(num <= 5 ? col1 : col2), getColor(num <= 5 ? col2 : col1));
        //newDrawString((WCHAR*)L"Score: %d", value);
    }
    else {
        wcscpy_s(base, _ARRAYSIZE(base), L"%d, %d列目に%ls(左),%ls(右)");
        Column = (num - 12) % 5 + 1;
        newDrawString(base, Column, Column + 1, getColor(num <= 16 ? col1 : col2), getColor(num <= 16 ? col2 : col1));
        //newDrawString((WCHAR*)L"Score: %d", value);
    }
}
static bool CanPut(int num, int* floorPointer)
{
    switch (num)
    {
    case 0:
    case 6:
    case 12:
    case 17:
        if (*floorPointer <= 11 && *(floorPointer + 1) <= 11) return true;
        else return false;
        break;
    case 1:
    case 7:
        if (*(floorPointer + 1) <= 11) return true;
        else return false;
        break;
    case 13:
    case 18:
        if (*(floorPointer + 1) <= 11 && *(floorPointer+2) <= 10) return true;
        else return false;
        break;
    case 2:
    case 8:
        if (*(floorPointer + 2) <= 9) return true;
        else return false;
        break;
    case 3:
    case 9:
        if (*(floorPointer + 3) <= 11) return true;
        else return false;
        break;
    case 14:
    case 19:
        if (*(floorPointer + 2) <= 10 && *(floorPointer + 3) <= 11) return true;
        else return false;
        break;
    case 4:
    case 10:
    case 15:
    case 20:
        if (*(floorPointer + 4) <= 11 && *(floorPointer + 3) <= 11) return true;
        else return false;
        break;
    case 5:
    case 11:
    case 16:
    case 21:
        if (*(floorPointer + 5) <= 11 && *(floorPointer + 4) <= 11 && *(floorPointer + 3) <= 11) return true;
        else return false;
        break;
    default:
        return false;
        break;
    }
}
struct result {
    std::vector<int> route;
    int value;
};

static int FieldEvaluation(const int* localFloor)
{
    int fieldScore = 0;
    for (int i = 0; i < 6; i++)
    {
        int num = idealHeight[i] + puyoNum / 6;
        fieldScore += (localFloor[i] - num) * (localFloor[i] - num);
    }
    if (fieldScore > 100) fieldScore = 100;
    return (int)(fieldScore / -20);
}
static void MakeTemplate() {
    std::deque<int> dis;
    static __int8 eraseCount[5] = { 0 };
    static __int64 erase[6] = { 0 };
    int value[BEAM_DEPTH][BEAM_WIDTH][2] = { 0 };// [0] - score, [1] & 0b11111 - place, [1] >> 5 - id
    __int8 color[BEAM_DEPTH][2] = { 0 };
    int Puyo[2] = { 0 };
    int place = 0;
    int score = 0;
    int parentScore = 0;
    int save = 0;
    int tmp = 0;
    int best = 0;
    int fieldValue;
    SetTmp();
    unsigned int Seed = frameCount;
    int copyConv[4] = { 0 };
    int copyApp = appearance;
    for (int i = 0; i < 4; i++)
    {
        if (conv[i] == 0)
        {
            int bitPos = bit_pos(copyApp) - 1;
            copyConv[i] = bitPos + 1;
            copyApp |= 1 << bitPos;
        }
        else
        {
            copyConv[i] = conv[i];
        }
    }
    for (int i = 0; i < MAXIMUM_DEPTH; i++)
    {
        switch (i - assist)
        {
        case -1:
            Puyo[0] = nowPuyo[0];
            Puyo[1] = nowPuyo[1];
            break;
        case 0:
            Puyo[0] = nextPuyo[0];
            Puyo[1] = nextPuyo[1];
            break;
        case 1:
            Puyo[0] = nextNextPuyo[0];
            Puyo[1] = nextNextPuyo[1];
            break;
        default:
            Xorshift(&Seed);
            __int8 num = Seed & 0b1111;
            __int8 c = num & 0b11;
            __int8 s = num >> 2;
            Puyo[0] = copyConv[c];
            Puyo[1] = copyConv[s];
            break;
        }
        color[i][0] = Puyo[0];
        color[i][1] = Puyo[1];
    }
    for (int t = 0; t < BEAM_DEPTH; t++)
    {
        Puyo[0] = color[t][0];
        Puyo[1] = color[t][1];
        for (int k = 0; k < BEAM_WIDTH; k++)
        {
            if (t == 0 && k == 1)
                break;
            if (t != 0)
            {
                if (value[t - 1][k][0] == 0) continue;
                parentScore = value[t - 1][k][0] / 5;
                int p = t;
                while (p)
                {
                    int id = k;
                    for (int q = 1; q < p; q++)
                    {
                        id = value[t - q][id][1]>>5;
                    }
                    GetStarted(copyField, copyFloor, color[t-p][0], color[t - p][1], value[t - p][id][1] & 0b1111);
                    p--;
                }
            }
            for (int i = 0; i < 22; i++)
            {
                if (!CanPut(i, copyFloor)) continue;
                SetTmp1();
                score = parentScore;
                bool Chigiri = false;
                if (i >= 12)
                {
                    if (i >= 17)
                    {
                        if (copyFloor[i - 17] != copyFloor[i - 16]) Chigiri = true;
                    }
                    else
                    {
                        if (copyFloor[i - 12] != copyFloor[i - 11]) Chigiri = true;
                    }
                }
                GetStarted(copyField, copyFloor, Puyo[0], Puyo[1], i);
                if (!detectChain(copyField, dis, erase, eraseCount))
                {
                    save = 0;
                    fieldValue = FieldEvaluation(copyFloor) * 10;
                    for (unsigned __int8 j = 0; j < T_COUNT; j++)
                    {
                        save = TemplateEvaluation(copyField, j) + fieldValue;
                        if (score < save) score = save;
                    }
                    if (Chigiri) score -= 500;
                    //LogMessage((WCHAR*)L"Score:%d(%d,%d,%d)", score, t, k, i);
                    if (value[t][BEAM_WIDTH - 1][0] < score)
                    {
                        int rank = 0;
                        for (int m = 0; m < BEAM_WIDTH; m++)
                        {
                            if (value[t][m][0] < score)
                            {
                                rank = m;
                                for (int k = 1; k < BEAM_WIDTH - m; k++)
                                {
                                    for (int l = 0; l < 2; l++)
                                    {
                                        value[t][BEAM_WIDTH - k][l] = value[t][BEAM_WIDTH - k - 1][l];
                                    }
                                }
                                break;
                            }
                        }
                        value[t][rank][0] = score;
                        value[t][rank][1] = k << 5 | i;
                    }
                }
                for (int j = 0; j < 6; j++)
                {
                    copyFloor[j] = tmpFloor1[j];
                    copyField[j] = tmpField1[j];
                }
            }
            for (int i = 0; i < 6; i++)
            {
                copyFloor[i] = tmpFloor[i];
                copyField[i] = tmpField[i];
            }
        }
    }

    if (BEAM_DEPTH >= 2)
    {
        dropQueue.clear();
        place = 0;
        for (int i = 0; i < BEAM_DEPTH - 1; i++)
        {
            place = value[BEAM_DEPTH - 1 - i][place][1]>>5;
            if (i == BEAM_DEPTH - 2)
            {
                tmp = place;
            }
        }
    }
    dropQueue.push_back(value[0][place][1]);
    if (assist)
    {
        GetStarted(copyField, copyFloor, nowPuyo[0], nowPuyo[1], value[0][place][1]);
        IntToWCHAR(value[0][place][1], nowPuyo[0], nowPuyo[1],value[0][place][0]);
    }
    else
    {
        GetStarted(copyField, copyFloor, nextPuyo[0], nextPuyo[1], value[0][place][1]);
        if (nextPuyo[0] < 1 || nextPuyo[0] > 5 || nextPuyo[1] < 1 || nextPuyo[1] > 5) {
            return; // 範囲外の場合の処理
        }
        ::color = (nextPuyo[0] - 1) << 3 | (nextPuyo[1] - 1);
        ::place = value[0][place][1];
        InvalidateRect(hNewWnd, nullptr, FALSE);
    }
}


static int Simulation(unsigned __int64* fieldPointer, int* floorPointer, std::deque<int>& dis, __int64* erase, __int8* eraseCount)
{
    int count = 0;
    int score = 0;
    int bestScore = 0;
    int save = 0;
    unsigned __int64 copyFi[6] = { 0 };
    int copyFl[6] = { 0 };
    for (int i = 0; i < 6; i++)
    {
        copyFi[i] = *(fieldPointer + i);
        copyFl[i] = *(floorPointer + i);
    }

    for (int i = 0; i < 6; i++)
    {
        for (int color = 1; color < 6; color++)
        {
            count = 0;
            score = 0;
            GetStarted(copyFi, copyFl, color, color, i);
            while (true)
            {
                save = ChainStart(copyFi, count, dis, erase, eraseCount);
                if (save == 0) break;
                score += save;
                fall(copyFi);
                count++;
            }
            //if (count * 100 > bestScore) bestScore = count * 100;
            if (score > bestScore) bestScore = score;
            for (int i = 0; i < 6; i++)
            {
                copyFi[i] = *(fieldPointer + i);
                copyFl[i] = *(floorPointer + i);
            }
        }
    }
    return bestScore;
}
/*static void Search()
{
    auto start = std::chrono::high_resolution_clock::now();
    int SEARCH_DEPTH = (6 * 13 - puyoNum) / 2 + 4;
    if (SEARCH_DEPTH > MAXIMUM_DEPTH) SEARCH_DEPTH = MAXIMUM_DEPTH;
    short value[MAXIMUM_DEPTH][SEARCH_WIDTH][2] = { 0 };//[0] - score, [1] & 0b11111 - place, [1] >> 5 - id
    __int8 color[MAXIMUM_DEPTH][2] = { 0 };
    short best[2] = { 0 };
    short nowBest[2] = { 0 };
    unsigned __int8 Puyo[2] = { 0 };
    short score = 0;
    unsigned int Seed = frameCount;
    int copyConv[4] = { 0 };
    int count = 0;
    int save = 0;
    int copyApp = appearance;
    int tmp = puyoNum;
    unsigned __int64 bestField[6] = { 0 };
    for (int i = 0; i < 4; i++)
    {
        if (conv[i] == 0)
        {
            int bitPos = bit_pos(copyApp) - 1;
            copyConv[i] = bitPos + 1;
            copyApp |= 1 << bitPos;
        }
        else
        {
            copyConv[i] = conv[i];
        }
    }
    for (int i = 0; i < MAXIMUM_DEPTH; i++)
    {
        switch (i)
        {
        case 0:
            color[0][0] = nextPuyo[0];
            color[0][1] = nextPuyo[1];
            break;
        case 1:
            color[1][0] = nextNextPuyo[0];
            color[1][1] = nextNextPuyo[1];
            break;
        default:
            Xorshift(&Seed);
            __int8 num = Seed & 0b1111;
            __int8 c = num & 0b11;
            __int8 s = num >> 2;
            color[i][0] = copyConv[c];
            color[i][1] = copyConv[s];
            break;
        }
    }
    SetTmp1();
    for (int i = 0; i < SEARCH_DEPTH; i++)
    {
        if (i == 1 && nowBest[0] > 2000) break;
        Puyo[0] = color[i][0];
        Puyo[1] = color[i][1];
        for (int j = 0; j < SEARCH_WIDTH; j++)
        {
            auto hoge = std::chrono::high_resolution_clock::now();
            puyoNum = tmp + 2 * j;
            if (i == 0 && j == 1) break;
            if (i != 0)
            {
                if (value[i - 1][j][0] == 0) continue;
                int p = i;
                while (p)
                {
                    int id = j;
                    for (int q = 1; q < p; q++)
                    {
                        id = value[i - q][id][1] >> 5;
                    }
                    GetStarted(copyField, copyFloor, color[i - p][0], color[i - p][1], value[i - p][id][1] & 0b11111);
                    p--;
                }
            }
            for (int k = 0; k < 22; k++)
            {
                auto fuga = std::chrono::high_resolution_clock::now();
                SetTmp();
                score = 0;
                GetStarted(copyField, copyFloor, Puyo[0], Puyo[1], k);
                if (!detectChain())
                {
                    unsigned __int64 hoge[6] = { 0 };
                    for (int l = 0; l < 6; l++)
                    {
                        hoge[l] = copyField[l];
                    }
                    score = Simulation() + FieldEvaluation();
                    if (score > value[i][SEARCH_WIDTH - 1][0])
                    {
                        static int rank = 0;
                        for (int l = 0; l < SEARCH_WIDTH; l++)
                        {
                            if (value[i][l][0] < score)
                            {
                                rank = l;
                                for (int m = 1; m < SEARCH_WIDTH - l; m++)
                                {
                                    for (int n = 0; n < 2; n++)
                                    {
                                        value[i][SEARCH_WIDTH - m][n] = value[i][SEARCH_WIDTH - m - 1][n];
                                    }
                                }
                                break;
                            }
                        }
                        if (score > best[0])
                        {
                            best[0] = score;
                            best[1] = i;
                            for (int l = 0; l < 6; l++)
                            {
                                bestField[l] = hoge[l];
                            }
                        }
                        value[i][rank][0] = score;
                        value[i][rank][1] = (j << 5) | k;
                    }
                }
                /*else
                {
                    if (i != 0)
                    {
                        count = 0;
                        while (true)
                        {
                            save = ChainStart(copyField, count);
                            if (save == 0) break;
                            score += save;
                            count++;
                            fall(copyField);
                        }
                        if (nowBest[0] < score)
                        {
                            nowBest[0] = score;
                            nowBest[1] = k;
                        }
                    }
                }
                for (int l = 0; l < 6; l++)
                {
                    copyField[l] = tmpField[l];
                    copyFloor[l] = tmpFloor[l];
                }
                auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> elapsed = end - fuga;
                Time = elapsed.count();

                WCHAR Text[] = L"Search.%d.%d.%d.Time:%lf";
                LogMessage(Text, i, j, k, Time);
            }
            for (int l = 0; l < 6; l++)
            {
                copyField[l] = tmpField1[l];
                copyFloor[l] = tmpFloor1[l];
            }
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed = end - hoge;
            Time = elapsed.count();

            WCHAR Text[] = L"Search.%d.%d.Time:%lf";
            LogMessage(Text, i, j, Time);
        }
    }
    puyoNum = tmp;
    if (nowBest[0] > 2000)
    {
        WCHAR text[] = L"????? : %d";
        DrawString(hMemDC, 0, 340, text, nowBest[0]);
        dropQueue.clear();
        GetStarted(copyField, copyFloor, nextPuyo[0], nextPuyo[1], nowBest[1]);
    }
    else
    {
        int place = value[best[1]][0][1] >> 5;
        for (int i = 0; i < best[1] - 1; i++)
        {
            place = value[best[1] - i][place][1] >> 5;
        }
        WCHAR text[] = L"Score : %d";
        DrawString(hMemDC, 0, 340, text, best[0]);
        queue.clear();
        dropQueue.clear();
        dropQueue.push_back(value[0][place][1] & 0b11111);
        //drop(value[0][place][1]);
        GetStarted(copyField, copyFloor, nextPuyo[0], nextPuyo[1], value[0][place][1] & 0b11111);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        Time = elapsed.count();


        WCHAR Text[] = L"Search.Time:%lf";
        LogMessage(Text, Time);
    }
}*/

static std::pair<int, int> ProcessK( const int k, const unsigned __int8 Puyo[2], const unsigned __int64* fieldPointer, const int* floorPointer, std::deque<int> dis) {
    int score = 0;
    unsigned __int64 copyFi[6] = { 0 };
    int copyFl[6] = { 0 };
    std::deque<int> copyDis;
    __int64 erase[6] = { 0 };
    __int8 eraseCount[5] = { 0 };
    for (int i = 0; i < 6; i++) {
        copyFi[i] = *(fieldPointer + i);
        copyFl[i] = *(floorPointer + i);
    }
    GetStarted(copyFi, copyFl, Puyo[0], Puyo[1], k);
    if (!detectChain(copyFi, dis, erase, eraseCount)) {
        score = Simulation(copyFi, copyFl, dis, erase, eraseCount) + FieldEvaluation(floorPointer);
    }
    return std::make_pair(score, k);
}

void Search() {
    auto start = std::chrono::high_resolution_clock::now();
    int SEARCH_DEPTH = (6 * 13 - puyoNum) / 2 + 4;
    if (SEARCH_DEPTH > MAXIMUM_DEPTH) SEARCH_DEPTH = MAXIMUM_DEPTH;

    short value[MAXIMUM_DEPTH][SEARCH_WIDTH][2] = { 0 }; // [0] - score, [1] & 0b11111 - place, [1] >> 5 - id
    short best[3] = { 0 };
    short nowBest[2] = { 0 };
    unsigned __int8 Puyo[2] = { 0 };
    unsigned int Seed = frameCount;
    int copyConv[4] = { 0 };
    int copyApp = appearance;
    int tmp = puyoNum;
    unsigned __int64 bestField[6] = { 0 };
    __int8 color[MAXIMUM_DEPTH][2] = { 0 };

    // コピーの作成と初期化
    for (int i = 0; i < 4; i++) {
        if (conv[i] == 0) {
            int bitPos = bit_pos(copyApp) - 1;
            copyConv[i] = bitPos + 1;
            copyApp |= 1 << bitPos;
        }
        else {
            copyConv[i] = conv[i];
        }
    }
    for (int i = 0; i < MAXIMUM_DEPTH; i++)
    {
        switch (i - assist)
        {
        case -1:
            Puyo[0] = nowPuyo[0];
            Puyo[1] = nowPuyo[1];
            break;
        case 0:
            Puyo[0] = nextPuyo[0];
            Puyo[1] = nextPuyo[1];
            break;
        case 1:
            Puyo[0] = nextNextPuyo[0];
            Puyo[1] = nextNextPuyo[1];
            break;
        default:
            Xorshift(&Seed);
            __int8 num = Seed & 0b1111;
            __int8 c = num & 0b11;
            __int8 s = num >> 2;
            Puyo[0] = copyConv[c];
            Puyo[1] = copyConv[s];
            break;
        }
        color[i][0] = Puyo[0];
        color[i][1] = Puyo[1];
    }

    SetTmp1();

    for (int i = 0; i < MAXIMUM_DEPTH; i++) {
        Puyo[0] = color[i][0];
        Puyo[1] = color[i][1];
        for (int j = 0; j < SEARCH_WIDTH; j++) {
            if (i == 0 && j == 1) break;
            if (i != 0)
            {
                if (value[i - 1][j][0] == 0) continue;
                int p = i;
                while (p)
                {
                    int id = j;
                    for (int q = 1; q < p; q++)
                    {
                        id = value[i - q][id][1] >> 5;
                    }
                    GetStarted(copyField, copyFloor, color[i - p][0], color[i - p][1], value[i - p][id][1] & 0b11111);
                    p--;
                }
            }
            std::vector<std::future<std::pair<int, int>>> futures;

            for (int k = 0; k < 22; k++) {
                unsigned __int64 copyFieldLocal[6];
                int copyFloorLocal[6];
                unsigned __int8 puyoLocal[2] = { Puyo[0], Puyo[1] };
                std::deque<int> dis;
                std::memcpy(copyFieldLocal, copyField, sizeof(copyFieldLocal));
                std::memcpy(copyFloorLocal, copyFloor, sizeof(copyFloorLocal));

                futures.push_back(std::async(std::launch::async, [k, puyoLocal, copyFieldLocal, copyFloorLocal, dis]() -> std::pair<int, int> {
                    try {
                        return ProcessK(k, puyoLocal, copyFieldLocal, copyFloorLocal, dis);
                    }
                    catch (const std::exception& e) {
                        std::cerr << "Exception in ProcessK: " << e.what() << '\n';
                        return std::make_pair(-1, k); // エラー処理として-1のスコアを返す
                    }
                    }));
            }

            for (int o = 0; o < 22; o++) {
                try {
                    auto result = futures[o].get();
                    int score = result.first;
                    int k = result.second;

                    // 結果の処理（scoreの最大値の更新など）
                    if (score > value[i][SEARCH_WIDTH - 1][0]) {
                        static int rank = 0;
                        for (int l = 0; l < SEARCH_WIDTH; l++) {
                            if (value[i][l][0] < score) {
                                if (best[0] < score) {
                                    best[0] = score;
                                    best[1] = i;
                                    best[2] = k;
                                    std::memcpy(bestField, copyField, sizeof(bestField));
                                }
                                rank = l;
                                for (int m = 1; m < SEARCH_WIDTH - l; m++) {
                                    for (int n = 0; n < 2; n++) {
                                        value[i][SEARCH_WIDTH - m][n] = value[i][SEARCH_WIDTH - m - 1][n];
                                    }
                                }
                                break;
                            }
                        }
                        value[i][rank][0] = score;
                        value[i][rank][1] = (j << 5) | k;
                    }
                }
                catch (const std::exception& e) {
                    std::cerr << "Exception while getting future result: " << e.what() << '\n';
                }
            }
        }
    }

    puyoNum = tmp;

    int place = value[best[1]][0][1] >> 5;
    for (int i = 0; i < best[1] - 1; i++) {
        place = value[best[1] - i][place][1] >> 5;
    }
    if (!assist)
    {
        queue.clear();
        dropQueue.clear();
        dropQueue.push_back(value[0][place][1] & 0b11111);
        GetStarted(copyField, copyFloor, nextPuyo[0], nextPuyo[1], value[0][place][1] & 0b11111);
    }
    else
    {
        GetStarted(copyField, copyFloor, nowPuyo[0], nowPuyo[1], value[0][place][1] & 0b11111);
        IntToWCHAR(value[0][place][1], nowPuyo[0], nowPuyo[1], value[0][place][0]);
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    Time = elapsed.count();
    WCHAR Text[] = L"Search.Time:%lf";
    LogMessage(Text, Time);
}

static bool Check(int n, int direction)
{
    if (n == 0 || n == 6 || n == 12 || n == 17)
    {
        return direction == 0;
    }
    else if (n == 4 || n == 5 || n == 10 || n == 11 || n == 15 || n == 16 || n == 21)
    {
        return direction == 1;
    }
    return false;
}

static bool Installation()
{
    static bool old = false;
    static int continous = 0;
    BYTE NewR, G, B;
    PPT.GetPixelColor(215, 300, NewR, G, B);
    if (NewR == R)
    {
        if (continous >= 3 && !old)
        {
            old = true;
            return true;

        }
        else
        {
            continous++;
            return false;
        }
    }
    else
    {
        old = false;
        continous = 0;
        R = NewR;
        return false;
    }
}

static void Draw()
{
    PatBlt(hMemDC, 0, 0, CLIENT_WIDTH, CLIENT_HEIGHT, BLACKNESS);

    SetCaptureBound();
    PPT.CaptureScreen();
    PPT.DrawCapturedScreen(hMemDC, 0, 0);
}

static void Hoge(int color)
{
    static bool Key = false, KeyOld = false;
    KeyOld = Key;
    Key = GetAsyncKeyState('Q') != 0;
    if (Key && !KeyOld)
    {
        for (int y = 95; y < 135; y++)
        {
            for (int x = 240; x < 280; x++)
            {
                if (JudgementColor(x, y) == color)
                {
                    newDrawString((WCHAR*)L"%d,%d", x, y);
                    LogMessage((WCHAR*)L"%d,%d", x, y);
                }
            }
        }
        newDrawString((WCHAR*)L"Not Found");
    }
}

static void BattleUpdate()
{
    //newDrawString((WCHAR*)L"%d", frameCount);
    //MakeTemplate();

    Hoge(1);

    Auto();

    DrawString(overlayDC, 0, 20, (WCHAR*)L"Hello, World!");
    InvalidateRect(overlayWnd, nullptr, FALSE);
    {
        DrawString(hMemDC, 0, 80, (WCHAR*)L"NPuyo = {%d,%d}", nextPuyo[0], nextPuyo[1]);
        DrawString(hMemDC, 0, 100, (WCHAR*)L"NNPuyo = {%d,%d}", nextNextPuyo[0], nextNextPuyo[1]);
        DrawString(hMemDC, 0, 140, (WCHAR*)L"scene = %d", scene);
        DrawString(hMemDC, 0, 180, (WCHAR*)L"Time:%f", Time);
        DrawString(hMemDC, 0, 200, (WCHAR*)L"floor = %d", fieldFloor[0]);
    }
    {
        for (int i = 0; i < 6; i++)
        {
            for (int j = 0; j < 12; j++)
            {
                WCHAR text[] = L"%d";
                DrawString(hMemDC, 104 + i * 21, 288 - j * 20, text, (copyField[i] >> j * 3) & 0b111);
            }
        }
    }

    {
        static int reliability = 0;
        static bool old = true;
        if (saveNext[0] != JudgementColor(251, 65) || saveNext[1] != JudgementColor(251, 85) || saveNextNext[0] != JudgementColor(263, 110) || saveNextNext[1] != JudgementColor(263, 126))
        {
            reliability = 0;
            if (!old) old = true;
            if (!dropQueue.empty() && !assist)
            {
                int n = dropQueue[0];
                if (Check(n, 0))
                {
                    PPT.InputKey(VK_LEFT, true, true);
                    WCHAR text[] = L"LEFT!";
                    DrawString(hMemDC, 0, 20, text);
                }
                else if (Check(n, 1))
                {
                    PPT.InputKey(VK_RIGHT, true, true);
                    WCHAR text[] = L"RIGHT!";
                    DrawString(hMemDC, 0, 20, text);
                }

            }
        }
        else
        {
            reliability++;
            if (reliability >= 2 && old && saveNext[0] == nextNextPuyo[0] && saveNext[1] == nextNextPuyo[1])
            {
                ImportField();
                WCHAR text[] = L"Color Changed!";
                DrawString(hMemDC, 0, 0, text);
                nowPuyo[0] = nextPuyo[0];
                nowPuyo[1] = nextPuyo[1];
                nextPuyo[0] = nextNextPuyo[0];
                nextPuyo[1] = nextNextPuyo[1];
                nextNextPuyo[0] = JudgementColor(263, 110);
                nextNextPuyo[1] = JudgementColor(263, 126);
                RaiseFlag(nextNextPuyo[0]);
                RaiseFlag(nextNextPuyo[1]);
                if (scene == 0 || assist) CheckArrays();
                //if (scene != 1) CheckArrays();
                if (!dropQueue.empty() && !assist)
                {
                    drop(dropQueue[0]);
                    dropQueue.erase(dropQueue.begin());
                }
                if (scene == 1) MakeTemplate();
                if (scene == 2) Search();
                //if (scene == 2) Potential();
                old = false;
            }
        }
        saveNext[0] = JudgementColor(251, 65);
        saveNext[1] = JudgementColor(251, 85);
        saveNextNext[0] = JudgementColor(263, 110);
        saveNextNext[1] = JudgementColor(263, 126);
        //DrawRGBAt(251, 65);
        //DrawRGBAt(251, 85);
        //DrawRGBAt(263, 110);
        //DrawRGBAt(263, 126);
    }
    ChangePuyo();

    // You can move the cursor with the cross key.
    // 十字キーでカーソルを動かせます。
    //MoveCursor();
    //DrawRGBAt(cursorX, cursorY);
    //JudgementColor(cursorX, cursorY);
    //DrawRGBAt(215, 300);
    //gamestart d7,77,20 x-310,y-185

    Operations();

}

static void ChangeScenes()
{
    static bool Key = false, KeyOld = false;

    // This if statement passes only the first frame when you press the S key.
    //このif文はTキーを押した最初の1フレームだけ通ります。
    KeyOld = Key;
    Key = GetAsyncKeyState('S') != 0;
    if (Key && !KeyOld)
    {
        scene++;
        //if (scene == 1) MakeTemplate();
        if (scene >= 3) scene = 0;
    }
}

void ActivityUpdate()
{
    Draw();

    ChangeScenes();

    BattleUpdate();

    frameCount++;

}

void ActivityCreate()
{
    PPT.SetWindowForeground();
    PPT.SetWindowPosition(0, 0);

    // If you want to enforce the size of the client area.
    // クライアント領域のサイズを強制したいとき。
    PPT.SetClientSize(CLIENT_WIDTH, CLIENT_HEIGHT);

    // Narrow the capture area from top, bottom, left and right.
    // キャプチャ領域を上下左右から狭めます。
    //PPT.TrimCaptureBound(50, 50, 50, 50);

    PPT.SetCapturePosition();
    PPT.SetCaptureSize();

    cursorX = 50;
    cursorY = 50;
}

void onButtonCommand(HWND hWnd, WORD code)
{
    if (code == BN_CLICKED) {
        assist = !assist;
        if (!assist) newDrawString((WCHAR*)L"Automatic Control");
        else newDrawString((WCHAR*)L"Assisting");
    }
}

void ActivityDestroy()
{
    // If you have anything to do when your application ends, you can add it here.
    // アプリケーションの終了時に何か処理することがあれば、ここに追加すると良いでしょう。
}