#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <math.h>


// Global variable
HINSTANCE hInst;
UINT  MessageCount = 0;
UINT  Count = 0;
int posX = 0;
int posY = 0;

COLORREF color = RGB(0, 0, 0);

// Function prototypes.
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
void paintFace(HWND hWnd, HDC hDC, PAINTSTRUCT ps, int posX, int posY, POINT cursorPosition);
void paintObjects(HWND hWnd, HDC hDC, PAINTSTRUCT ps, int posX, int posY, POINT cursorPosition);
void paintPosition(HWND hWnd, HDC hDC, PAINTSTRUCT ps, POINT cursorPosition);

// Application entry point. This is the same as main() in standart C.
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;
	BOOL bRet;
	WNDCLASS wcx;          // register class
	HWND hWnd;

	hInst = hInstance;     // Save the application-instance handle.
		// Fill in the window class structure with parameters that describe the main window.

	wcx.style = CS_HREDRAW | CS_VREDRAW;              // redraw if size changes
	wcx.lpfnWndProc = (WNDPROC)MainWndProc;          // points to window procedure
	wcx.cbClsExtra = 0;                               // no extra class memory
	wcx.cbWndExtra = 0;                               // no extra window memory
	wcx.hInstance = hInstance;                        // handle to instance
	wcx.hIcon = LoadIcon(NULL, IDI_APPLICATION);      // predefined app. icon
	wcx.hCursor = LoadCursor(NULL, IDC_ARROW);        // predefined arrow
	wcx.hbrBackground = GetStockObject(WHITE_BRUSH);  // white background brush
	wcx.lpszMenuName = (LPCSTR)"MainMenu";          // name of menu resource
	wcx.lpszClassName = (LPCSTR)"MainWClass";        // name of window class

	// Register the window class.

	if (!RegisterClass(&wcx)) return FALSE;

	// create window of registered class

	hWnd = CreateWindow(
		"MainWClass",        // name of window class
		"ITU",               // title-bar string
		WS_OVERLAPPEDWINDOW, // top-level window
		200,                  // default horizontal position
		25,                 // default vertical position
		1000,                // default width
		700,                 // default height
		(HWND)NULL,         // no owner window
		(HMENU)NULL,        // use class menu
		hInstance,           // handle to application instance
		(LPVOID)NULL);      // no window-creation data
	if (!hWnd) return FALSE;

	// Show the window and send a WM_PAINT message to the window procedure.
	// Record the current cursor position.

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	// loop of message processing
	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
	{
		if (bRet == -1)
		{
			// handle the error and possibly exit
		}
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return (int)msg.wParam;
}


LRESULT CALLBACK MainWndProc(
	HWND hWnd,        // handle to window
	UINT uMsg,        // message identifier
	WPARAM wParam,    // first message parameter
	LPARAM lParam)    // second message parameter
{
	HDC         hDC;
	PAINTSTRUCT ps;
	POINT cursorPosition;

	// init cursor position 
	GetCursorPos(&cursorPosition);
	ScreenToClient(hWnd, &cursorPosition);

	switch (uMsg)
	{
	case WM_CREATE:
		break;

	// character input 
	case WM_CHAR:
		switch (wParam) {
		case 0x08:  // backspace
		case 0x0A:  // linefeed
		case 0x1B:  // escape
			break;

		case 0x09:  // tab
			break;

		default:
			break;
		}
		break;

	// key input
	case WM_KEYDOWN:
		switch (wParam) {
	        // update posX and posY in order to move object
		case VK_LEFT: // left arrow
			posX -= 5;
			InvalidateRect(hWnd, NULL, TRUE);
			break;
		case VK_RIGHT: // right arrow
			posX += 5;
			InvalidateRect(hWnd, NULL, TRUE);
			break;
		case VK_UP: // up arrow
			posY -= 5;
			InvalidateRect(hWnd, NULL, TRUE);
			break;
		case VK_DOWN: // down arrow
			posY += 5;
			InvalidateRect(hWnd, NULL, TRUE);
			break;

		// react on the other pressed keys 
		case 0x52:	// R
			color = RGB(255, 0, 0);
			InvalidateRect(hWnd, NULL, TRUE);
			break;
		case 0x47:
			color = RGB(0, 255, 0);
			InvalidateRect(hWnd, NULL, TRUE);
			break;
		case 0x42:
			color = RGB(0, 0, 255);
			InvalidateRect(hWnd, NULL, TRUE);
			break;
		}
		break;

	// get cursor position 
	case WM_MOUSEMOVE:
		break;

	// react on mouse clicks
	case WM_LBUTTONDOWN:
		break;



	case WM_LBUTTONUP:
		break;

	// paint objects
	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ps);
		paintFace(hWnd, hDC, ps, posX, posY, cursorPosition);
		paintPosition(hWnd, hDC, ps, cursorPosition);
		paintObjects(hWnd, hDC, ps, posX, posY, cursorPosition);
		EndPaint(hWnd, &ps);
		DeleteDC(hDC);
		break;

		//
		// Process other messages.
		//

	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
}

void paintObjects(HWND hWnd, HDC hDC, PAINTSTRUCT ps, int posX, int posY, POINT cursorPosition)
{

	RECT to_update = { 50+posX, 50+posY, posX+540, posY+100 };

	HPEN pen = CreatePen(PS_SOLID, 3, color);
	SelectObject(hDC, GetStockObject(DC_BRUSH));
	SetDCBrushColor(hDC, color);

	Rectangle(hDC, 50 + posX, 50 + posY, posX + 100, posY + 100);


	MoveToEx(hDC, posX + 200, posY + 50, NULL);
	LineTo(hDC, posX + 250, posY + 150);

	Ellipse(hDC, posX + 300, posY + 50, posX + 400, posY + 100);

	POINT vertices[] = { {posX + 500, posY + 50}, {posX + 540, posY + 55}, {posX + 500, posY + 100} };
	Polygon(hDC, vertices, 3);
	
	//InvalidateRect(hWnd, &to_update, FALSE);
	DeleteObject(pen);
	return;
}


void paintFace(HWND hWnd, HDC hDC, PAINTSTRUCT ps, int posX, int posY, POINT cursorPosition)
{
	
	RECT to_update = { 200, 300, 320, 450 };
	
	HPEN pen = CreatePen(PS_SOLID, 3, RGB(255, 192, 203));
	SelectObject(hDC, GetStockObject(DC_BRUSH));
	SetDCBrushColor(hDC, RGB(255, 192, 203));
	Ellipse(hDC, 200, 300, 320, 450);  // tvar
	DeleteObject(pen);

	pen = CreatePen(PS_SOLID, 3, RGB(0, 0, 0));
	SelectObject(hDC, GetStockObject(DC_BRUSH));
	SetDCBrushColor(hDC, color);
	Ellipse(hDC, 230, 350, 240, 360);  // L oko
	Ellipse(hDC, 280, 350, 290, 360);  // R oko
	DeleteObject(pen);

	pen = CreatePen(PS_SOLID, 3, RGB(0, 0, 0));
	SelectObject(hDC, GetStockObject(DC_BRUSH));
	SetDCBrushColor(hDC, RGB(0, 0, 0));
	Rectangle(hDC, 255, 370, 265, 400);  // nos


	Rectangle(hDC, 230, 415, 290, 425); // usta

	DeleteObject(pen);


	//InvalidateRect(hWnd, &to_update, FALSE);
	
	return;
}

void paintPosition(HWND hWnd, HDC hDC, PAINTSTRUCT ps, POINT cursorPosition)
{
	char        text[256];          // buffer to store an output text
	HFONT       font;              // new large font
	HFONT       oldFont;           // saves the previous font

	font = CreateFont(25, 0, 0, 0, 0, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, 0);
	oldFont = (HFONT)SelectObject(hDC, font);
	sprintf(text, "Position -- x:%d, y:%d, Color: %d", cursorPosition.x, cursorPosition.y, color);
	TextOut(hDC, 50, 600, text, (int)strlen(text));
	SelectObject(hDC, oldFont);
	DeleteObject(font);

	RECT to_update = { 50, 670, 400, 500 };
	//InvalidateRect(hWnd, &to_update, FALSE);
}
