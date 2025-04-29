#pragma once

void ActivityUpdate();
void ActivityCreate();
void ActivityDestroy();
void onButtonCommand(HWND hWnd, WORD code);

void DrawString(HDC hdc, int x, int y, WCHAR* format, ...);
