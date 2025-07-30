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
extern bool hakka;

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

// �I�����ɌĂ�
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

// �I�����ɂ܂Ƃ߂ĉ��
static void cleanupBrushes() {
    for (auto& p : brushMap) {
        DeleteObject(p.second);
    }
    brushMap.clear();
}

std::string WCHARArrayToString(const WCHAR* wideCharArray) {
    // �ϊ����邽�߂̃T�C�Y���擾����
    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wideCharArray, -1, nullptr, 0, nullptr, nullptr);
    if (sizeNeeded <= 0) {
        // �G���[�����A�K�v�ɉ����ė�O���X���[����Ȃ�
        return "";
    }

    // �ϊ���̃o�b�t�@���m�ۂ���
    std::string multiByteString(sizeNeeded - 1, '\0'); // -1 �� NULL �I�[������

    // ���ۂɕϊ����s��
    WideCharToMultiByte(CP_UTF8, 0, wideCharArray, -1, &multiByteString[0], sizeNeeded, nullptr, nullptr);

    return multiByteString;
}

static void LogMessage(WCHAR* format, ...)
{
    // ���O�t�@�C���̃p�X
    const std::string logFilePath = "application.log";

    // std::ofstream���g�p���ăt�@�C�����J���i�t�@�C�������݂��Ȃ��ꍇ�͎����I�ɍ쐬�����j
    std::ofstream logFile(logFilePath, std::ios::app); // �ǋL���[�h�ŊJ��

    // �t�@�C��������ɊJ�������`�F�b�N
    if (logFile.is_open())
    {
        va_list args;
        va_start(args, format);
        WCHAR buf[256];
        int len = _vsntprintf_s(buf, _ARRAYSIZE(buf), _TRUNCATE, format, args);
        va_end(args);

        std::string result = WCHARArrayToString(buf);

        // ���b�Z�[�W����������
        logFile << result << std::endl;
        // �t�@�C�������
        logFile.close();
    }
    else
    {
        std::cerr << "Failed to open log file: " << logFilePath << std::endl;
    }
}
void OverlayPaint() {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(overlayWnd, &ps);

    // ������DC�ƌ݊��r�b�g�}�b�v�쐬
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBitmap = CreateCompatibleBitmap(hdc, OVERLAY_WIDTH, OVERLAY_HEIGHT);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

    // �w�i�𔒂œh��Ԃ��iPatBlt �̓�����DC�ɑ΂��āj
    PatBlt(memDC, 0, 0, OVERLAY_WIDTH, OVERLAY_HEIGHT, WHITENESS);

    // �t�B�[���h�̃}�X��`��
    for (int i = 0; i < 13; i++) {
        for (int j = 0; j < 6; j++) {
            Rectangle(memDC, 20 + j * 21, 40 + i * 20, j * 21 + 21 + 20, 60 + i * 20);
        }
    }
    HPEN pen = CreatePen(PS_SOLID, hakka ? 5 : 1, RGB(0, 0, 0));
    HPEN hOldPen = (HPEN)SelectObject(memDC, pen);
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


    // (1) 32bpp DIB�Z�N�V�������쐬
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = OVERLAY_WIDTH;
    bmi.bmiHeader.biHeight = -OVERLAY_HEIGHT; // �ォ�牺��
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* pvBits = nullptr;

    HBITMAP alphaBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pvBits, nullptr, 0);
    if (alphaBitmap == NULL)
    {
        MessageBox(NULL, L"CreateDIBSection failed", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    HDC alphaDC = CreateCompatibleDC(hdc);
    if (alphaDC == NULL)
    {
        MessageBox(NULL, L"CreateCompatibleDC failed", L"Error", MB_OK | MB_ICONERROR);
        DeleteObject(alphaBitmap);
        return;
    }

    HGDIOBJ oldAlphaBitmap = SelectObject(alphaDC, alphaBitmap);
    if (oldAlphaBitmap == NULL || oldAlphaBitmap == HGDI_ERROR)
    {
        MessageBox(NULL, L"SelectObject failed", L"Error", MB_OK | MB_ICONERROR);
        DeleteDC(alphaDC);
        DeleteObject(alphaBitmap);
        return;
    }

    UINT32* pixels = (UINT32*)pvBits;
    std::fill(pixels, pixels + (OVERLAY_WIDTH * OVERLAY_HEIGHT), 0x00000000);

    for (const Data& element : sub)
    {
        if (element.display == 0)
        {
            HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, getBrush(6));
            for (const auto& puyo : element.puyos)
            {
                LogMessage((WCHAR*)L"puyo: (%d, %d, %d)", get<0>(puyo), get<1>(puyo), get<2>(puyo));
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
                UINT32 color = 0xFF000000; // �܂����߂Ȃ��̍����������iA=255�j

                switch (get<2>(puyo)) {
                case 1: // ��
                    color = (128 << 24) | (255 << 16) | (0 << 8) | 0;    // A=128, R=255, G=0, B=0
                    break;
                case 2: // ��
                    color = (128 << 24) | (255 << 16) | (255 << 8) | 0;  // A=128, R=255, G=255, B=0
                    break;
                case 3: // ��
                    color = (128 << 24) | (0 << 16) | (255 << 8) | 0;    // A=128, R=0, G=255, B=0
                    break;
                case 4: // ��
                    color = (128 << 24) | (0 << 16) | (0 << 8) | 255;    // A=128, R=0, G=0, B=255
                    break;
                case 5: // ��
                    color = (128 << 24) | (255 << 16) | (0 << 8) | 255;  // A=128, R=255, G=0, B=255
                    break;
                case 6: // ��
                    color = (128 << 24) | (255 << 16) | (255 << 8) | 255; // A=128, R=255, G=255, B=255
                    break;
                default: // �� or ����
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
    bf.SourceConstantAlpha = 255;   // �S�̕s����
    bf.AlphaFormat = AC_SRC_ALPHA;  // �s�N�Z�����Ƃ̃����g��

    AlphaBlend(memDC, 0, 0, OVERLAY_WIDTH, OVERLAY_HEIGHT,
        alphaDC, 0, 0, OVERLAY_WIDTH, OVERLAY_HEIGHT, bf);

    SelectObject(alphaDC, oldAlphaBitmap);
    DeleteObject(alphaBitmap);
    DeleteDC(alphaDC);

    SelectObject(memDC, hOldPen);
    SelectObject(memDC, hOldBrush);


    // ���[�h�\��
    if (assist) {
        TextOut(memDC, 10, 10, L"Assist Mode", 11);
    }
    else {
        TextOut(memDC, 10, 10, L"Auto Mode", 9);
    }

    // ������DC������ʂɓ]���iBitBlt�j
    BitBlt(hdc, 0, 0, OVERLAY_WIDTH, OVERLAY_HEIGHT, memDC, 0, 0, SRCCOPY);

    // ���\�[�X���
    DeleteObject(pen);
    DeleteObject(hOldPen);
    DeleteObject(hOldBrush);
    SelectObject(memDC, oldBitmap);
    DeleteObject(memBitmap);
    DeleteDC(memDC);

    EndPaint(overlayWnd, &ps);
}



void OverlayDestroy() {
    cleanupPens();
    cleanupBrushes();
    LogMessage((WCHAR*)L"Pens And Brushes Were Deleted.");
}