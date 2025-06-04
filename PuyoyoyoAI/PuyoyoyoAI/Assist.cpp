#include "framework.h"
#include "Assist.h"
#include "PuyoyoyoAI.h"
#include "Windows.h"
#include "template.h"


#define SIZE 60
#define START (ASSIST_WIDTH / 2 - 3 * SIZE - 10)
#define GRAY RGB(192, 192, 192)

const COLORREF colors[] = {
    RGB(0,0,0),
    RGB(255,0,0),
    RGB(255,255,0),
    RGB(0,255,0),
    RGB(0,0,255),
    RGB(255,0,255),
    RGB(0,255,255),
    RGB(255,0,165),
    RGB(255,165,0),
    RGB(255,105,180),
    RGB(220,20,60),
    RGB(117,255,0),
    RGB(0,165,255),
    RGB(0,191,255),
    RGB(64,224,208),
    RGB(75,0,130)
};


extern HWND hNewWnd;
extern int index;
extern WCHAR buf[256];

void DrawRectangle(HDC hdc, int x, int y, int color) {
    HPEN pen = CreatePen(PS_SOLID, 4, colors[color]);
    HBRUSH brush = color == 0 ? CreateSolidBrush(GRAY) : CreateSolidBrush(colors[color]);
    HGDIOBJ oldBrush = SelectObject(hdc, brush);
    HGDIOBJ oldPen = SelectObject(hdc, pen);

    int left = x * SIZE + START;
    int top = 100 + (6 - y) * SIZE;
    int right = x * SIZE + SIZE + START;
    int bottom = 100 + (6 - y) * SIZE + SIZE;
    Rectangle(hdc, left, top, right, bottom);

    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);

    DeleteObject(pen);
    DeleteObject(brush);
}

void AssistPaint() {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hNewWnd, &ps);

    // メモリデバイスコンテキストを作成
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBitmap = CreateCompatibleBitmap(hdc, ps.rcPaint.right - ps.rcPaint.left, ps.rcPaint.bottom - ps.rcPaint.top);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

    // 背景を灰色で塗りつぶす
    HBRUSH hBrush = CreateSolidBrush(GRAY); // 灰色
    FillRect(memDC, &ps.rcPaint, hBrush);

    HPEN hPen = CreatePen(PS_SOLID, 4, RGB(0, 0, 0));
    HGDIOBJ oldBrush = SelectObject(memDC, hBrush);
    HGDIOBJ oldPen = SelectObject(memDC, hPen);

    // 盤面を描画
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 6; j++) {
            DrawRectangle(memDC, j, i, 0);
        }
    }

    if (index != -1) {
        for (int i = 0; i < 6; i++) {
            __int8 max = notPuyo[index];
            for (int j = 0; j < 7; j++) {
                if ((LABEL[index][i] >> (j * 4) & 0b1111) >= max) break;
                DrawRectangle(memDC, i, j, (LABEL[index][i] >> (j * 4) & 0b1111));
            }
        }
    }

    // テキスト描画
    SetBkColor(memDC, GRAY);
    RECT rect;
    SetRect(&rect, 10, 10, 300, 60);
    ExtTextOut(memDC, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
    SetTextColor(memDC, RGB(0, 0, 0));

    TextOut(memDC, 10, 10, index == -1 ? (WCHAR*)L"現在の状況で使えるテンプレートは存在しません。" : buf,
        index != -1 ? wcslen(buf) : 23);

    // メモリDCから実際のDCへ転送
    BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top,
        ps.rcPaint.right - ps.rcPaint.left,
        ps.rcPaint.bottom - ps.rcPaint.top,
        memDC, 0, 0, SRCCOPY);

    // オブジェクトの選択を元に戻す
    SelectObject(memDC, oldPen);
    SelectObject(memDC, oldBrush);
    SelectObject(memDC, oldBitmap);

    // リソースを解放
    DeleteObject(hPen);
    DeleteObject(hBrush);
    DeleteObject(memBitmap);
    DeleteDC(memDC);

    EndPaint(hNewWnd, &ps);
}


void AssistDestroy() {

}