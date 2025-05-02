#include "framework.h"
#include "Overlay.h"
#include "PuyoyoyoAI.h"
#include "Windows.h"

#define GRAY RGB(192, 192, 192)
#define RED RGB(255,0,0)
#define YELLOW RGB(255,255,0)
#define GREEN RGB(0,255,0)
#define BLUE RGB(0,0,255)
#define PURPLE RGB(255,0,255)
#define WHITE RGB(255,255,255)
#define BLACK RGB(0,0,0)


static COLORREF getColor(int num) {
    switch (num) {
    case 1: return RED;
    case 2: return YELLOW;
    case 3: return GREEN;
    case 4: return BLUE;
    case 5: return PURPLE;
    default: return BLACK; // デフォルトは黒
    }
}

extern HWND overlayWnd;
extern HDC overlayDC;
extern bool assist;
extern int put[2][3];

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
    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
    HPEN hOldPen = (HPEN)SelectObject(memDC, hPen);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(memDC, CreateSolidBrush(BLACK));
    if (put[0][0] != -1)
    {
        HBRUSH hBrush = CreateSolidBrush(getColor(put[0][2]));
        SelectObject(memDC, hBrush);
        Rectangle(memDC, 20 + put[0][0] * 21, 40 + put[0][1] * 20, put[0][0] * 21 + 21 + 20, 60 + put[0][1] * 20);
        DeleteObject(hBrush);
    }
    if (put[1][0] != -1)
    {
        HBRUSH hBrush = CreateSolidBrush(getColor(put[1][2]));
        SelectObject(memDC, hBrush);
        Rectangle(memDC, 20 + put[1][0] * 21, 40 + put[1][1] * 20, put[1][0] * 21 + 21 + 20, 60 + put[1][1] * 20);
        DeleteObject(hBrush);
    }
    SelectObject(memDC, hOldPen);
    SelectObject(memDC, hOldBrush);
    DeleteObject(hPen);

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

}