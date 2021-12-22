#include <windows.h>

LRESULT CallWindowProcA(
	HWND Window,
	UINT Message,
	WPARAM WParam,
	LPARAM LParam)
{
	LRESULT Result = 0;
	switch (Message)
	{
	case WM_SIZE:
	{
		OutputDebugStringA("WM_SIZE\n");
	}
	break;
	case WM_DESTROY:
	{
		OutputDebugStringA("WM_SIZE\n");
	}
	break;
	case WM_CLOSE:
	{
		OutputDebugStringA("WM_SIZE\n");
	}
	break;
	case WM_ACTIVATEAPP:
	{
		OutputDebugStringA("WM_SIZE\n");
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT Paint;
		HDC DeviceContext = BeginPaint(Window, &Paint);

		RECT rect = Paint.rcPaint;

		int Width = rect.right - rect.left;
		int Height = rect.bottom - rect.top;
		int X = rect.left;
		int Y = rect.top;

		static DWORD operation = WHITENESS;

		if (operation == WHITENESS)
		{
			operation = BLACKNESS;
		}
		else
		{
			operation = WHITENESS;
		}

		PatBlt(DeviceContext, X, Y, Width, Height, operation);

		EndPaint(Window, &Paint);
	}
	break;
	default:
	{
		Result = DefWindowProc(Window, Message, WParam, LParam);
		//OutputDebugStringA("WM_SIZE\n");
	}
	break;
	}
	return (Result);
}
int CALLBACK
WinMain(HINSTANCE hInstance,
		HINSTANCE hPrevInstance,
		LPSTR lpCmdLine,
		int nShowCmd)
{
	WNDCLASS windowClass = {};

	windowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = CallWindowProcA;
	windowClass.hInstance = hInstance;
	//windowClass.hIcon;
	windowClass.lpszClassName = "HandmadeHeroWindowClass";

	if (RegisterClass(&windowClass))
	{
		HWND WindowHandle = CreateWindowEx(
			0,
			windowClass.lpszClassName,
			"Handmade Hero",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			0,
			0,
			hInstance,
			0);
		if (WindowHandle)
		{

			for (;;)
			{
				MSG Message;
				BOOL MessageResult = GetMessage(&Message, 0, 0, 0);
				if (MessageResult > 0)
				{
					TranslateMessage(&Message);
					DispatchMessage(&Message);
				}
				else
				{
					break;
				}
			}
		}
		else
		{
		}
	}
	else
	{
	}

	return (0);
}
