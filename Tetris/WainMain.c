#include <Windows.h>
#include <time.h>
#include "resource.h"
#include "Tetris.h"

HINSTANCE hInstance;
TCHAR szAppName[] = TEXT("俄罗斯方块");

HFONT FontInitialize(HWND hwnd);//初始化窗口字体
void ShowBitmap(HWND hwnd, RECT rect, TCHAR *szBitmapName);	//显示暂停/结束位图
BOOL CALLBACK DlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR szCmd, int nShow)
{
	HWND hwnd;
	MSG msg;
	WNDCLASSEX wcls;

	wcls.cbSize = sizeof(wcls);
	wcls.style = CS_HREDRAW | CS_VREDRAW;
	wcls.lpfnWndProc = WndProc;
	wcls.hInstance = hInst;
	wcls.cbWndExtra = 0;
	wcls.cbClsExtra = 0;
	wcls.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);//(COLOR_BTNFACE + 1);
	wcls.hIcon = LoadIcon(hInst, szAppName);
	wcls.hIconSm = LoadIcon(hInst, szAppName);
	wcls.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcls.lpszClassName = szAppName;
	wcls.lpszMenuName = szAppName;

	if (!RegisterClassEx(&wcls)){
		MessageBox(NULL, TEXT("注册类出错！"), TEXT("错误"), MB_ICONERROR);
		return 0;
	}

	hInstance = hInst;

	hwnd = CreateWindow(szAppName,
		szAppName,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		400,
		500,
		NULL, NULL,
		hInst, NULL);
	ShowWindow(hwnd, nShow);
	UpdateWindow(hwnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static RECT rect;
	static HFONT hfont;
	static BOOL bGameBegin, bGameOver, bGamePause;
	static int cxChar, cyChar, iBlockSize, score, iTime;
	int iCleanRows;
	HDC hdc;
	HMENU hmenu;
	PAINTSTRUCT ps;
	TEXTMETRIC txm;

	switch (message)
	{
	case WM_CREATE:
		srand((UINT)time(NULL));
		hfont = FontInitialize(hwnd);

		hdc = GetDC(hwnd);
		GetTextMetrics(hdc, &txm);
		cyChar = txm.tmHeight + txm.tmExternalLeading;
		ReleaseDC(hwnd, hdc);

		return 0;
	case WM_SIZE:
		GetClientRect(hwnd, &rect);
		rect.left = cyChar;
		rect.top = cyChar;
		iBlockSize = (rect.bottom - cyChar - 1) / VERT_BLK_NUMS;//初始化方块的大小
		rect.right = iBlockSize * HORZ_BLK_NUMS + rect.left;
		rect.bottom = iBlockSize * VERT_BLK_NUMS + rect.top;
		return 0;
	case WM_INITMENUPOPUP:
		hmenu = GetMenu(hwnd);
		switch (LOWORD(lParam))
		{
		case 0:
			EnableMenuItem(hmenu, IDM_PAUSE, bGameBegin && !bGamePause ? MF_ENABLED : MF_DISABLED);
			EnableMenuItem(hmenu, IDM_RESUME, bGameBegin && bGamePause ? MF_ENABLED : MF_DISABLED);
		}
		return 0;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDM_START:
			ClientInit(&score);//初始化客户区数据
			NextBlockInit();//生成新的方块
			NewBlockEntry();//新方块放入游戏区

			hdc = GetDC(hwnd);
			PaintClient(hwnd, rect, iBlockSize, score, hfont);//显示客户区
			ReleaseDC(hwnd, hdc);

			iTime = 1000;//控制下落的速度
			bGameBegin = TRUE;
			bGameOver = bGamePause = FALSE;
			SetTimer(hwnd, ID_TIMER, iTime, NULL);
			break;
		case IDM_PAUSE:
			bGamePause = TRUE;
			ShowBitmap(hwnd, rect, TEXT("暂停"));
			KillTimer(hwnd, ID_TIMER);
			break;
		case IDM_RESUME:
			bGamePause = FALSE;
			InvalidateRect(hwnd, NULL, TRUE);
			SetTimer(hwnd, ID_TIMER, iTime, NULL);
			break;
		case IDM_ABOUT:
			DialogBox(hInstance, szAppName, hwnd, DlgProc);
			break;
		case IDM_HELP:
			MessageBox(hwnd, TEXT("点击“开始游戏”开始\n方向键“左、右、下”控制方块移动\n“空格键”控制方块翻转"),
				szAppName, MB_ICONINFORMATION);
			break;
		case IDM_EXIT:
			SendMessage(hwnd, WM_CLOSE, 0, 0);
			break;
		}
		return 0;
	case WM_KEYDOWN:
		if (!bGameBegin || bGamePause)
			break;
		switch (wParam)
		{
		case VK_LEFT:
		case VK_RIGHT:
		case VK_SPACE:
			PaintNowPos(hwnd, rect, iBlockSize, FALSE);//清除原位置图形
			CanToNextPos(wParam);
			PaintNowPos(hwnd, rect, iBlockSize, TRUE);//绘制新位置图形
			return 0;
		case VK_DOWN:
			SetTimer(hwnd, ID_TIMER, 1, NULL);//按下↓按键后快速下落
			return 0;
		}
		break;
	case WM_TIMER:
		PaintNowPos(hwnd, rect, iBlockSize, FALSE);//清除原位置图形
		if (!CanToNextPos(VK_DOWN))//判断活动方块是否下落到底
		{
			iCleanRows = CleanRow();
			score += iCleanRows;
			iTime = 1000 - score * 10;//加快下落速度
			SetTimer(hwnd, ID_TIMER, iTime, NULL);//结束快速下落，更改成新的速度
			if (iCleanRows)
				//消除了若干列，游戏区需重新绘制；否则只绘制活动方块已降低窗口闪烁！
				PaintClient(hwnd, rect, iBlockSize, score, hfont);
			else
				PaintNowPos(hwnd, rect, iBlockSize, TRUE);//绘制原位置图形
			if (!NewBlockEntry())//判断游戏是否结束
			{//游戏结束！
				bGameOver = GameOver(hwnd, &bGameBegin);
				ShowBitmap(hwnd, rect, TEXT("游戏结束"));
				return 0;
			}
			//游戏未结束！
			PaintNextBlock(hwnd, rect, iBlockSize);//绘制新生成的“下个”方块图形
		}
		PaintNowPos(hwnd, rect, iBlockSize, TRUE);//绘制新位置图形
		return 0;
	case WM_PAINT:
		BeginPaint(hwnd, &ps);

		PaintClient(hwnd, rect, iBlockSize, score, hfont);
		if (bGamePause)
			ShowBitmap(hwnd, rect, TEXT("暂停"));
		if (bGameOver)
			ShowBitmap(hwnd, rect, TEXT("游戏结束"));

		EndPaint(hwnd, &ps);
		return 0;
	case WM_DESTROY:
		KillTimer(hwnd, ID_TIMER);
		DeleteObject(hfont);
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}
//初始化窗口字体
HFONT FontInitialize(HWND hwnd)
{
	static LOGFONT logfont;

	wcscpy_s(logfont.lfFaceName, sizeof(logfont.lfFaceName),TEXT("宋体"));
	logfont.lfHeight = 14;

	return CreateFontIndirect(&logfont);
}
//绘制相应位图
void ShowBitmap(HWND hwnd, RECT rect, TCHAR *szBitmapName)
{
	HDC hdc, hdcMem, hdcMask;
	BITMAP bitmap;
	HBITMAP hBitmap, hBitmapMask;

	hdc = GetDC(hwnd);
	//载入位图，计算位图大小
	hBitmap = LoadBitmap(hInstance, szBitmapName);
	GetObject(hBitmap, sizeof(BITMAP), &bitmap);
	//创建内存设备环境，选入位图
	hdcMem = CreateCompatibleDC(hdc);
	SelectObject(hdcMem, hBitmap);
	//创建单色位图，并选入新的内存环境
	hBitmapMask = CreateBitmap(bitmap.bmWidth, bitmap.bmHeight, 1, 1, NULL);
	hdcMask = CreateCompatibleDC(hdc);
	SelectObject(hdcMask, hBitmapMask);
	//在单色内存设备环境中创建屏蔽位图：背景色为1，其他色为0
	SetBkColor(hdcMem, RGB(255, 255, 255));
	BitBlt(hdcMask, 0, 0, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);
	//让目标位图背景色变为0
	BitBlt(hdcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, hdcMask, 0, 0, 0x220326);
	//窗口目标位置颜色变为黑色（0）
	BitBlt(hdc,
		rect.left + (rect.right - rect.left - bitmap.bmWidth) / 2,
		rect.top + (rect.bottom - rect.top - bitmap.bmHeight) / 2,
		bitmap.bmWidth, bitmap.bmHeight, hdcMask, 0, 0, SRCAND);
	//将源位图和窗口目标区域相“或”
	BitBlt(hdc,
		rect.left + (rect.right - rect.left - bitmap.bmWidth) / 2,
		rect.top + (rect.bottom - rect.top - bitmap.bmHeight) / 2,
		bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCPAINT);

	DeleteDC(hdcMem);
	DeleteDC(hdcMask);
	DeleteObject(hBitmap);
	DeleteObject(hBitmapMask);
	ReleaseDC(hwnd, hdc);
	return;
}

//游戏结束后的相应操作
BOOL GameOver(HWND hwnd, BOOL *bGameBegin)
{
	*bGameBegin = FALSE;
	KillTimer(hwnd, ID_TIMER);
	MessageBeep(MB_ICONWARNING);
	return TRUE;
}
//“关于” 对话框消息处理函数
BOOL CALLBACK DlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
		case IDCANCEL:
			EndDialog(hwnd, 0);
			return TRUE;
		}
		break;
	}
	return FALSE;
}