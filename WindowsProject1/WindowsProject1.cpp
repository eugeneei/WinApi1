#include <windows.h>
#include <wchar.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <cmath>
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

using namespace Gdiplus;


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void CenterWindow(HWND);
void DrawBitmap(HWND);
void UpdateSprite(RECT* rect);
void DrawSprite(HDC hdc, RECT* rect);
void OnTimer(HWND hwnd);

#define LEFT_ARROW_ID 1
#define RIGHT_ARROW_ID 2
#define UP_ARROW_ID 3
#define DOWN_ARROW_ID 4
#define SPRITE_MOVE_DELTA 12
#define TIMER_ID 13


typedef struct _SPRITEINFO
{
	int width;
	int height;
	int x;
	int y;

	int dx;
	int dy;
}SPRITEINFO;

SPRITEINFO spriteInfo;
HBITMAP bitmap = NULL;


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PWSTR pCmdLine, int nCmdShow) {

	MSG  msg;
	HWND hwnd;
	WNDCLASSW wc;

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.lpszClassName = L"Window";
	wc.hInstance = hInstance;
	wc.hbrBackground = GetSysColorBrush(COLOR_3DSHADOW);
	wc.lpszMenuName = NULL;

	wc.lpfnWndProc = WndProc;

	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);

	RegisterClassW(&wc);

	int width = 800;
	int height = 600;

	hwnd = CreateWindowW(wc.lpszClassName, L"Sprite",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		100, 100, width, height, NULL, NULL, hInstance, NULL);

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	while (GetMessage(&msg, NULL, 0, 0)) {

		DispatchMessage(&msg);
	}


	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg,
	WPARAM wParam, LPARAM lParam) {

	static int clientWidth;
	static int clientHeight;

	static int spriteWidth;
	static int spriteHeight;

	static int step = 5;
	static int x, y;


	switch (msg) {

	case WM_SIZE:
	{
		clientWidth = LOWORD(lParam);
		clientHeight = HIWORD(lParam);
		break;
	}
	case WM_PAINT:
		DrawBitmap(hwnd);
		break;

	case WM_HOTKEY:
		CenterWindow(hwnd);

		/*

		if ((wParam) == ARROW_ID) {


			Debug(hwnd, )
		}*/

		break;

	case WM_CREATE:
	{
		Image temp(L"sprite.bmp");
		spriteHeight = temp.GetHeight();
		spriteWidth = temp.GetWidth();
		
		RegisterHotKey(hwnd, LEFT_ARROW_ID, NULL, VK_LEFT);
		RegisterHotKey(hwnd, RIGHT_ARROW_ID, NULL, VK_RIGHT);
		RegisterHotKey(hwnd, UP_ARROW_ID, NULL, VK_UP);
		RegisterHotKey(hwnd, DOWN_ARROW_ID, NULL, VK_DOWN);

		bitmap = (HBITMAP)LoadImage(NULL, L"sprite.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		if (bitmap == NULL)
			MessageBox(hwnd, L"Could not load image file!", L"Error", MB_OK | MB_ICONEXCLAMATION);

		if (SetTimer(hwnd, TIMER_ID, 50, NULL) == 0)
			MessageBox(hwnd, L"Could not SetTimer()!", L"Error", MB_OK | MB_ICONEXCLAMATION);




		break;
	}

	case WM_MOUSEWHEEL:
	{
		int wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		if (LOWORD(wParam) == MK_CONTROL)
		{
			for (; wheelDelta > WHEEL_DELTA; wheelDelta -= WHEEL_DELTA)
			{
				if (x < clientWidth - spriteWidth / 2)
				{
					x += step;
					InvalidateRect(hwnd, NULL, true);
				}
			}
			for (; wheelDelta < 0; wheelDelta += WHEEL_DELTA)
			{
				if (x > step)
				{
					x -= step;
					InvalidateRect(hwnd, NULL, true);
				}
			}
		}
		else
		{
			for (; wheelDelta > WHEEL_DELTA; wheelDelta -= WHEEL_DELTA)
			{
				if (y < clientHeight - spriteHeight / 2 - step)
				{
					y += step;
					InvalidateRect(hwnd, NULL, true);
				}
			}
			for (; wheelDelta < 0; wheelDelta += WHEEL_DELTA)
			{
				if (y > spriteHeight / 2 + step)
				{
					y -= step;
					InvalidateRect(hwnd, NULL, true);
				}
			}
		}
		break;
	}


	case WM_TIMER:
		OnTimer(hwnd);
		break;

	case WM_DESTROY:
		DeleteObject(bitmap);
		KillTimer(hwnd, TIMER_ID);
		PostQuitMessage(0);
		break;
	}

	return DefWindowProcW(hwnd, msg, wParam, lParam);
}

void OnTimer(HWND hwnd) {
	RECT rcClient;
	HDC hdc = GetDC(hwnd);

	GetClientRect(hwnd, &rcClient);

	UpdateSprite(&rcClient);
	DrawSprite(hdc, &rcClient);

	ReleaseDC(hwnd, hdc);

}

void DrawBitmap(HWND hwnd) {
	BITMAP bm;
	PAINTSTRUCT ps;

	HDC hdc = BeginPaint(hwnd, &ps);

	HDC hdcMem = CreateCompatibleDC(hdc);
	HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, bitmap);

	GetObject(bitmap, sizeof(bm), &bm);

	ZeroMemory(&spriteInfo, sizeof(spriteInfo));
	spriteInfo.width = bm.bmWidth;
	spriteInfo.height = bm.bmHeight;

	spriteInfo.dx = SPRITE_MOVE_DELTA;
	spriteInfo.dy = SPRITE_MOVE_DELTA;

	BitBlt(hdc, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);

	SelectObject(hdcMem, hbmOld);
	DeleteDC(hdcMem);

	EndPaint(hwnd, &ps);
}

void UpdateSprite(RECT* rect)
{
	spriteInfo.x += spriteInfo.dx;
	spriteInfo.y += spriteInfo.dy;

	if (spriteInfo.x < 0)
	{
		spriteInfo.x = 0;
		spriteInfo.dx = SPRITE_MOVE_DELTA;
	}
	else if (spriteInfo.x + spriteInfo.width > rect->right)
	{
		spriteInfo.x = rect->right - spriteInfo.width;
		spriteInfo.dx = -SPRITE_MOVE_DELTA;
	}

	if (spriteInfo.y < 0)
	{
		spriteInfo.y = 0;
		spriteInfo.dy = SPRITE_MOVE_DELTA;
	}
	else if (spriteInfo.y + spriteInfo.height > rect->bottom)
	{
		spriteInfo.y = rect->bottom - spriteInfo.height;
		spriteInfo.dy = -SPRITE_MOVE_DELTA;
	}
}

void DrawSprite(HDC hdc, RECT* rect)
{
	HDC hdcBuffer = CreateCompatibleDC(hdc);
	HBITMAP hbmBuffer = CreateCompatibleBitmap(hdc, rect->right, rect->bottom);
	HBITMAP hbmOldBuffer = (HBITMAP)SelectObject(hdcBuffer, hbmBuffer);

	HDC hdcMem = CreateCompatibleDC(hdc);
	HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, bitmap);

	FillRect(hdcBuffer, rect, (HBRUSH)GetStockObject(WHITE_BRUSH));

	BitBlt(hdcBuffer, spriteInfo.x, spriteInfo.y, spriteInfo.width, spriteInfo.height, hdcMem, 0, 0, SRCAND);

	SelectObject(hdcMem, bitmap);
	BitBlt(hdcBuffer, spriteInfo.x, spriteInfo.y, spriteInfo.width, spriteInfo.height, hdcMem, 0, 0, SRCPAINT);

	BitBlt(hdc, 0, 0, rect->right, rect->bottom, hdcBuffer, 0, 0, SRCCOPY);

	SelectObject(hdcMem, hbmOld);
	DeleteDC(hdcMem);

	SelectObject(hdcBuffer, hbmOldBuffer);
	DeleteDC(hdcBuffer);
	DeleteObject(hbmBuffer);
}

void CenterWindow(HWND hwnd) {

	RECT rc = { 0 };

	GetWindowRect(hwnd, &rc);
	int win_w = rc.right - rc.left;
	int win_h = rc.bottom - rc.top;

	int screen_w = GetSystemMetrics(SM_CXSCREEN);
	int screen_h = GetSystemMetrics(SM_CYSCREEN);

	SetWindowPos(hwnd, HWND_TOP, (screen_w - win_w) / 2,
		(screen_h - win_h) / 2, 0, 0, SWP_NOSIZE);
}
#pragma clang diagnostic pop