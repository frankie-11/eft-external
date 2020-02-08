#include <Windows.h>
#include <iostream>
#include "PMemHelper.h"
#include "eftstructs.h"
#include "Overlay.h"


MemoryIO* memio = nullptr;
auto pRender = PRENDER::Instance();
auto gameData = EFTData::Instance();

//render function
void render_scene()
{
	// clear the window alpha
	d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	d3ddev->BeginScene();    // begins the 3D scene

	static char cTitle[256];
	sprintf_s(cTitle, _xor_("getgoodv1 | EFT 0.12 \nFrame Rate: %i\nPlayers: %i").operator const char* (), FrameRate(), gameData->playercount);
	GradientBoxOutlined(40, 70, StringWidth(pFont, cTitle) + 4, 40, 1, D3DCOLOR_RGBA(100, 100, 100, 255), D3DCOLOR_RGBA(50, 50, 50, 255), D3DCOLOR_RGBA(0, 0, 0, 255), true);
	String(42, 72, D3DCOLOR_RGBA(255, 255, 255, 255), pFont, true, cTitle);

	//calculate and and draw esp stuff
	pRender->Render();
	
	//draw_string(10, 10, color, pFont, "output");

	d3ddev->EndScene();    // ends the 3D scene

	d3ddev->Present(NULL, NULL, NULL, NULL);   // displays the created frame on the screen
}

//set up overlay window
void start_window()
{
	RECT rc;

	twnd = NULL;
	while (!twnd)
		twnd = FindWindow(_xor_(L"UnityWndClass"), 0);
	if (twnd != NULL)
	{
		GetWindowRect(twnd, &rc);
		s_width = rc.right - rc.left;
		s_height = rc.bottom - rc.top;
	}

	WNDCLASSEX wndclass;
	ZeroMemory(&wndclass, sizeof(WNDCLASSEX)); // Initialises, sets all bits to 0 to remove garbage data
	wndclass.cbClsExtra = NULL;
	wndclass.cbWndExtra = NULL;
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WindowProc; // Function that will be executed when the window receives a "message" (input). Required! (crashes if set to NULL)
	wndclass.hInstance = NULL;
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hIcon = LoadIcon(0, IDI_APPLICATION);
	wndclass.hIconSm = LoadIcon(0, IDI_APPLICATION);
	wndclass.hbrBackground = (HBRUSH)RGB(0, 0, 0);
	wndclass.lpszClassName = L"Class_RiotWnd";
	RegisterClassEx(&wndclass);

	hWnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_TRANSPARENT, wndclass.lpszClassName, L"", WS_POPUP, rc.left, rc.top, s_width, s_height, NULL, NULL, wndclass.hInstance, NULL);

	// Activate transparency on color black.
	SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 0, ULW_COLORKEY);
	SetLayeredWindowAttributes(hWnd, 0, 255, LWA_ALPHA);
//	SetLayeredWindowAttributes(hWnd, 0, 255, LWA_ALPHA);

	ShowWindow(hWnd, SW_SHOW);
	initD3D(hWnd);
}

uint32_t EntryMode()
{
	MSG msg;
	//RECT rc;

	while (TRUE)
	{
		ZeroMemory(&msg, sizeof(MSG));
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT)
			exit(0);


		SetWindowPos(twnd, hWnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

		UpdateWindow(hWnd);

		if (!gameData->Read())
			gameData->InitOffsets();

		//render your esp
		render_scene();

		Sleep(5);
	}
	return 0;
}


int main()
{
	printf(_xor_("Welcome , press enter to initialize systems"));
	getchar();

	PMem* mem = new PMem();

	const auto connection = mem->connectsystem();

	printf(_xor_("systems up.. press enter in game"));
	getchar();

	FreeConsole();

	memio = new MemoryIO(*mem, connection);

	start_window();


	uint32_t entry = (uint32_t)EntryMode();
	//getchar();
	return entry;
}