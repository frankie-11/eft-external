#include "overlay.h"
#include <string>
#include <cstdarg>
#include <d3dtypes.h>
#include <time.h>       /* clock_t, clock, CLOCKS_PER_SEC */
#include <vector>


//defining data
int s_width = 1920;
int s_height = 1080;
LPDIRECT3D9 d3d;
LPDIRECT3DDEVICE9 d3ddev;
LPDIRECT3DVERTEXBUFFER9 g_pVB;
HWND hWnd;
HWND twnd;
MARGINS  margin = { 0,0,s_width,s_height };
LPD3DXFONT pFont;
LPD3DXFONT pFont2;
ID3DXLine* d3dLine;


//ESP stuff
void initD3D(HWND hWnd)
{
	d3d = Direct3DCreate9(D3D_SDK_VERSION);    // create the Direct3D interface

	D3DPRESENT_PARAMETERS d3dpp;    // create a struct to hold various device information

	ZeroMemory(&d3dpp, sizeof(d3dpp));    // clear out the struct for use
	d3dpp.Windowed = TRUE;    // program windowed, not fullscreen
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;    // discard old frames
	d3dpp.hDeviceWindow = hWnd;    // set the window to be used by Direct3D
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;     // set the back buffer format to 32-bit
	d3dpp.BackBufferWidth = s_width;    // set the width of the buffer
	d3dpp.BackBufferHeight = s_height;    // set the height of the buffer

	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

	// create a device class using this information and the info from the d3dpp stuct
	d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &d3ddev);

	D3DXCreateFontA(d3ddev, 12, 0, FW_MEDIUM, 1, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Purista-light", &pFont);
	D3DXCreateFontA(d3ddev, 15, 0, FW_MEDIUM, 1, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "icomoon", &pFont2);
	D3DXCreateLine(d3ddev, &d3dLine);
}


void draw_line(float x, float y, float xx, float yy, D3DCOLOR color)
{
	D3DXVECTOR2 dLine[2];

	d3dLine->SetWidth(1.f);

	dLine[0].x = x;
	dLine[0].y = y;

	dLine[1].x = xx;
	dLine[1].y = yy;

	d3dLine->Draw(dLine, 2, color);

}



void DrawLine2(float x1, float y1, float x2, float y2, float size, bool antiAlias, D3DCOLOR COLOR)
{
	D3DXVECTOR2 dLine[2];

	d3dLine->SetWidth(size);

	dLine[1] = { x1, y1 };
	dLine[0] = { x2, y2 };

	if (antiAlias)
		d3dLine->SetAntialias(1);
	else
		d3dLine->SetAntialias(0);

	d3dLine->Begin();
	d3dLine->Draw(dLine, 2, COLOR);
	d3dLine->End();
}

void draw_box(float x, float y, float width, float height, D3DCOLOR color)
{
	D3DXVECTOR2 points[5];
	points[0] = D3DXVECTOR2(x, y);
	points[1] = D3DXVECTOR2(x + width, y);
	points[2] = D3DXVECTOR2(x + width, y + height);
	points[3] = D3DXVECTOR2(x, y + height);
	points[4] = D3DXVECTOR2(x, y);
	d3dLine->SetWidth(1);
	d3dLine->Draw(points, 5, color);
}


void String(int x, int y, D3DCOLOR colour, ID3DXFont* font, bool outlined, const char* string, ...)
{
	va_list args;
	char cBuffer[256];

	va_start(args, string);
	vsprintf_s(cBuffer, string, args);
	va_end(args);

	RECT pRect;
	if (outlined) {
		pRect.left = x - 1;
		pRect.top = y;
		font->DrawTextA(NULL, cBuffer, strlen(cBuffer), &pRect, DT_NOCLIP, D3DCOLOR_RGBA(0, 0, 0, 255));
		pRect.left = x + 1;
		pRect.top = y;
		font->DrawTextA(NULL, cBuffer, strlen(cBuffer), &pRect, DT_NOCLIP, D3DCOLOR_RGBA(0, 0, 0, 255));
		pRect.left = x;
		pRect.top = y - 1;
		font->DrawTextA(NULL, cBuffer, strlen(cBuffer), &pRect, DT_NOCLIP, D3DCOLOR_RGBA(0, 0, 0, 255));
		pRect.left = x;
		pRect.top = y + 1;
		font->DrawTextA(NULL, cBuffer, strlen(cBuffer), &pRect, DT_NOCLIP, D3DCOLOR_RGBA(0, 0, 0, 255));
	}
	pRect.left = x;
	pRect.top = y;
	font->DrawTextA(NULL, cBuffer, strlen(cBuffer), &pRect, DT_NOCLIP, colour);
}

void FilledBox(int x, int y, int width, int height, D3DCOLOR colour)
{
	SD3DVertex pVertex[4] = { { x, y + height, 0.0f, 1.0f, colour }, { x, y, 0.0f, 1.0f, colour }, { x + width, y + height, 0.0f, 1.0f, colour }, { x + width, y, 0.0f, 1.0f, colour } };
	d3ddev->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	d3ddev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pVertex, sizeof(SD3DVertex));
}


void BorderedBox(int x, int y, int width, int height, int thickness, D3DCOLOR colour) {
	FilledBox(x, y, width, thickness, colour);
	FilledBox(x, y, thickness, height, colour);
	FilledBox(x + width - thickness, y, thickness, height, colour);
	FilledBox(x, y + height - thickness, width, thickness, colour);
}

void GradientBox(int x, int y, int width, int height, D3DCOLOR colour, D3DCOLOR color2, bool vertical) {
	SD3DVertex pVertex[4] = { { x, y, 0.0f, 1.0f, colour }, { x + width, y, 0.0f, 1.0f, vertical ? colour : color2 }, { x, y + height, 0.0f, 1.0f, vertical ? color2 : colour }, { x + width, y + height, 0.0f, 1.0f, color2 } };
	d3ddev->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	d3ddev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pVertex, sizeof(SD3DVertex));
}
void GradientBoxOutlined(int x, int y, int width, int height, int thickness, D3DCOLOR colour, D3DCOLOR color2, D3DCOLOR outlinecolor, bool vertical) {
	BorderedBox(x, y, width, height, thickness, outlinecolor);
	GradientBox(x + thickness, y + thickness, width - (thickness * 2), height - (thickness * 2), colour, color2, vertical);
}

int StringWidth(ID3DXFont* font, char* string) {
	RECT pRect = RECT();
	font->DrawTextA(NULL, string, strlen(string), &pRect, DT_CALCRECT, D3DCOLOR_RGBA(0, 0, 0, 0));
	return pRect.right - pRect.left;
}

int FrameRate()
{
	static int iFps, iLastFps;
	static float flLastTickCount, flTickCount;
	flTickCount = clock() * 0.001f;
	iFps++;
	if ((flTickCount - flLastTickCount) >= 1.0f) {
		flLastTickCount = flTickCount;
		iLastFps = iFps;
		iFps = 0;
	}
	return iLastFps;
}


void DrawBar(int x, int y, int value)
{
	float l, r, g;

	g = value * 2.55;
	r = 255 - g;
	l = value / 3;
	FillRGB(x - (l / 2) - 1, y - 1, l + 2 + 13, 5, 0, 0, 0, 255);
	FillRGB(x - (l / 2), y, l + 13, 3, r, g, 0, 255);
}

void FillRGB(float x, float y, float w, float h, int r, int g, int b, int a)
{
	D3DXVECTOR2 vLine[2];

	d3dLine->SetWidth(w);

	vLine[0].x = x + w / 2;
	vLine[0].y = y;
	vLine[1].x = x + w / 2;
	vLine[1].y = y + h;

	d3dLine->Begin();
	d3dLine->Draw(vLine, 2, D3DCOLOR_RGBA(r, g, b, a));
	d3dLine->End();
}

void Circle(float x, float y, float radius, int rotate, int type, bool smoothing, int resolution, DWORD color)
{
	std::vector<SD3DVertex> circle(resolution + 2);

	float angle = rotate * D3DX_PI / 180;
	float pi;

	if (type == 1) pi = D3DX_PI;        // Full circle
	if (type == 2) pi = D3DX_PI / 2;      // 1/2 circle
	if (type == 3) pi = D3DX_PI / 4;   // 1/4 circle

	for (int i = 0; i < resolution + 2; i++)
	{
		circle[i].x = (float)(x - radius * cos(i * (2 * pi / resolution)));
		circle[i].y = (float)(y - radius * sin(i * (2 * pi / resolution)));
		circle[i].z = 0;
		circle[i].rhw = 1;
		circle[i].colour = color;
	}

	// Rotate matrix
	int _res = resolution + 2;
	for (int i = 0; i < _res; i++)
	{
		circle[i].x = x + cos(angle) * (circle[i].x - x) - sin(angle) * (circle[i].y - y);
		circle[i].y = y + sin(angle) * (circle[i].x - x) + cos(angle) * (circle[i].y - y);
	}

	d3ddev->CreateVertexBuffer((resolution + 2) * sizeof(SD3DVertex), D3DUSAGE_WRITEONLY, D3DFVF_XYZRHW | D3DFVF_DIFFUSE, D3DPOOL_DEFAULT, &g_pVB, NULL);

	VOID* pVertices;
	g_pVB->Lock(0, (resolution + 2) * sizeof(SD3DVertex), (void**)&pVertices, 0);
	memcpy(pVertices, &circle[0], (resolution + 2) * sizeof(SD3DVertex));
	g_pVB->Unlock();


	d3ddev->SetTexture(0, NULL);
	d3ddev->SetPixelShader(NULL);
	if (smoothing)
	{
		d3ddev->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
		d3ddev->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, TRUE);
	}
	d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	d3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	d3ddev->SetStreamSource(0, g_pVB, 0, sizeof(SD3DVertex));
	d3ddev->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);

	d3ddev->DrawPrimitive(D3DPT_LINESTRIP, 0, resolution);
	if (g_pVB != NULL) g_pVB->Release();
}



LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT:
		render_scene();
		break;
	case WM_CREATE:
		DwmExtendFrameIntoClientArea(hWnd, &margin);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}
