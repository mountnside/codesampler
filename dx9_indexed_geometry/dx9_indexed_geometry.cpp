//-----------------------------------------------------------------------------
//           Name: dx9_indexed_geometry.cpp
//         Author: Kevin Harris
//  Last Modified: 06/15/05
//    Description: This sample demonstrates how to optimize performance by 
//                 using indexed geometry. As a demonstration, the sample 
//                 reduces the vertex count of a simple cube from 24 to 8 by 
//                 redefining the cube’s geometry using an indices array.
//
//   Control Keys: F1 - Toggle between indexed and non-indexed geoemtry.
//                      Shouldn't produce any noticeable change since they
//                      render the same cube.
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "resource.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND               g_hWnd       = NULL;
LPDIRECT3D9        g_pD3D       = NULL;
LPDIRECT3DDEVICE9  g_pd3dDevice = NULL;

LPDIRECT3DVERTEXBUFFER9 g_pVertexBuffer = NULL;

LPDIRECT3DVERTEXBUFFER9 g_pVertexBuffer_Indexed = NULL;
LPDIRECT3DINDEXBUFFER9  g_pIndexBuffer = NULL;

bool g_bUseIndexedGeometry = true;

float g_fSpinX = 0.0f;
float g_fSpinY = 0.0f;

struct Vertex
{
	float x, y, z;
	DWORD color;

	enum FVF
	{
		FVF_Flags = D3DFVF_XYZ | D3DFVF_DIFFUSE
	};
};

//
// To understand how indexed geometry works, we must first build something
// which can be optimized through the use of indices.
//
// Below, is the vertex data for a simple multi-colored cube, which is defined
// as 6 individual quads, one quad for each of the cube's six sides. At first,
// this doesn’t seem too wasteful, but trust me it is.
//
// You see, we really only need 8 vertices to define a simple cube, but since 
// we're using a quad list, we actually have to repeat the usage of our 8 
// vertices 3 times each. To make this more understandable, I've actually 
// numbered the vertices below so you can see how the vertices get repeated 
// during the cube's definition.
//
// Note how the first 8 vertices are unique. Everything else after that is just
// a repeat of the first 8.
//

Vertex g_cubeVertices[] =
{
	// Quad 0
	{-1.0f, 1.0f,-1.0f,  D3DCOLOR_COLORVALUE( 1.0, 0.0, 0.0, 1.0 ) }, // 0 (unique)
	{ 1.0f, 1.0f,-1.0f,  D3DCOLOR_COLORVALUE( 0.0, 1.0, 0.0, 1.0 ) }, // 1 (unique)
	{-1.0f,-1.0f,-1.0f,  D3DCOLOR_COLORVALUE( 0.0, 0.0, 1.0, 1.0 ) }, // 2 (unique)
	{ 1.0f,-1.0f,-1.0f,  D3DCOLOR_COLORVALUE( 1.0, 1.0, 0.0, 1.0 ) }, // 3 (unique)

	// Quad 1
	{-1.0f, 1.0f, 1.0f,  D3DCOLOR_COLORVALUE( 1.0, 0.0, 1.0, 1.0 ) }, // 4 (unique)
	{-1.0f,-1.0f, 1.0f,  D3DCOLOR_COLORVALUE( 0.0, 1.0, 1.0, 1.0 ) }, // 5 (unique)
	{ 1.0f, 1.0f, 1.0f,  D3DCOLOR_COLORVALUE( 1.0, 1.0, 1.0, 1.0 ) }, // 6 (unique)
	{ 1.0f,-1.0f, 1.0f,  D3DCOLOR_COLORVALUE( 1.0, 0.0, 0.0, 1.0 ) }, // 7 (unique)

	// Quad 2
	{-1.0f, 1.0f, 1.0f,  D3DCOLOR_COLORVALUE( 1.0, 0.0, 1.0, 1.0 ) }, // 4 (start repeating here)
	{ 1.0f, 1.0f, 1.0f,  D3DCOLOR_COLORVALUE( 1.0, 1.0, 1.0, 1.0 ) }, // 6 (repeat of vertex 6)
	{-1.0f, 1.0f,-1.0f,  D3DCOLOR_COLORVALUE( 1.0, 0.0, 0.0, 1.0 ) }, // 0 (repeat of vertex 0... etc.)
	{ 1.0f, 1.0f,-1.0f,  D3DCOLOR_COLORVALUE( 0.0, 1.0, 0.0, 1.0 ) }, // 1

	// Quad 3
	{-1.0f,-1.0f, 1.0f,  D3DCOLOR_COLORVALUE( 0.0, 1.0, 1.0, 1.0 ) }, // 5
	{-1.0f,-1.0f,-1.0f,  D3DCOLOR_COLORVALUE( 0.0, 0.0, 1.0, 1.0 ) }, // 2
	{ 1.0f,-1.0f, 1.0f,  D3DCOLOR_COLORVALUE( 1.0, 0.0, 0.0, 1.0 ) }, // 7
	{ 1.0f,-1.0f,-1.0f,  D3DCOLOR_COLORVALUE( 1.0, 1.0, 0.0, 1.0 ) }, // 3

	// Quad 4
	{ 1.0f, 1.0f,-1.0f,  D3DCOLOR_COLORVALUE( 0.0, 1.0, 0.0, 1.0 ) }, // 1
	{ 1.0f, 1.0f, 1.0f,  D3DCOLOR_COLORVALUE( 1.0, 1.0, 1.0, 1.0 ) }, // 6
	{ 1.0f,-1.0f,-1.0f,  D3DCOLOR_COLORVALUE( 1.0, 1.0, 0.0, 1.0 ) }, // 3
	{ 1.0f,-1.0f, 1.0f,  D3DCOLOR_COLORVALUE( 1.0, 0.0, 0.0, 1.0 ) }, // 7

	// Quad 5
	{-1.0f, 1.0f,-1.0f,  D3DCOLOR_COLORVALUE( 1.0, 0.0, 0.0, 1.0 ) }, // 0
	{-1.0f,-1.0f,-1.0f,  D3DCOLOR_COLORVALUE( 0.0, 0.0, 1.0, 1.0 ) }, // 2
	{-1.0f, 1.0f, 1.0f,  D3DCOLOR_COLORVALUE( 1.0, 0.0, 1.0, 1.0 ) }, // 4
	{-1.0f,-1.0f, 1.0f,  D3DCOLOR_COLORVALUE( 0.0, 1.0, 1.0, 1.0 ) }  // 5
};

//
// Now, to save ourselves the bandwidth of passing a bunch or redundant vertices
// down the graphics pipeline, we shorten our vertex list and pass only the 
// unique vertices. We then create a indices array, which contains index values
// that reference vertices in our vertex array. 
//
// In other words, the vertex array doens't actually define our cube anymore, 
// it only holds the unique vertices; it's the indices array that now defines  
// the cube's geometry.
//

Vertex g_cubeVertices_indexed[] =
{
	{-1.0f, 1.0f,-1.0f,  D3DCOLOR_COLORVALUE( 1.0, 0.0, 0.0, 1.0 ) }, // 0
	{ 1.0f, 1.0f,-1.0f,  D3DCOLOR_COLORVALUE( 0.0, 1.0, 0.0, 1.0 ) }, // 1
	{-1.0f,-1.0f,-1.0f,  D3DCOLOR_COLORVALUE( 0.0, 0.0, 1.0, 1.0 ) }, // 2
	{ 1.0f,-1.0f,-1.0f,  D3DCOLOR_COLORVALUE( 1.0, 1.0, 0.0, 1.0 ) }, // 3
	{-1.0f, 1.0f, 1.0f,  D3DCOLOR_COLORVALUE( 1.0, 0.0, 1.0, 1.0 ) }, // 4
	{-1.0f,-1.0f, 1.0f,  D3DCOLOR_COLORVALUE( 0.0, 1.0, 1.0, 1.0 ) }, // 5
	{ 1.0f, 1.0f, 1.0f,  D3DCOLOR_COLORVALUE( 1.0, 1.0, 1.0, 1.0 ) }, // 6
	{ 1.0f,-1.0f, 1.0f,  D3DCOLOR_COLORVALUE( 1.0, 0.0, 0.0, 1.0 ) }  // 7
};

WORD g_cubeIndices[] =
{
	0, 1, 2, 3, // Quad 0
	4, 5, 6, 7, // Quad 1
	4, 6, 0, 1, // Quad 2
	5, 2, 7, 3, // Quad 3
	1, 6, 3, 7, // Quad 4
	0, 2, 4, 5  // Quad 5
};

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void init(void);
void shutDown(void);
void render(void);

//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: The application's entry point
//-----------------------------------------------------------------------------
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
                             "Direct3D (DX9) - Indexed Geometry",
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

//-----------------------------------------------------------------------------
// Name: WindowProc()
// Desc: The window's message handler
//-----------------------------------------------------------------------------
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

			case VK_F1:
				g_bUseIndexedGeometry = !g_bUseIndexedGeometry;
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

//-----------------------------------------------------------------------------
// Name: init()
// Desc: 
//-----------------------------------------------------------------------------
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


	//
	// Create a vertex buffer...
	//

	g_pd3dDevice->CreateVertexBuffer( 24*sizeof(Vertex),
		                              D3DUSAGE_WRITEONLY, 
									  Vertex::FVF_Flags,
		                              D3DPOOL_DEFAULT, 
									  &g_pVertexBuffer, 
									  NULL );
	void *pVertices = NULL;

	g_pVertexBuffer->Lock( 0, sizeof(g_cubeVertices), (void**)&pVertices, 0 );
	memcpy( pVertices, g_cubeVertices, sizeof(g_cubeVertices) );
	g_pVertexBuffer->Unlock();

	//
	// Create a vertex buffer, which contains indexed geometry...
	//

	g_pd3dDevice->CreateVertexBuffer( 8*sizeof(Vertex),
		                              D3DUSAGE_WRITEONLY,
									  Vertex::FVF_Flags,
		                              D3DPOOL_DEFAULT,
									  &g_pVertexBuffer_Indexed,
									  NULL );
	pVertices = NULL;

	g_pVertexBuffer_Indexed->Lock( 0, sizeof(g_cubeVertices_indexed), (void**)&pVertices, 0 );
	memcpy( pVertices, g_cubeVertices_indexed, sizeof(g_cubeVertices_indexed) );
	g_pVertexBuffer_Indexed->Unlock();

	//
	// Create an index buffer to use with our indexed vertex buffer...
	//

	g_pd3dDevice->CreateIndexBuffer( 24*sizeof(WORD),
									 D3DUSAGE_WRITEONLY,
									 D3DFMT_INDEX16,
									 D3DPOOL_DEFAULT,
									 &g_pIndexBuffer,
									 NULL );
	WORD *pIndices = NULL;

	g_pIndexBuffer->Lock( 0, sizeof(g_cubeIndices), (void**)&pIndices, 0 );
	memcpy( pIndices, g_cubeIndices, sizeof(g_cubeIndices) );
	g_pIndexBuffer->Unlock();
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )
{
    if( g_pVertexBuffer != NULL )
        g_pVertexBuffer->Release(); 

	if( g_pVertexBuffer_Indexed != NULL )
		g_pVertexBuffer_Indexed->Release(); 

	if( g_pIndexBuffer != NULL )
		g_pIndexBuffer->Release(); 

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

	if( g_bUseIndexedGeometry == true )
	{
		g_pd3dDevice->SetStreamSource( 0, g_pVertexBuffer_Indexed, 0, sizeof(Vertex) );
		g_pd3dDevice->SetIndices( g_pIndexBuffer );
		g_pd3dDevice->SetFVF( Vertex::FVF_Flags );

        g_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLESTRIP, 0, 0, 8,  0, 2 );
        g_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLESTRIP, 0, 0, 8,  4, 2 );
        g_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLESTRIP, 0, 0, 8,  8, 2 );
        g_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLESTRIP, 0, 0, 8, 12, 2 );
        g_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLESTRIP, 0, 0, 8, 16, 2 );
        g_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLESTRIP, 0, 0, 8, 20, 2 );
	}
	else
	{
		g_pd3dDevice->SetStreamSource( 0, g_pVertexBuffer, 0, sizeof(Vertex) );
		g_pd3dDevice->SetFVF( Vertex::FVF_Flags );

		g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,  0, 2 );
		g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,  4, 2 );
		g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,  8, 2 );
		g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 12, 2 );
		g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 16, 2 );
		g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 20, 2 );
	}

    g_pd3dDevice->EndScene();
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}
