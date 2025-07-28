#include "framework.h"
#include "Overlay.h"
#include "PuyoyoyoAI.h"
#include "Windows.h"
#include "vector"
#include "algorithm"
#include "iterator"
#include "map"

#include "iostream"
#include "fstream"
#include "string"


#pragma comment(lib, "Msimg32.lib")

using namespace std;

extern HWND overlayWnd;
extern HDC overlayDC;
extern bool assist;
extern int put[2][3];
extern vector<Data> sub;

static map<int, HPEN> penMap;
static map<int, HBRUSH> brushMap;

static HPEN getPen(int num) {
    auto it = penMap.find(num);
    if (it != penMap.end()) {
        return it->second;
    }
    COLORREF color;
    switch (num) {
    case 1: color = RGB(255, 0, 0); break;     // RED
    case 2: color = RGB(255, 255, 0); break;   // YELLOW
    case 3: color = RGB(0, 255, 0); break;     // GREEN
    case 4: color = RGB(0, 0, 255); break;     // BLUE
    case 5: color = RGB(255, 0, 255); break;   // PURPLE
    case 6: color = RGB(255, 255, 255); break; // WHITE
    default: color = RGB(0, 0, 0); break;      // BLACK
    }
    HPEN pen = CreatePen(PS_SOLID, 1, color);
    penMap[num] = pen;
    return pen;
}

// 終了時に呼ぶ
static void cleanupPens() {
    for (auto& p : penMap) {
        DeleteObject(p.second);
    }
    penMap.clear();
}

static HBRUSH getBrush(int num) {
    auto it = brushMap.find(num);
    if (it != brushMap.end()) {
        return it->second;
    }
    COLORREF color;
    switch (num) {
    case 1: color = RGB(255, 0, 0); break;     // RED
    case 2: color = RGB(255, 255, 0); break;   // YELLOW
    case 3: color = RGB(0, 255, 0); break;     // GREEN
    case 4: color = RGB(0, 0, 255); break;     // BLUE
    case 5: color = RGB(255, 0, 255); break;   // PURPLE
    case 6: color = RGB(255, 255, 255); break; // WHITE
    default: color = RGB(0, 0, 0); break;      // BLACK
    }
    HBRUSH brush = CreateSolidBrush(color);
    brushMap[num] = brush;
    return brush;
}

// 終了時にまとめて解放
static void cleanupBrushes() {
    for (auto& p : brushMap) {
        DeleteObject(p.second);
    }
    brushMap.clear();
}

void OverlayPaint() {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(overlayWnd, &ps);

    // メモリDCと互換ビットマップ作成
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBitmap = CreateCompatibleBitmap(hdc, OVERLAY_WIDTH, OVERLAY_HEIGHT);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

    // 背景を白で塗りつぶし（PatBlt はメモリDCに対して）
    PatBlt(memDC, 0, 0, OVERLAY_WIDTH, OVERLAY_HEIGHT, WHITENESS);

    // フィールドのマスを描画
    for (int i = 0; i < 13; i++) {
        for (int j = 0; j < 6; j++) {
            Rectangle(memDC, 20 + j * 21, 40 + i * 20, j * 21 + 21 + 20, 60 + i * 20);
        }
    }
    HPEN hOldPen = (HPEN)SelectObject(memDC, getPen(100));
    HBRUSH hOldBrush = (HBRUSH)SelectObject(memDC, getBrush(100));
    if (put[0][0] != -1)
    {
        SelectObject(memDC, getBrush(put[0][2]));
        Rectangle(memDC, 20 + put[0][0] * 21, 40 + 240 - put[0][1] * 20, put[0][0] * 21 + 21 + 20, 60 + 240 - put[0][1] * 20);
    }
    if (put[1][0] != -1)
    {
        SelectObject(memDC, getBrush(put[1][2]));
        Rectangle(memDC, 20 + put[1][0] * 21, 40 + 240 - put[1][1] * 20, put[1][0] * 21 + 21 + 20, 60 + 240 - put[1][1] * 20);
    }

    // (1) 32bpp DIBセクションを作成
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = OVERLAY_WIDTH;
    bmi.bmiHeader.biHeight = -OVERLAY_HEIGHT; // 上から下へ
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;


    void* pvBits = nullptr;
    HBITMAP alphaBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pvBits, nullptr, 0);
    HDC alphaDC = CreateCompatibleDC(hdc);
    HBITMAP oldAlphaBitmap = (HBITMAP)SelectObject(alphaDC, alphaBitmap);
    if (alphaBitmap == nullptr) {
        // エラー処理（例：ログ出力やリターン）
        MessageBox(nullptr, L"CreateDIBSection failed!", L"Error", MB_OK | MB_ICONERROR);
        return;
    }
    UINT32* pixels = (UINT32*)pvBits;


    for (const Data& element : sub)
    {
        if (element.display == 0)
        {
            HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, getBrush(6));
            for (const auto& puyo : element.puyos)
            {
                if (get<0>(puyo) == -1) continue;
                HPEN oldPen = (HPEN)SelectObject(memDC, getPen(get<2>(puyo)));

                Rectangle(memDC,
                    20 + get<0>(puyo) * 21,
                    40 + 240 - get<1>(puyo) * 20,
                    get<0>(puyo) * 21 + 21 + 20,
                    60 + 240 - get<1>(puyo) * 20);

                SelectObject(memDC, oldPen);
            }
            SelectObject(memDC, oldBrush);
        }
        else
        {
            
            //HPEN oldPen = (HPEN)SelectObject(alphaDC, getPen(100));
            for (const auto& puyo : element.puyos)
            {
                /*if (get<0>(puyo) == -1) continue;
                HBRUSH oldBrush = (HBRUSH)SelectObject(alphaDC, getBrush(get<2>(puyo)));

                Rectangle(alphaDC,
                    20 + get<0>(puyo) * 21,
                    40 + 240 - get<1>(puyo) * 20,
                    get<0>(puyo) * 21 + 21 + 20,
                    60 + 240 - get<1>(puyo) * 20);

                SelectObject(alphaDC, oldBrush);*/
                UINT32 color = 0xFF000000; // まず透過なしの黒を初期化（A=255）

                switch (get<2>(puyo)) {
                case 1: // 赤
                    color = (128 << 24) | (255 << 16) | (0 << 8) | 0;    // A=128, R=255, G=0, B=0
                    break;
                case 2: // 黄
                    color = (128 << 24) | (255 << 16) | (255 << 8) | 0;  // A=128, R=255, G=255, B=0
                    break;
                case 3: // 緑
                    color = (128 << 24) | (0 << 16) | (255 << 8) | 0;    // A=128, R=0, G=255, B=0
                    break;
                case 4: // 青
                    color = (128 << 24) | (0 << 16) | (0 << 8) | 255;    // A=128, R=0, G=0, B=255
                    break;
                case 5: // 紫
                    color = (128 << 24) | (255 << 16) | (0 << 8) | 255;  // A=128, R=255, G=0, B=255
                    break;
                case 6: // 白
                    color = (128 << 24) | (255 << 16) | (255 << 8) | 255; // A=128, R=255, G=255, B=255
                    break;
                default: // 黒 or 透明
                    color = (128 << 24) | (0 << 16) | (0 << 8) | 0;
                    break;
                }

                for (int x = 20 + get<0>(puyo) * 21; x < 20 + get<0>(puyo) * 21 + 21; x++)
                {
                    for (int y = 40 + 240 - get<1>(puyo) * 20; y < 40 + 240 - get<1>(puyo) * 20 + 20; y++)
                    {
                        pixels[y * OVERLAY_WIDTH + x] = color;
                    }
                }
            }

            //SelectObject(alphaDC, oldPen);
        }
    }

    BLENDFUNCTION bf;
    bf.BlendOp = AC_SRC_OVER;
    bf.BlendFlags = 0;
    bf.SourceConstantAlpha = 255;   // 全体不透明
    bf.AlphaFormat = AC_SRC_ALPHA;  // ピクセルごとのαを使う

    AlphaBlend(memDC, 0, 0, OVERLAY_WIDTH, OVERLAY_HEIGHT,
        alphaDC, 0, 0, OVERLAY_WIDTH, OVERLAY_HEIGHT, bf);

    SelectObject(alphaDC, oldAlphaBitmap);
    DeleteObject(alphaBitmap);
    DeleteDC(alphaDC);

    SelectObject(memDC, hOldPen);
    SelectObject(memDC, hOldBrush);


    // モード表示
    if (assist) {
        TextOut(memDC, 10, 10, L"Assist Mode", 11);
    }
    else {
        TextOut(memDC, 10, 10, L"Auto Mode", 9);
    }

    // メモリDCを実画面に転送（BitBlt）
    BitBlt(hdc, 0, 0, OVERLAY_WIDTH, OVERLAY_HEIGHT, memDC, 0, 0, SRCCOPY);

    // リソース解放
    SelectObject(memDC, oldBitmap);
    DeleteObject(memBitmap);
    DeleteDC(memDC);

    EndPaint(overlayWnd, &ps);
}

void OverlayDestroy() {
    cleanupPens();
    cleanupBrushes();
    // ログファイルのパス
    const string logFilePath = "application.log";

    // std::ofstreamを使用してファイルを開く（ファイルが存在しない場合は自動的に作成される）
    ofstream logFile(logFilePath, std::ios::app); // 追記モードで開く

    // ファイルが正常に開けたかチェック
    if (logFile.is_open())
    {
        // メッセージを書き込む
        logFile << (string)"Pens And Brushes Are Deleted!" << std::endl;
        // ファイルを閉じる
        logFile.close();
    }
    else
    {
        std::cerr << "Failed to open log file: " << logFilePath << std::endl;
    }
}