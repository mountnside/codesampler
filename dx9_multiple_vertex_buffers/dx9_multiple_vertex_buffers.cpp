//------------------------------------------------------------------------------
//           Name: dx9_multiple_vertex_buffers.cpp
//         Author: Kevin Harris
//  Last Modified: 02/01/05
//    Description: This sample demonstrates how to create 3D geometry with 
//                 Direct3D by loading vertex data into a multiple Vertex 
//                 Buffers.
//------------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "resource.h"

//------------------------------------------------------------------------------
// GLOBALS
//------------------------------------------------------------------------------
HWND               g_hWnd       = NULL;
LPDIRECT3D9        g_pD3D       = NULL;
LPDIRECT3DDEVICE9  g_pd3dDevice = NULL;
LPDIRECT3DTEXTURE9 g_pTexture   = NULL;

LPDIRECT3DVERTEXDECLARATION9 g_pVertexDeclaration;

LPDIRECT3DVERTEXBUFFER9 g_pVertexBuffer   = NULL;
LPDIRECT3DVERTEXBUFFER9 g_pColorBuffer    = NULL;
LPDIRECT3DVERTEXBUFFER9 g_pTexCoordBuffer = NULL;

float g_fSpinX = 0.0f;
float g_fSpinY = 0.0f;

struct Vertex
{
    float x, y, z;
};

Vertex g_cubeVertices[] =
{
	{-1.0f, 1.0f,-1.0f },
	{ 1.0f, 1.0f,-1.0f },
	{-1.0f,-1.0f,-1.0f },
	{ 1.0f,-1.0f,-1.0f },
	
	{-1.0f, 1.0f, 1.0f },
	{-1.0f,-1.0f, 1.0f },
	{ 1.0f, 1.0f, 1.0f },
	{ 1.0f,-1.0f, 1.0f },
	
	{-1.0f, 1.0f, 1.0f },
	{ 1.0f, 1.0f, 1.0f },
	{-1.0f, 1.0f,-1.0f },
	{ 1.0f, 1.0f,-1.0f },
	
	{-1.0f,-1.0f, 1.0f },
	{-1.0f,-1.0f,-1.0f },
	{ 1.0f,-1.0f, 1.0f },
	{ 1.0f,-1.0f,-1.0f },
	
	{ 1.0f, 1.0f,-1.0f },
	{ 1.0f, 1.0f, 1.0f },
	{ 1.0f,-1.0f,-1.0f },
	{ 1.0f,-1.0f, 1.0f },
	
	{-1.0f, 1.0f,-1.0f },
	{-1.0f,-1.0f,-1.0f },
	{-1.0f, 1.0f, 1.0f },
	{-1.0f,-1.0f, 1.0f }
};

struct Color
{
	DWORD color;
};

Color g_cubeColors[] =
{
	{ D3DCOLOR_COLORVALUE( 1.0, 0.0, 0.0, 1.0 ) },
	{ D3DCOLOR_COLORVALUE( 1.0, 0.0, 0.0, 1.0 ) },
	{ D3DCOLOR_COLORVALUE( 1.0, 0.0, 0.0, 1.0 ) },
	{ D3DCOLOR_COLORVALUE( 1.0, 0.0, 0.0, 1.0 ) },

	{ D3DCOLOR_COLORVALUE( 0.0, 1.0, 0.0, 1.0 ) },
	{ D3DCOLOR_COLORVALUE( 0.0, 1.0, 0.0, 1.0 ) },
	{ D3DCOLOR_COLORVALUE( 0.0, 1.0, 0.0, 1.0 ) },
	{ D3DCOLOR_COLORVALUE( 0.0, 1.0, 0.0, 1.0 ) },

	{ D3DCOLOR_COLORVALUE( 0.0, 0.0, 1.0, 1.0 ) },
	{ D3DCOLOR_COLORVALUE( 0.0, 0.0, 1.0, 1.0 ) },
	{ D3DCOLOR_COLORVALUE( 0.0, 0.0, 1.0, 1.0 ) },
	{ D3DCOLOR_COLORVALUE( 0.0, 0.0, 1.0, 1.0 ) },

	{ D3DCOLOR_COLORVALUE( 1.0, 1.0, 0.0, 1.0 ) },
	{ D3DCOLOR_COLORVALUE( 1.0, 1.0, 0.0, 1.0 ) },
	{ D3DCOLOR_COLORVALUE( 1.0, 1.0, 0.0, 1.0 ) },
	{ D3DCOLOR_COLORVALUE( 1.0, 1.0, 0.0, 1.0 ) },

	{ D3DCOLOR_COLORVALUE( 1.0, 0.0, 1.0, 1.0 ) },
	{ D3DCOLOR_COLORVALUE( 1.0, 0.0, 1.0, 1.0 ) },
	{ D3DCOLOR_COLORVALUE( 1.0, 0.0, 1.0, 1.0 ) },
	{ D3DCOLOR_COLORVALUE( 1.0, 0.0, 1.0, 1.0 ) },

	{ D3DCOLOR_COLORVALUE( 0.0, 1.0, 1.0, 1.0 ) },
	{ D3DCOLOR_COLORVALUE( 0.0, 1.0, 1.0, 1.0 ) },
	{ D3DCOLOR_COLORVALUE( 0.0, 1.0, 1.0, 1.0 ) },
	{ D3DCOLOR_COLORVALUE( 0.0, 1.0, 1.0, 1.0 ) }
};

struct TexCoord
{
	float tu, tv;
};

TexCoord g_cubeTexCoords[] =
{
	{ 0.0f, 0.0f },
	{ 1.0f, 0.0f },
	{ 0.0f, 1.0f },
	{ 1.0f, 1.0f },

	{ 1.0f, 0.0f },
	{ 1.0f, 1.0f },
	{ 0.0f, 0.0f },
	{ 0.0f, 1.0f },

	{ 0.0f, 0.0f },
	{ 1.0f, 0.0f },
	{ 0.0f, 1.0f },
	{ 1.0f, 1.0f },

    { 0.0f, 0.0f },
    { 1.0f, 0.0f },
    { 0.0f, 1.0f },
    { 1.0f, 1.0f },

	{ 0.0f, 0.0f },
	{ 1.0f, 0.0f },
	{ 0.0f, 1.0f },
	{ 1.0f, 1.0f },

	{ 1.0f, 0.0f },
	{ 1.0f, 1.0f },
	{ 0.0f, 0.0f },
	{ 0.0f, 1.0f }
};

//------------------------------------------------------------------------------
// PROTOTYPES
//------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void loadTexture(void);
void init(void);
void shutDown(void);
void render(void);

//------------------------------------------------------------------------------
// Name: WinMain()
// Desc: The application's entry point
//------------------------------------------------------------------------------
int WINAPI WinMain(	HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPSTR     lpCmdLine,
					int       nCmdShow )
{
	WNDCLASSEX winClass;
	MSG        uMsg;

    memset(&uMsg,0,sizeof(uMsg));
    
	winClass.lpszClassName = "MY_WINDOWS_CLASS";
	winClass.cbSize        = sizeof(WNDCLASSEX);
	winClass.style         = CS_HREDRAW | CS_VREDRAW;
	winClass.lpfnWndProc   = WindowProc;
	winClass.hInstance     = hInstance;
	winClass.hIcon	       = LoadIcon(hInstance, (LPCTSTR)IDI_DIRECTX_ICON);
    winClass.hIconSm	   = LoadIcon(hInstance, (LPCTSTR)IDI_DIRECTX_ICON);
	winClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	winClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	winClass.lpszMenuName  = NULL;
	winClass.cbClsExtra    = 0;
	winClass.cbWndExtra    = 0;

	if( !RegisterClassEx(&winClass) )
		return E_FAIL;

	g_hWnd = CreateWindowEx( NULL, "MY_WINDOWS_CLASS", 
                             "Direct3D (DX9) - Multiple Vertex Buffers",
						     WS_OVERLAPPEDWINDOW | WS_VISIBLE,
					         0, 0, 640, 480, NULL, NULL, hInstance, NULL );

	if( g_hWnd == NULL )
		return E_FAIL;

    ShowWindow( g_hWnd, nCmdShow );
    UpdateWindow( g_hWnd );

	init();

	while( uMsg.message != WM_QUIT )
	{
		if( PeekMessage( &uMsg, NULL, 0, 0, PM_REMOVE ) )
		{ 
			TranslateMessage( &uMsg );
			DispatchMessage( &uMsg );
		}
        else
		    render();
	}

	shutDown();

    UnregisterClass( "MY_WINDOWS_CLASS", winClass.hInstance );

	return uMsg.wParam;
}

//------------------------------------------------------------------------------
// Name: WindowProc()
// Desc: The window's message handler
//------------------------------------------------------------------------------
LRESULT CALLBACK WindowProc( HWND   hWnd, 
							 UINT   msg, 
							 WPARAM wParam, 
							 LPARAM lParam )
{
	static POINT ptLastMousePosit;
	static POINT ptCurrentMousePosit;
	static bool bMousing;

	switch( msg )
	{	
	case WM_KEYDOWN:
		{
			switch( wParam )
			{
			case VK_ESCAPE:
				PostQuitMessage(0);
				break;
			}
		}
		break;

	case WM_LBUTTONDOWN:
		{
			ptLastMousePosit.x = ptCurrentMousePosit.x = LOWORD (lParam);
			ptLastMousePosit.y = ptCurrentMousePosit.y = HIWORD (lParam);
			bMousing = true;
		}
		break;

	case WM_LBUTTONUP:
		{
			bMousing = false;
		}
		break;

	case WM_MOUSEMOVE:
		{
			ptCurrentMousePosit.x = LOWORD (lParam);
			ptCurrentMousePosit.y = HIWORD (lParam);

			if( bMousing )
			{
				g_fSpinX -= (ptCurrentMousePosit.x - ptLastMousePosit.x);
				g_fSpinY -= (ptCurrentMousePosit.y - ptLastMousePosit.y);
			}

			ptLastMousePosit.x = ptCurrentMousePosit.x;
			ptLastMousePosit.y = ptCurrentMousePosit.y;
		}
		break;

		case WM_CLOSE:
		{
			PostQuitMessage(0);	
		}
		
        case WM_DESTROY:
		{
            PostQuitMessage(0);
		}
        break;

		default:
		{
			return DefWindowProc( hWnd, msg, wParam, lParam );
		}
		break;
	}

	return 0;
}

//------------------------------------------------------------------------------
// Name: loadTexture()
// Desc: 
//------------------------------------------------------------------------------
void loadTexture( void )	
{
    D3DXCreateTextureFromFile( g_pd3dDevice, "test.bmp", &g_pTexture );

	g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
}

//------------------------------------------------------------------------------
// Name: init()
// Desc: 
//------------------------------------------------------------------------------
void init( void )
{
    g_pD3D = Direct3DCreate9( D3D_SDK_VERSION );

    D3DDISPLAYMODE d3ddm;

    g_pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm );

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof(d3dpp) );

    d3dpp.Windowed               = TRUE;
    d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat       = d3ddm.Format;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_IMMEDIATE;

    g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, g_hWnd,
                          D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                          &d3dpp, &g_pd3dDevice );

	g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
	g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );

	D3DXMATRIX matProj;
	D3DXMatrixPerspectiveFovLH( &matProj, D3DXToRadian( 45.0f ), 
                                640.0f / 480.0f, 0.1f, 100.0f );
	g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

	loadTexture();

	//
	// Create a vertex buffer that contains only the cube's vertex data
	//

	g_pd3dDevice->CreateVertexBuffer( 24*sizeof(Vertex), 0, 0, D3DPOOL_DEFAULT, &g_pVertexBuffer, NULL );
	void *pVertices = NULL;

    g_pVertexBuffer->Lock( 0, sizeof(g_cubeVertices), (void**)&pVertices, 0 );
    memcpy( pVertices, g_cubeVertices, sizeof(g_cubeVertices) );
    g_pVertexBuffer->Unlock();

	//
	// Create a vertex buffer that contains only the cube's color data
	//

	g_pd3dDevice->CreateVertexBuffer( 24*sizeof(Color), 0, 0, D3DPOOL_DEFAULT, &g_pColorBuffer, NULL );
	void *pColors = NULL;

	g_pColorBuffer->Lock( 0, sizeof(g_cubeColors), (void**)&pColors, 0 );
	memcpy( pColors, g_cubeColors, sizeof(g_cubeColors) );
	g_pColorBuffer->Unlock();

	//
	// Create a vertex buffer that contains only the cube's texture coordinate data
	//

	g_pd3dDevice->CreateVertexBuffer( 24*sizeof(TexCoord), 0, 0, D3DPOOL_DEFAULT, &g_pTexCoordBuffer, NULL );
	void *pTexCoords = NULL;

	g_pTexCoordBuffer->Lock( 0, sizeof(g_cubeTexCoords), (void**)&pTexCoords, 0 );
	memcpy( pTexCoords, g_cubeTexCoords, sizeof(g_cubeTexCoords) );
	g_pTexCoordBuffer->Unlock();

	//
	// Create a vertex declaration so we can describe to Direct3D how we'll 
	// be passing our data to it.
	//

	D3DVERTEXELEMENT9 dwDecl[] = 
	{
		//  Stream  Offset         Type                   Method                 Usage          Usage Index       
		{     0,      0,    D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,     0      },
		{     1,      0,    D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,        0      },
		{     2,      0,    D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,     0      },
		D3DDECL_END()
	};

	g_pd3dDevice->CreateVertexDeclaration( dwDecl, &g_pVertexDeclaration );
}

//------------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//------------------------------------------------------------------------------
void shutDown( void )
{
    if( g_pTexture != NULL )
        g_pTexture->Release();

    if( g_pVertexBuffer != NULL )
        g_pVertexBuffer->Release(); 

	if( g_pColorBuffer != NULL )
		g_pColorBuffer->Release(); 

	if( g_pTexCoordBuffer != NULL )
		g_pTexCoordBuffer->Release(); 

    if( g_pd3dDevice != NULL )
        g_pd3dDevice->Release();

    if( g_pD3D != NULL )
        g_pD3D->Release();
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: 
//-----------------------------------------------------------------------------
void render( void )
{
    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                         D3DCOLOR_COLORVALUE(0.0f,0.0f,0.0f,1.0f), 1.0f, 0 );

	D3DXMATRIX matTrans;
	D3DXMATRIX matRot;
	D3DXMATRIX matWorld;

	D3DXMatrixTranslation( &matTrans, 0.0f, 0.0f, 5.0f );

	D3DXMatrixRotationYawPitchRoll( &matRot, 
		                            D3DXToRadian(g_fSpinX), 
		                            D3DXToRadian(g_fSpinY), 
		                            0.0f );
	matWorld = matRot * matTrans;
	g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

    g_pd3dDevice->BeginScene();

    g_pd3dDevice->SetTexture( 0, g_pTexture );
	g_pd3dDevice->SetVertexDeclaration( g_pVertexDeclaration );

    g_pd3dDevice->SetStreamSource( 0, g_pVertexBuffer,   0, sizeof(Vertex) );
	g_pd3dDevice->SetStreamSource( 1, g_pColorBuffer,    0, sizeof(Color) );
	g_pd3dDevice->SetStreamSource( 2, g_pTexCoordBuffer, 0, sizeof(TexCoord) );

	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,  0, 2 );
	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,  4, 2 );
	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,  8, 2 );
	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 12, 2 );
	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 16, 2 );
	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 20, 2 );

    g_pd3dDevice->EndScene();
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}
