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


COLORREF getColor(int num) {
    switch (num) {
    case 0: return RED;
    case 1: return YELLOW;
    case 2: return GREEN;
    case 3: return BLUE;
    case 4: return PURPLE;
    default: return BLACK; // デフォルトは黒
    }
}

extern HWND hNewWnd;
extern WCHAR buf[256];
extern int place;
extern int color;

void OverlayPaint() {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hNewWnd, &ps);

    // 背景を白色で塗りつぶす
    HBRUSH hBrush = CreateSolidBrush(WHITE);

    FillRect(hdc, &ps.rcPaint, hBrush);
    HPEN hPen = CreatePen(PS_SOLID, 4, WHITE);
    // ブラシとペンを描画コンテキストに選択
    HGDIOBJ oldBrush = SelectObject(hdc, hBrush);
    HGDIOBJ oldPen = SelectObject(hdc, hPen);
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 6; j++)
        {
            Rectangle(hdc, j * 50, 250 - i * 50, j * 50 + 50, 300 - i * 50);
        }
    }
    if (place != -1)
    {
        HPEN shaft = CreatePen(PS_SOLID, 4, getColor(color & 0b111));
        HPEN child = CreatePen(PS_SOLID, 4, getColor(color >> 3));
        if (place <= 11)
        {
            bool hoge = place <= 5;
            SelectObject(hdc, shaft);
            Rectangle(hdc, (place % 6) * 50, hoge ? 250 : 200, (place % 6) * 50 + 50, hoge ? 300 : 250);
            SelectObject(hdc, child);
            Rectangle(hdc, (place % 6) * 50, hoge ? 200 : 250, (place % 6) * 50 + 50, hoge ? 250 : 300);
        }
        else
        {
            bool hoge = place <= 16;
            int n = place - 12;
            if (n >= 5) n -= 5;
            SelectObject(hdc, shaft);
            Rectangle(hdc, hoge ? n * 50 + 50 : n * 50, 250, hoge ? n * 50 + 100 : n * 50 + 50, 300);
            SelectObject(hdc, child);
            Rectangle(hdc, hoge ? n * 50 : n * 50 + 50, 250, hoge ? n * 50 + 50 : n * 50 + 100, 300);
        }
        SelectObject(hdc, oldBrush);
        SelectObject(hdc, oldPen);

        DeleteObject(shaft);
        DeleteObject(child);
    }

    // リソースを解放

    DeleteObject(hPen);
    DeleteObject(hBrush);

    SetBkColor(hdc, GRAY);
    RECT rect;
    SetRect(&rect, 10, 10, 300, 60); // 矩形の座標を設定
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
    SetTextColor(hdc, RGB(0, 0, 0));

    TextOut(hdc, 10, 10, buf, wcslen(buf));
    EndPaint(hNewWnd, &ps);
}

void OverlayDestroy() {

}