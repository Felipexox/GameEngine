#include <windows.h>
#include <stdint.h>
#include <xinput.h>

#define internal static 
#define local_persist static 
#define global_variable static 

struct win32_offscreen_buffer{
	BITMAPINFO Info;
	VOID *Memory;
	int Width;
	int Height;
	int Pitch;
	int BytesPerPixel;
};

struct win32_dimension {
	int Width;
	int Height;
};

global_variable bool GlobalRunning;
global_variable win32_offscreen_buffer GlobalBuffer;

#define bool32 int32_t

typedef DWORD WINAPI x_input_get_state(DWORD dwUserIndex, XINPUT_STATE* pState);

typedef DWORD WINAPI x_input_set_state(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration);

#define XInputGetState XInputGetState_
#define XInputSetState XInputSetState_

#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE* pState)
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)

typedef X_INPUT_GET_STATE(x_input_get_state);
typedef X_INPUT_SET_STATE(x_input_set_state);

X_INPUT_GET_STATE(XInputGetStateStub){
	return (ERROR_DEVICE_NOT_CONNECTED);
}

X_INPUT_SET_STATE(XInputSetStateStub){
	return (ERROR_DEVICE_NOT_CONNECTED);
}

global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;

global_variable int XOffSet = 0;
global_variable int YOffSet = 0;

internal void 
Win32LoadXInput(void){
	HMODULE XInputLibrary = LoadLibraryA("xinput1_3.dll");
	if(XInputLibrary){
		XInputGetState = (x_input_get_state*)GetProcAddress(XInputLibrary, "XInputGetState");
		XInputSetState = (x_input_set_state*)GetProcAddress(XInputLibrary, "XInputSetState");
	}
}

internal win32_dimension 
Win32GetDimension(HWND Window){
	RECT WindowRect;
				
	GetClientRect(Window, &WindowRect);

	win32_dimension Dimension;
	Dimension.Width = WindowRect.right - WindowRect.left;
	Dimension.Height = WindowRect.bottom - WindowRect.top;
				
	return Dimension;
}

internal void 
RenderGradient(win32_offscreen_buffer* Buffer, int XOffset, int YOffSet){
	uint8_t * Row = (uint8_t *) Buffer->Memory;

	for(int Y = 0; Y < Buffer->Height; Y++){
		uint8_t * Pixels = (uint8_t *) Row; 
		for(int X = 0; X < Buffer->Width; X++){
			
			Pixels[0] = X + Y;			
			Pixels[1] = X + (XOffset) ;			
			Pixels[2] = Y + (YOffSet);			
			Pixels[3] = 0;			
			Pixels += sizeof(uint32_t);
		}
		Row += Buffer->Pitch;
	}
}

internal void 
Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height){
	Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
  	Buffer->Info.bmiHeader.biWidth = Width;
  	Buffer->Info.bmiHeader.biHeight = -Height;
  	Buffer->Info.bmiHeader.biPlanes = 1;
  	Buffer->Info.bmiHeader.biBitCount = 32;

	Buffer->Width = Width;
	Buffer->Height = Height;
	Buffer->BytesPerPixel = 4;

	if(Buffer->Memory){
		VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
	}

	Buffer->Memory = VirtualAlloc(0, (Buffer->Width * Buffer->Height) * Buffer->BytesPerPixel,  MEM_COMMIT, PAGE_READWRITE);
	Buffer->Pitch = Width * Buffer->BytesPerPixel; 
}
internal void 
Win32DisplayBufferInWindow(HDC DeviceContext, int WindowWidth, int WindowHeight, win32_offscreen_buffer* Buffer, int X, int Y, int Width, int Height){
		StretchDIBits(
  						DeviceContext,
						0, 0, WindowWidth, WindowHeight,
  						0, 0, Buffer->Width, Buffer->Height,
  						Buffer->Memory,
						&Buffer->Info,
  						DIB_RGB_COLORS,
  						SRCCOPY
	);
}

LRESULT Win32CallWindowProcA(
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
	}
	break;
	case WM_DESTROY:
	{
		GlobalRunning = false;
		OutputDebugStringA("WM_SIZE\n");
	}
	break;
	case WM_CLOSE:
	{
		GlobalRunning = false;
		OutputDebugStringA("WM_CLOSE\n");
	}
	break;
	case WM_ACTIVATEAPP:
	{
		OutputDebugStringA("WM_ACTIVATEAPP\n");
	}
	break;
	case WM_SYSKEYUP:
	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
	case WM_KEYUP:
	{

		bool WasDown = (LParam & (1 << 30)) != 0;
		bool IsDown = (LParam & (1 << 31)) == 0;
		if(WasDown != IsDown){
			if(WParam == VK_DOWN){
				OutputDebugStringA("DOWN");
			}
			if(WParam == VK_UP){
				OutputDebugStringA("UP");
			}
			if(WParam == VK_LEFT){
				OutputDebugStringA("LEFT");
			}
			if(WParam == VK_RIGHT){
				OutputDebugStringA("RIGHT");
			}
			if(WParam == VK_ESCAPE){

			}
			if(WParam == VK_SPACE){

			}
			if(WParam == 'W'){
				OutputDebugStringA("W");
				YOffSet++;
			}
			if(WParam == 'S'){
				OutputDebugStringA("S");
				YOffSet--;
			}
			if(WParam == 'D'){
				OutputDebugStringA("D");
			}
			if(WParam == 'A'){
				OutputDebugStringA("A");
			}
			if(WParam == 'Q'){
				OutputDebugStringA("Q");
			}
			if(WParam == 'E'){
				OutputDebugStringA("E");
			}
			OutputDebugStringA("\n");
		}

		bool32 WasAltPressed = (LParam & (1 << 29));
		if(WParam == VK_F4 && WasAltPressed){
			GlobalRunning = false;
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT Paint;
		HDC DeviceContext = BeginPaint(Window, &Paint);
		
		int Width = Paint.rcPaint.right - Paint.rcPaint.left;
		int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
		int X = Paint.rcPaint.left;
		int Y = Paint.rcPaint.top;

		win32_dimension Dimension = Win32GetDimension(Window);
		Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height, &GlobalBuffer, X, Y, Width, Height);
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
	Win32LoadXInput();

	Win32ResizeDIBSection(&GlobalBuffer, 1280, 720);		
		
	WNDCLASSA windowClass = {};

	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = Win32CallWindowProcA;
	windowClass.hInstance = hInstance;
	//windowClass.hIcon;
	windowClass.lpszClassName = "HandmadeHeroWindowClass";

	if (RegisterClass(&windowClass))
	{
		HWND Window = CreateWindowEx(
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
		if (Window)
		{
			GlobalRunning = true;
		
			while(GlobalRunning)
			{
				MSG Message;
	
				while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
				{
					if(Message.message == WM_QUIT){
						GlobalRunning = false;
					}
					TranslateMessage(&Message);
					DispatchMessage(&Message);
				}

				for (DWORD controllerIndex=0; controllerIndex < XUSER_MAX_COUNT; controllerIndex++ )
				{
					XINPUT_STATE ControllerState;
					ZeroMemory( &ControllerState, sizeof(XINPUT_STATE) );

					if(XInputGetState( controllerIndex, &ControllerState ) == ERROR_SUCCESS){
						// Controller is connected
						XINPUT_GAMEPAD *Gamepad = &ControllerState.Gamepad;
						bool Up = (Gamepad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
						bool Down = (Gamepad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
						bool Left = (Gamepad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
						bool Right = (Gamepad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);

						bool Start = (Gamepad->wButtons & XINPUT_GAMEPAD_START);	
						bool Back = (Gamepad->wButtons & XINPUT_GAMEPAD_BACK);

						bool LeftShoulder = (Gamepad->wButtons & VK_GAMEPAD_LEFT_SHOULDER);	
						bool RightShoulder = (Gamepad->wButtons & VK_GAMEPAD_RIGHT_SHOULDER);	

						bool ButtonA = (Gamepad->wButtons & XINPUT_GAMEPAD_A);	
						bool ButtonB = (Gamepad->wButtons & XINPUT_GAMEPAD_B);	
						bool ButtonY = (Gamepad->wButtons & XINPUT_GAMEPAD_Y);	
						bool ButtonX = (Gamepad->wButtons & XINPUT_GAMEPAD_X);	
						
						SHORT AnalogHorizontal = Gamepad->sThumbLX;
						SHORT AnalogVertical = Gamepad->sThumbLY;

						if( ButtonA ){
							OutputDebugStringA("XINPUT_GAMEPAD_UP");
							YOffSet ++;			
						
						}
				

					}else{
						// Controller is not connected
					}
				}

			
				HDC DeviceContext = GetDC(Window);
				
				win32_dimension Dimension = Win32GetDimension(Window);

				RenderGradient(&GlobalBuffer, XOffSet, YOffSet);
				Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height, &GlobalBuffer, 0, 0, Dimension.Width, Dimension.Height);
				
				ReleaseDC(Window, DeviceContext);
				XOffSet ++;
		
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
