// PuyoyoyoAI.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "framework.h"
#include "PuyoyoyoAI.h"
#include "Activity.h"
#include "Assist.h"
#include "Overlay.h"
#include "Windows.h"
#include "iostream"
#include "fstream"
#include "cstdio"
#include "vector"

#define MAX_LOADSTRING 100


// グローバル変数:
HINSTANCE hInst;                                // 現在のインターフェイス
WCHAR szTitle[MAX_LOADSTRING];                  // タイトル バーのテキスト
WCHAR szWindowClass[MAX_LOADSTRING];            // メイン ウィンドウ クラス名


WCHAR buf[256];
int index = -1;
int put[2][3] = { {-1,0,0}, {-1,0,0} };
bool assist = true;
std::vector<Data> sub;

// このコード モジュールに含まれる関数の宣言を転送します:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
HDC hMemDC;
HBITMAP hMemPrev;

HWND hNewWnd;
HBITMAP windowPrev;

HWND overlayWnd;
HDC overlayDC;
HBITMAP overlayPrev;

LRESULT CALLBACK NewWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_PAINT: {
        AssistPaint();
        break;
    }
    case WM_DESTROY:
        AssistDestroy();
        PostQuitMessage(0);
        break;
    case WM_GETMINMAXINFO:
    {
        MINMAXINFO* pMinMax = (MINMAXINFO*)lParam;
        pMinMax->ptMinTrackSize.x = ASSIST_WIDTH; // 最小幅
        pMinMax->ptMinTrackSize.y = ASSIST_HEIGHT; // 最小高さ
        pMinMax->ptMaxTrackSize.x = ASSIST_WIDTH; // 最大幅
        pMinMax->ptMaxTrackSize.y = ASSIST_HEIGHT; // 最大高さ
    }
    return 0;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK OverlayWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
    {
        HDC hdc = GetDC(hWnd);
        overlayDC = CreateCompatibleDC(hdc);
        HBITMAP hBitmap = CreateCompatibleBitmap(hdc, OVERLAY_WIDTH, OVERLAY_HEIGHT);
        overlayPrev = (HBITMAP)SelectObject(overlayDC, hBitmap);
        ReleaseDC(hWnd, hdc);
    }
    case WM_PAINT:
    {
        
        //TextOut(overlayDC, 10, 10, L"こんにちは", 4);
        //DrawString(overlayDC, 10, 50, (WCHAR*)L"Hello", 5);
        OverlayPaint();

        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        BitBlt(hdc, 0, 0, OVERLAY_WIDTH, OVERLAY_HEIGHT, overlayDC, 0, 0, SRCCOPY);
        EndPaint(hWnd, &ps);
        break;
    }
    case WM_ERASEBKGND:
    {
        HDC hdc = (HDC)wParam;
        RECT rect;
        GetClientRect(hWnd, &rect);
        // 背景色を白に設定
        SetBkColor(hdc, RGB(255, 255, 255)); // 白
        ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rect, nullptr, 0, nullptr);
        return 1;
    }
    case WM_GETMINMAXINFO:
    {
        MINMAXINFO* pMinMax = (MINMAXINFO*)lParam;
        pMinMax->ptMinTrackSize.x = OVERLAY_WIDTH; // 最小幅
        pMinMax->ptMinTrackSize.y = OVERLAY_HEIGHT; // 最小高さ
        pMinMax->ptMaxTrackSize.x = OVERLAY_WIDTH; // 最大幅
        pMinMax->ptMaxTrackSize.y = OVERLAY_HEIGHT; // 最大高さ
    }
    case WM_DESTROY:
    {

        HBITMAP hBitmap = (HBITMAP)SelectObject(overlayDC, overlayPrev);
        DeleteObject(hBitmap);
        DeleteDC(overlayDC);
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // グローバル文字列を初期化する
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PUYOYOYOAI, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // アプリケーション初期化の実行:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PUYOYOYOAI));

    MSG msg;

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = NewWndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"NewWindowClass";

    RegisterClass(&wc);

    // 新しいウィンドウを作成
    hNewWnd = CreateWindowEx(
        0,
        L"NewWindowClass",
        L"Assist",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, ASSIST_WIDTH, ASSIST_HEIGHT,
        NULL, NULL, GetModuleHandle(NULL), NULL);
    //ShowWindow(hNewWnd, SW_SHOW);

    WNDCLASS oWc = { 0 };
    oWc.lpfnWndProc = OverlayWndProc;
    oWc.hInstance = GetModuleHandle(NULL);
    oWc.lpszClassName = L"OverlayClass";

    if (!RegisterClass(&oWc)) {
        return 1;
    }

    overlayWnd = CreateWindowEx(
        WS_EX_LAYERED,
        oWc.lpszClassName,
        L"Overlay",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 315, 360,
        NULL, NULL, GetModuleHandle(NULL), NULL);
    SetLayeredWindowAttributes(overlayWnd, RGB(255, 255, 255), 0, LWA_COLORKEY);
    ShowWindow(overlayWnd, SW_SHOW);
    UpdateWindow(overlayWnd);

    // メイン メッセージ ループ:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}




//
//  関数: MyRegisterClass()
//
//  目的: ウィンドウ クラスを登録します。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PUYOYOYOAI));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_PUYOYOYOAI);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   関数: InitInstance(HINSTANCE, int)
//
//   目的: インスタンス ハンドルを保存して、メイン ウィンドウを作成します
//
//   コメント:
//
//        この関数で、グローバル変数でインスタンス ハンドルを保存し、
//        メイン プログラム ウィンドウを作成および表示します。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // グローバル変数にインスタンス ハンドルを格納する

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

static void DeleteLogFile(const std::string& filePath)
{
    if (std::remove(filePath.c_str()) == 0)
    {
        std::cout << "File deleted successfully: " << filePath << std::endl;
    }
}

//
//  関数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的: メイン ウィンドウのメッセージを処理します。
//
//  WM_COMMAND  - アプリケーション メニューの処理
//  WM_PAINT    - メイン ウィンドウを描画する
//  WM_DESTROY  - 中止メッセージを表示して戻る
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        DeleteLogFile("application.log");
        HDC hdc = GetDC(hWnd);
        hMemDC = CreateCompatibleDC(hdc);
        HBITMAP hBitmap = CreateCompatibleBitmap(hdc, CLIENT_WIDTH, CLIENT_HEIGHT);
        hMemPrev = (HBITMAP)SelectObject(hMemDC, hBitmap);
        ReleaseDC(hWnd, hdc);


        SetTimer(hWnd, 100, 1000 / 30, nullptr);

        ActivityCreate();
        HWND button;
        button = CreateWindowA(
            "BUTTON",
            "AUTO",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            640, 0, 100, 30,
            hWnd, (HMENU)1, ((LPCREATESTRUCT)(lParam))->hInstance, NULL);

    }
    break;
    case WM_TIMER:
    {
        ActivityUpdate();
        InvalidateRect(hWnd, nullptr, FALSE);
    }
    break;
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        static bool flag = true;
        switch (wmId)
        {
        case 1:
            if (LOWORD(wParam) == 1) {
                onButtonCommand(hWnd, HIWORD(wParam));
            }
            break;
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        case ID_ASSIST_BUTTON:
            if (flag) ShowWindow(hNewWnd, SW_SHOW);
            else ShowWindow(hNewWnd, SW_HIDE);
            flag = !flag;
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        BitBlt(hdc, 0, 0, CLIENT_WIDTH, CLIENT_HEIGHT, hMemDC, 0, 0, SRCCOPY);
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
    {
        ActivityDestroy();

        KillTimer(hWnd, 100);

        HBITMAP hBitmap = (HBITMAP)SelectObject(hMemDC, hMemPrev);
        DeleteObject(hBitmap);
        DeleteDC(hMemDC);

        PostQuitMessage(0);
    }
    break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// バージョン情報ボックスのメッセージ ハンドラーです。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
