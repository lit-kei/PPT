#include "framework.h"
#include "Overlay.h"
#include "PuyoyoyoAI.h"
#include "Windows.h"
#include "vector"
#include "algorithm"
#include "iterator"

#pragma comment(lib, "Msimg32.lib")

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

using namespace std;

extern HWND overlayWnd;
extern HDC overlayDC;
extern bool assist;
extern int put[2][3];
extern vector<Data> sub;

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
    HPEN hPen = CreatePen(PS_SOLID, 1, BLACK);
    HPEN hOldPen = (HPEN)SelectObject(memDC, hPen);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(memDC, CreateSolidBrush(BLACK));
    if (put[0][0] != -1)
    {
        HBRUSH hBrush = CreateSolidBrush(getColor(put[0][2]));
        SelectObject(memDC, hBrush);
        Rectangle(memDC, 20 + put[0][0] * 21, 40 + 240 - put[0][1] * 20, put[0][0] * 21 + 21 + 20, 60 + 240 - put[0][1] * 20);
        DeleteObject(hBrush);
    }
    if (put[1][0] != -1)
    {
        HBRUSH hBrush = CreateSolidBrush(getColor(put[1][2]));
        SelectObject(memDC, hBrush);
        Rectangle(memDC, 20 + put[1][0] * 21, 40 + 240 - put[1][1] * 20, put[1][0] * 21 + 21 + 20, 60 + 240 - put[1][1] * 20);
        DeleteObject(hBrush);
    }

    vector<Data> strokes;
    copy_if(sub.begin(), sub.end(), back_inserter(strokes), [](const Data& d) { return d.display == 0; });
    for (unsigned int i = 0; i < strokes.size(); i++)
    {
        Data element = strokes[i];
        if (get<0>(element.shaft) == -1 || get<0>(element.child) == -1) continue;
        HPEN pen = CreatePen(PS_SOLID, 1, getColor(get<2>(element.shaft)));
        SelectObject(memDC, pen);
        SelectObject(memDC, CreateSolidBrush(WHITE));
        Rectangle(memDC, 20 + get<0>(element.shaft) * 21, 40 + 240 - get<1>(element.shaft) * 20, get<0>(element.shaft) * 21 + 21 + 20, 60 + 240 - get<1>(element.shaft) * 20);
        pen = CreatePen(PS_SOLID, 1, getColor(get<2>(element.child)));
        SelectObject(memDC, pen);
        Rectangle(memDC, 20 + get<0>(element.child) * 21, 40 + 240 - get<1>(element.child) * 20, get<0>(element.child) * 21 + 21 + 20, 60 + 240 - get<1>(element.child) * 20);
        DeleteObject(pen);
    }
    vector<Data> alpha;
    copy_if(sub.begin(), sub.end(), back_inserter(alpha), [](const Data& d) { return d.display == 1; });
    HDC alphaDC = CreateCompatibleDC(hdc);
    HBITMAP alphaBitmap = CreateCompatibleBitmap(hdc, OVERLAY_WIDTH, OVERLAY_HEIGHT);
    HBITMAP oldAlphaBitmap = (HBITMAP)SelectObject(alphaDC, alphaBitmap);
    PatBlt(alphaDC, 0, 0, OVERLAY_WIDTH, OVERLAY_HEIGHT, WHITENESS);
    SelectObject(alphaDC, hPen);
    for (unsigned int i = 0; i < alpha.size(); i++)
    {
        Data element = alpha[i];
        if (get<0>(element.shaft) == -1 || get<0>(element.child) == -1) continue;
        HBRUSH puyo = CreateSolidBrush(getColor(get<2>(element.shaft)));
        SelectObject(alphaDC, puyo);

        Rectangle(alphaDC, 20 + get<0>(element.shaft) * 21, 40 + 240 - get<1>(element.shaft) * 20, get<0>(element.shaft) * 21 + 21 + 20, 60 + 240 - get<1>(element.shaft) * 20);
        puyo = CreateSolidBrush(getColor(get<2>(element.child)));
        SelectObject(alphaDC, puyo);
        Rectangle(alphaDC, 20 + get<0>(element.child) * 21, 40 + 240 - get<1>(element.child) * 20, get<0>(element.child) * 21 + 21 + 20, 60 + 240 - get<1>(element.child) * 20);
        DeleteObject(puyo);
    }
    //BLENDFUNCTION bf = { AC_SRC_OVER, 0, 128, 0 };
    //AlphaBlend(memDC, 0, 0, OVERLAY_WIDTH, OVERLAY_HEIGHT, alphaDC, 0, 0, OVERLAY_WIDTH, OVERLAY_HEIGHT, bf);
    TransparentBlt(memDC, 0, 0, OVERLAY_WIDTH, OVERLAY_HEIGHT, alphaDC, 0, 0, OVERLAY_WIDTH, OVERLAY_HEIGHT, WHITE);

    SelectObject(alphaDC, oldAlphaBitmap);
    DeleteObject(alphaBitmap);
    DeleteDC(alphaDC);

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