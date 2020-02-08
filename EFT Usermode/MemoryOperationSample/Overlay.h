#include <d3d9.h>
#include <d3dx9.h>
#include <Dwmapi.h> 




//ESP data
extern int s_width;
extern int s_height;
extern LPDIRECT3D9 d3d;    // the pointer to our Direct3D interface
extern LPDIRECT3DDEVICE9 d3ddev;
extern LPDIRECT3DVERTEXBUFFER9 g_pVB;
extern HWND hWnd;
extern HWND twnd;
extern MARGINS  margin;
extern LPD3DXFONT pFont;
extern LPD3DXFONT pFont2;
extern ID3DXLine* d3dLine;

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void FilledBox(int x, int y, int width, int height, D3DCOLOR colour);
void String(int x, int y, D3DCOLOR colour, ID3DXFont* font, bool outlined, const char* string, ...);
void DrawLine2(float x1, float y1, float x2, float y2, float size, bool antiAlias, D3DCOLOR COLOR);
void BorderedBox(int x, int y, int width, int height, int thickness, D3DCOLOR colour);
void GradientBox(int x, int y, int width, int height, D3DCOLOR colour, D3DCOLOR color2, bool vertical);
void GradientBoxOutlined(int x, int y, int width, int height, int thickness, D3DCOLOR colour, D3DCOLOR color2, D3DCOLOR outlinecolor, bool vertical);
void DrawBar(int x, int y, int value);
void FillRGB(float x, float y, float w, float h, int r, int g, int b, int a);
void Circle(float x, float y, float radius, int rotate, int type, bool smoothing, int resolution, DWORD color);
int StringWidth(ID3DXFont* font, char* string);
int FrameRate();
void draw_line(float x, float y, float xx, float yy, D3DCOLOR color);
void draw_box(float x, float y, float width, float height, D3DCOLOR color);
void initD3D(HWND hWnd);
void render_scene();
//

class CVertexList
{
public:
    FLOAT X, Y, Z;
    DWORD colour;
};


	struct SD3DVertex {
	float x, y, z, rhw;
	DWORD colour;
};

