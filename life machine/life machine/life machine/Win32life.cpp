#define _CRT_SECURE_NO_WARNINGS // Отключить сообщение о безопасности _itoa

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

bool CreateField = true;	// Флаг следяший за созданием нового поля
int SellSize = 32;			// Размер 1 ячейки

//// фигня отвечает за отрисовываемую область

int sizeOfMenuBar;			// = winRect.right - clientRect.right - winRect.left; 
int sizeOfBorder;			// = winRect.bottom - clientRect.bottom - winRect.top;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Главное окно
ATOM MyRegisterClass(HINSTANCE);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void InitOptionWindow();													// Создание модального окно
LRESULT CALLBACK WndProcChild(HWND, UINT, WPARAM, LPARAM);

void DrawBmp(HDC hdc, LPCTSTR PathToFile, int ZeroX, int ZeroY);			// Вывод bmp от точки (ZeroX, ZeroY)
void RedrawField(HDC hdc);													// Перерисовываем поле
void RestartGame(HDC hdc, int &gameStat, int &seconds);	                    // Обнуляет и начинает новую игру

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	newSize();	// Создание поля 10x10x10

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

//////////////////////////// нужная штука

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

	// Определяем и устанавливаем реальные размеры поля
	RECT winRect;
	GetWindowRect(hWndMain, &winRect); // вся область
	RECT clientRect;
	GetClientRect(hWndMain, &clientRect); // определяем то, где мы можем ривовать

	sizeOfBorder = winRect.right - clientRect.right - winRect.left;
	sizeOfMenuBar = winRect.bottom - clientRect.bottom - winRect.top;

	SetWindowPos(hWndMain, HWND_TOP, 0, 0, SellSize * width + sizeOfBorder, SellSize * height + sizeOfMenuBar, SWP_NOMOVE);

	ShowWindow(hWndMain, SW_SHOW);
	UpdateWindow(hWndMain);

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HDC hdcMem;					// offScreen для двойной буфферизации
	static HBITMAP hbmMem;              // тут хранится полное изображение на контексте
	static HANDLE hOld;                 // тут старый вид окна

	static PAINTSTRUCT ps;
	static HDC hdc;

	static int gameStat;				// Состояние игры. 0 -> идет. 1 -> победа. -1 -> проигрыш
	static int seconds;					// Секунды от страта таймера

	/// магия
	static bool newOffScreenDC = true;	// Флаг создания нового буффера для двойной буфферизации

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
						 // Определяем ячейку
						 int j = LOWORD(lParam) / SellSize;		// j - ячейка по x
						 int i = HIWORD(lParam) / SellSize; 		// i - ячейка по y

						 if (i >= 0 && j >= 0 && i < height  && j < width) // Координаты попадают в поле
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
					   if (wParam == ID_OPTIONS) // Открыть дочернее окно с параметрами
					   {
						   EnableWindow(hWndMain, FALSE); // Делаем неактивным главное окно

						   InitOptionWindow(); // Создаем окно. Если надо пересоздать поле CreateField -> true

						   if (CreateField)
						   {
							   KillTimer(hWnd, 1); // Сбрасиваем таймер
							   SetWindowPos(hWnd, HWND_TOP, 0, 0, SellSize * width + sizeOfBorder, SellSize * height + sizeOfMenuBar, SWP_SHOWWINDOW | SWP_NOMOVE);
							   InvalidateRect(hWnd, NULL, FALSE);	// Перерисовываем
						   }
						   //SetTimer(hWndMain, 1, 5000, NULL);
					   }
					   else if (wParam == ID_NEW)
					   {
						   KillTimer(hWnd, 1); // Сбрасиваем таймер
						   clearField();
						   InvalidateRect(hWnd, NULL, FALSE); // Перерисовываем
					   }
					   else if (wParam == ID_START)
					   {
						   SetTimer(hWndMain, 1, 1000, NULL);
					   }
	}	break;

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	case WM_SIZE: // При изменении размера создаем off-screen DC
	{
					  /// эта магия для того чтоб при сворачивании мы заново не рисовали окно а пользовались старой отрисовкой

					  if (wParam == SIZE_MINIMIZED) // Не создаем нового экрана gосле минимизации
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

						  SetBkColor(hdcMem, RGB(249, 128, 182)); // Черный фон
						  newOffScreenDC = true;
					  }
					  else
					  {
						  newOffScreenDC = true;
					  }
	}	break;
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	case WM_DESTROY: // При закрытии освобождаем память
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

	RECT rc;                        //Определение размеров родительского окна
	GetWindowRect(hWndMain, &rc);

	/// окно дочернее создается под опциями

	hWndChild = CreateWindowA(szWindowClassChild, szTitleChild,
		WS_POPUP | WS_VISIBLE | DS_MODALFRAME,
		rc.left + sizeOfBorder / 2, rc.top + sizeOfMenuBar - sizeOfBorder / 2, 120, 120,
		hWndMain, NULL, GetModuleHandle(NULL), NULL);

	ShowWindow(hWndChild, SW_SHOW);
	UpdateWindow(hWndChild);

	/// цикл обработки сообщений
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	UnregisterClass(szWindowClassChild, GetModuleHandle(NULL)); //// надо проверитьь шо будет если его грохнуть
}


LRESULT CALLBACK WndProcChild(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND ChildButtonGenerate, ChildButtonDefault, ChildButtonStart,
		ChildEditVSize, ChildEditHSize;

	static int NumberOfDigits = 4; //вводимое количество цифр
	static char * temp;            // куда вводим

	PAINTSTRUCT ps;
	HDC hdc;
	RECT rc;

	switch (message)
	{
	case WM_CREATE:
	{
					  temp = new char[NumberOfDigits + 1];
					  ChildEditVSize = CreateWindow("edit", _itoa(height, temp, 10), WS_VISIBLE | WS_CHILD | ES_NUMBER | DT_CENTER,
						  50, 2, 45, 16, hWnd, NULL, GetModuleHandle(NULL), NULL); // создает поля 
					  /// итоа выводит уже заданый параметр
					  ChildEditHSize = CreateWindowA("edit", _itoa(width, temp, 10), WS_VISIBLE | WS_CHILD | ES_NUMBER | DT_CENTER,
						  50, 22, 45, 16, hWnd, NULL, GetModuleHandle(NULL), NULL);



					  SendMessage(ChildEditVSize, EM_SETLIMITTEXT, 3, 0);	// Ограничение на количество символов = 3
					  SendMessage(ChildEditHSize, EM_SETLIMITTEXT, 3, 0);

					  ChildButtonGenerate = CreateWindowA("button", "Create", WS_VISIBLE | WS_CHILD, 2, 60, 116, 20, hWnd, NULL, GetModuleHandle(NULL), NULL);
					  ChildButtonDefault = CreateWindowA("button", "Default", WS_VISIBLE | WS_CHILD, 2, 80, 116, 20, hWnd, NULL, GetModuleHandle(NULL), NULL);
					  ChildButtonStart = CreateWindowA("button", "Start", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 2, 100, 116, 20, hWnd, NULL, GetModuleHandle(NULL), NULL);
	}	break;

	case WM_PAINT:
	{
					 hdc = BeginPaint(hWnd, &ps);

					 SetBkColor(hdc, RGB(255, 255, 255));            //Фон за тестом
					 SetTextColor(hdc, RGB(249, 128, 182));

					 /// выводим текст

					 rc.top = 1; rc.left = 2; rc.bottom = 20; rc.right = 140; ///// координаты между которыми выводится текст
					 DrawText(hdc, "Height", 6, &rc, DT_LEFT);

					 rc.top = 21; rc.left = 2; rc.bottom = 40; rc.right = 140;
					 DrawText(hdc, "Width", 5, &rc, DT_LEFT);

					 EndPaint(hWnd, &ps);
	}	break;

	case WM_COMMAND:
	{
					   if (ChildButtonGenerate == (HWND)lParam) /// кнопка создать
					   {
						   int tempHeight, tempWidth;

						   GetWindowText(ChildEditVSize, temp, 5);
						   tempHeight = atoi(temp);
						   GetWindowText(ChildEditHSize, temp, 5);
						   tempWidth = atoi(temp);

						   //// не даем юзеру издеваться над полем

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

						   SendMessage(hWndChild, WM_CLOSE, NULL, NULL); //// закрываем окошко
					   }

					   else if (ChildButtonDefault == (HWND)lParam) /// кнопка по умолчанию

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
					 delete[] temp;                /// удаляем временный массив
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
//// отрисовывает картиночки
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
///// перерисовывает поле
void RedrawField(HDC hdc)
{
	for (int i = 0; i < height; i++) // Отрисовываем изминившиеся клетки
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
/////// функция все обнулят
void RestartGame(HDC hdc, int &gameStat, int &seconds)
{

	for (int i = 0; i < height; i++) // Нарисовать закрытые ячейки
	for (int j = 0; j < width; j++)
		DrawBmp(hdc, MAKEINTRESOURCE(IDB_BITMAP1), (SellSize * j), (SellSize * i));

}