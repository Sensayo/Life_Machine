#define _CRT_SECURE_NO_WARNINGS // ��������� ��������� � ������������ _itoa

#include <Windows.h>
#include "life machine.h"
#include "resource.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

HINSTANCE hInst;
HWND hWndMain, hWndChild;

LPCTSTR szWindowClass = "Main";
LPCTSTR szTitle = "Virus";
LPCTSTR szWindowClassChild = "Child";
LPCTSTR szTitleChild = "Options";

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CreateField = true;	// ���� �������� �� ��������� ������ ����
int SellSize = 32;			// ������ 1 ������

//// ����� �������� �� �������������� �������

int sizeOfMenuBar;			// = winRect.right - clientRect.right - winRect.left; 
int sizeOfBorder;			// = winRect.bottom - clientRect.bottom - winRect.top;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// ������� ����
ATOM MyRegisterClass(HINSTANCE);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void InitOptionWindow();													// �������� ���������� ����
LRESULT CALLBACK WndProcChild(HWND, UINT, WPARAM, LPARAM);

void DrawBmp(HDC hdc, LPCTSTR PathToFile, int ZeroX, int ZeroY);			// ����� bmp �� ����� (ZeroX, ZeroY)
void RedrawField(HDC hdc);													// �������������� ����
void RestartGame(HDC hdc, int &gameStat, int &seconds);	                    // �������� � �������� ����� ����

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	newSize();	// �������� ���� 10x10x10

	MSG msg;

	MyRegisterClass(hInstance);
	if (!InitInstance(hInstance, nCmdShow))
		return FALSE;

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = NULL;
	wcex.lpfnWndProc = (WNDPROC)WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(NULL, IDI_SHIELD);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = NULL;
	wcex.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = NULL;

	return RegisterClassExA(&wcex);
}

//////////////////////////// ������ �����

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance;

	hWndMain = CreateWindowA(szWindowClass, szTitle,
		WS_CAPTION | WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, hInstance, NULL);

	if (!hWndMain)
		return FALSE;

	// ���������� � ������������� �������� ������� ����
	RECT winRect;
	GetWindowRect(hWndMain, &winRect); // ��� �������
	RECT clientRect;
	GetClientRect(hWndMain, &clientRect); // ���������� ��, ��� �� ����� ��������

	sizeOfBorder = winRect.right - clientRect.right - winRect.left;
	sizeOfMenuBar = winRect.bottom - clientRect.bottom - winRect.top;

	SetWindowPos(hWndMain, HWND_TOP, 0, 0, SellSize * width + sizeOfBorder, SellSize * height + sizeOfMenuBar, SWP_NOMOVE);

	ShowWindow(hWndMain, SW_SHOW);
	UpdateWindow(hWndMain);

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HDC hdcMem;					// offScreen ��� ������� ������������
	static HBITMAP hbmMem;              // ��� �������� ������ ����������� �� ���������
	static HANDLE hOld;                 // ��� ������ ��� ����

	static PAINTSTRUCT ps;
	static HDC hdc;

	static int gameStat;				// ��������� ����. 0 -> ����. 1 -> ������. -1 -> ��������
	static int seconds;					// ������� �� ������ �������

	/// �����
	static bool newOffScreenDC = true;	// ���� �������� ������ ������� ��� ������� ������������

	switch (message)
	{
	case WM_PAINT:
	{
					 RedrawField(hdcMem);

					 hdc = BeginPaint(hWnd, &ps);
					 BitBlt(hdc, 0, 0, SellSize * width, SellSize * height, hdcMem, 0, 0, SRCCOPY);
					 EndPaint(hWnd, &ps);

	}	break;
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	case WM_TIMER:
	{
					 seconds = nextTime();
					 RedrawField(hdcMem);
					 hdc = GetDC(hWnd);
					 BitBlt(hdc, 0, 0, SellSize * width, SellSize * height, hdcMem, 0, 0, SRCCOPY);
					 ReleaseDC(hWnd, hdc);

					 if (seconds == 0)
					 {
						 KillTimer(hWnd, 1);
					 }

	}	break;
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	case WM_LBUTTONUP:
	{
						 // ���������� ������
						 int j = LOWORD(lParam) / SellSize;		// j - ������ �� x
						 int i = HIWORD(lParam) / SellSize; 		// i - ������ �� y

						 if (i >= 0 && j >= 0 && i < height  && j < width) // ���������� �������� � ����
						 {
							 field[i][j].value = true;
							 RedrawField(hdcMem);

							 hdc = GetDC(hWnd);
							 BitBlt(hdc, 0, 0, SellSize * width, SellSize * height, hdcMem, 0, 0, SRCCOPY);
							 ReleaseDC(hWnd, hdc);
						 }
	}	break;
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	case WM_COMMAND:
	{
					   if (wParam == ID_OPTIONS) // ������� �������� ���� � �����������
					   {
						   EnableWindow(hWndMain, FALSE); // ������ ���������� ������� ����

						   InitOptionWindow(); // ������� ����. ���� ���� ����������� ���� CreateField -> true

						   if (CreateField)
						   {
							   KillTimer(hWnd, 1); // ���������� ������
							   SetWindowPos(hWnd, HWND_TOP, 0, 0, SellSize * width + sizeOfBorder, SellSize * height + sizeOfMenuBar, SWP_SHOWWINDOW | SWP_NOMOVE);
							   InvalidateRect(hWnd, NULL, FALSE);	// ��������������
						   }
						   //SetTimer(hWndMain, 1, 5000, NULL);
					   }
					   else if (wParam == ID_NEW)
					   {
						   KillTimer(hWnd, 1); // ���������� ������
						   clearField();
						   InvalidateRect(hWnd, NULL, FALSE); // ��������������
					   }
					   else if (wParam == ID_START)
					   {
						   SetTimer(hWndMain, 1, 1000, NULL);
					   }
	}	break;

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	case WM_SIZE: // ��� ��������� ������� ������� off-screen DC
	{
					  /// ��� ����� ��� ���� ���� ��� ������������ �� ������ �� �������� ���� � ������������ ������ ����������

					  if (wParam == SIZE_MINIMIZED) // �� ������� ������ ������ g���� �����������
					  {
						  newOffScreenDC = false;
					  }
					  else if (wParam == SIZE_RESTORED && newOffScreenDC)
					  {
						  hdc = GetDC(hWnd);
						  hdcMem = CreateCompatibleDC(hdc);
						  hbmMem = CreateCompatibleBitmap(hdc, SellSize * width, SellSize * height);
						  hOld = SelectObject(hdcMem, hbmMem);
						  ReleaseDC(hWnd, hdc);

						  SetBkColor(hdcMem, RGB(249, 128, 182)); // ������ ���
						  newOffScreenDC = true;
					  }
					  else
					  {
						  newOffScreenDC = true;
					  }
	}	break;
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	case WM_DESTROY: // ��� �������� ����������� ������
	{
						 SelectObject(hdcMem, hOld);
						 DeleteObject(hbmMem);
						 DeleteDC(hdcMem);

						 PostQuitMessage(0);
	}	break;
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void InitOptionWindow()
{
	MSG msg;

	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = NULL;
	wcex.lpfnWndProc = (WNDPROC)WndProcChild;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = GetModuleHandle(NULL);
	wcex.hIcon = LoadIcon(NULL, IDI_HAND);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = CreateSolidBrush(RGB(249, 128, 182));
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClassChild;
	wcex.hIconSm = NULL;
	RegisterClassEx(&wcex);

	RECT rc;                        //����������� �������� ������������� ����
	GetWindowRect(hWndMain, &rc);

	/// ���� �������� ��������� ��� �������

	hWndChild = CreateWindowA(szWindowClassChild, szTitleChild,
		WS_POPUP | WS_VISIBLE | DS_MODALFRAME,
		rc.left + sizeOfBorder / 2, rc.top + sizeOfMenuBar - sizeOfBorder / 2, 120, 120,
		hWndMain, NULL, GetModuleHandle(NULL), NULL);

	ShowWindow(hWndChild, SW_SHOW);
	UpdateWindow(hWndChild);

	/// ���� ��������� ���������
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	UnregisterClass(szWindowClassChild, GetModuleHandle(NULL)); //// ���� ���������� �� ����� ���� ��� ��������
}


LRESULT CALLBACK WndProcChild(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND ChildButtonGenerate, ChildButtonDefault, ChildButtonStart,
		ChildEditVSize, ChildEditHSize;

	static int NumberOfDigits = 4; //�������� ���������� ����
	static char * temp;            // ���� ������

	PAINTSTRUCT ps;
	HDC hdc;
	RECT rc;

	switch (message)
	{
	case WM_CREATE:
	{
					  temp = new char[NumberOfDigits + 1];
					  ChildEditVSize = CreateWindow("edit", _itoa(height, temp, 10), WS_VISIBLE | WS_CHILD | ES_NUMBER | DT_CENTER,
						  50, 2, 45, 16, hWnd, NULL, GetModuleHandle(NULL), NULL); // ������� ���� 
					  /// ���� ������� ��� ������� ��������
					  ChildEditHSize = CreateWindowA("edit", _itoa(width, temp, 10), WS_VISIBLE | WS_CHILD | ES_NUMBER | DT_CENTER,
						  50, 22, 45, 16, hWnd, NULL, GetModuleHandle(NULL), NULL);



					  SendMessage(ChildEditVSize, EM_SETLIMITTEXT, 3, 0);	// ����������� �� ���������� �������� = 3
					  SendMessage(ChildEditHSize, EM_SETLIMITTEXT, 3, 0);

					  ChildButtonGenerate = CreateWindowA("button", "Create", WS_VISIBLE | WS_CHILD, 2, 60, 116, 20, hWnd, NULL, GetModuleHandle(NULL), NULL);
					  ChildButtonDefault = CreateWindowA("button", "Default", WS_VISIBLE | WS_CHILD, 2, 80, 116, 20, hWnd, NULL, GetModuleHandle(NULL), NULL);
					  ChildButtonStart = CreateWindowA("button", "Start", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 2, 100, 116, 20, hWnd, NULL, GetModuleHandle(NULL), NULL);
	}	break;

	case WM_PAINT:
	{
					 hdc = BeginPaint(hWnd, &ps);

					 SetBkColor(hdc, RGB(255, 255, 255));            //��� �� ������
					 SetTextColor(hdc, RGB(249, 128, 182));

					 /// ������� �����

					 rc.top = 1; rc.left = 2; rc.bottom = 20; rc.right = 140; ///// ���������� ����� �������� ��������� �����
					 DrawText(hdc, "Height", 6, &rc, DT_LEFT);

					 rc.top = 21; rc.left = 2; rc.bottom = 40; rc.right = 140;
					 DrawText(hdc, "Width", 5, &rc, DT_LEFT);

					 EndPaint(hWnd, &ps);
	}	break;

	case WM_COMMAND:
	{
					   if (ChildButtonGenerate == (HWND)lParam) /// ������ �������
					   {
						   int tempHeight, tempWidth;

						   GetWindowText(ChildEditVSize, temp, 5);
						   tempHeight = atoi(temp);
						   GetWindowText(ChildEditHSize, temp, 5);
						   tempWidth = atoi(temp);

						   //// �� ���� ����� ���������� ��� �����

						   if (tempHeight < 6)
							   tempHeight = 6;
						   else if (tempHeight > 45)
							   tempHeight = 45;

						   if (tempWidth < 6)
							   tempWidth = 6;
						   else if (tempWidth > 60)
							   tempWidth = 60;

						   newSize(tempHeight, tempWidth);

						   CreateField = true;

						   SendMessage(hWndChild, WM_CLOSE, NULL, NULL); //// ��������� ������
					   }

					   else if (ChildButtonDefault == (HWND)lParam) /// ������ �� ���������

					   {
						   CreateField = true;
						   newSize();

						   SendMessage(hWndChild, WM_CLOSE, NULL, NULL);
					   }
					   else if (ChildButtonStart == (HWND)lParam)
					   {
						   SendMessage(hWndChild, WM_CLOSE, NULL, NULL);
					   }
	}	break;

	case WM_CLOSE:
	{
					 delete[] temp;                /// ������� ��������� ������
					 EnableWindow(hWndMain, TRUE);
					 DestroyWindow(hWndChild);
	}	break;

	case WM_DESTROY:
	{
					   PostQuitMessage(0);
	}	break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}
//// ������������ ����������
void DrawBmp(HDC hdc, LPCTSTR PathToFile, int ZeroX, int ZeroY)
{
	BITMAP Bitmap;
	HBITMAP hOldBitmap;
	HDC hCompatibleDC = CreateCompatibleDC(hdc);

	HBITMAP hBitmap = (HBITMAP)LoadImage(hInst, PathToFile, IMAGE_BITMAP, 0, 0, NULL);
	GetObject(hBitmap, sizeof(BITMAP), &Bitmap);
	hOldBitmap = (HBITMAP)SelectObject(hCompatibleDC, hBitmap);
	StretchBlt(hdc, ZeroX, ZeroY, Bitmap.bmWidth, Bitmap.bmHeight, hCompatibleDC, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, SRCCOPY);

	SelectObject(hCompatibleDC, hOldBitmap);
	DeleteObject(hBitmap);
	DeleteDC(hCompatibleDC);
}
///// �������������� ����
void RedrawField(HDC hdc)
{
	for (int i = 0; i < height; i++) // ������������ ������������ ������
	for (int j = 0; j < width; j++)

		switch (Sell.value)
	{
		case false:
			DrawBmp(hdc, MAKEINTRESOURCE(IDB_BITMAP1), (SellSize * j), (SellSize * i));
			break;

		case true:
			DrawBmp(hdc, MAKEINTRESOURCE(IDB_BITMAP2), (SellSize * j), (SellSize * i));
			break;
	}


}
/////// ������� ��� �������
void RestartGame(HDC hdc, int &gameStat, int &seconds)
{

	for (int i = 0; i < height; i++) // ���������� �������� ������
	for (int j = 0; j < width; j++)
		DrawBmp(hdc, MAKEINTRESOURCE(IDB_BITMAP1), (SellSize * j), (SellSize * i));

}