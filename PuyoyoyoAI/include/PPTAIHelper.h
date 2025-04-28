#pragma once

#include <Windows.h>

#ifdef PPTAIHELPERLIBRARY_EXPORTS
#define PPTAIHELPER_API __declspec(dllexport)
#else
#define PPTAIHELPER_API __declspec(dllimport)
#endif

class PPTAIHELPER_API PPTAIHelper
{
private:
	HDC CaptureScreenDC;
	HBITMAP CaptureScreenPrev;
	LPBYTE StartPixel;
	int CaptureX;
	int CaptureY;
	int CaptureWidth;
	int CaptureHeight;
	RECT TrimRect;

	HBITMAP CreateDibBuffer(int width, int height);
	void CreateCaptureScreen(int width, int height);
	void DestroyCaptureScreen();
public:
	PPTAIHelper();
	~PPTAIHelper();
	void SetWindowForeground();
	void SetWindowPosition(int x, int y);
	void SetClientSize(int width, int height);
	void SetCapturePosition();
	void SetCaptureSize();
	void TrimCaptureBound(int left, int top, int right, int bottom);
	void CaptureScreen();
	void DrawCapturedScreen(HDC hdc, int x, int y);
	void GetPixelColor(int x, int y, BYTE& r, BYTE& g, BYTE& b);
	int GetCaptureWidth();
	int GetCaptureHeight();
	void InputKey(WORD virtualKeyCode, bool isExtended, bool isPressing);
};

