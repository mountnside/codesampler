//------------------------------------------------------------------------------
//           Name: dx9_primitive_types.cpp
//         Author: Kevin Harris
//  Last Modified: 06/06/05
//    Description: This sample demonstrates how to properly use all the 
//                 primitive types available under Direct3D.
//
//                 The primitive types are:
//
//                 D3DPT_POINTLIST
//                 D3DPT_LINELIST
//                 D3DPT_LINESTRIP
//                 D3DPT_TRIANGLELIST
//                 D3DPT_TRIANGLESTRIP
//                 D3DPT_TRIANGLEFAN
//
//   Control Keys: F1 - Switch the primitive type to be rendered.
//                 F2 - Toggle wire-frame mode.
//------------------------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <mmsystem.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "resource.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND              g_hWnd       = NULL;
LPDIRECT3D9       g_pD3D       = NULL;
LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;

LPDIRECT3DVERTEXBUFFER9 g_pPointList_VB     = NULL;
LPDIRECT3DVERTEXBUFFER9 g_pLineStrip_VB     = NULL;
LPDIRECT3DVERTEXBUFFER9 g_pLineList_VB      = NULL;
LPDIRECT3DVERTEXBUFFER9 g_pTriangleList_VB  = NULL;
LPDIRECT3DVERTEXBUFFER9 g_pTriangleStrip_VB = NULL;
LPDIRECT3DVERTEXBUFFER9 g_pTriangleFan_VB   = NULL;

#define D3DFVF_MY_VERTEX ( D3DFVF_XYZ | D3DFVF_DIFFUSE )

struct Vertex
{
	float x, y, z; // Position of vertex in 3D space
    DWORD color;   // Color of vertex
};

bool g_bRenderInWireFrame = false;
D3DPRIMITIVETYPE g_currentPrimitive = D3DPT_TRIANGLEFAN;

//
// We'll store the vertex data in simple arrays until we're ready to load 
// them into actual Direct3D Vertex Buffers. Seeing the vertices laid out 
// like this should make it easier to understand how vertices need to be 
// passed to be considered valid for each primitive type.
//

Vertex g_pointList[] = 
{
    { 0.0f, 0.0f, 0.0f,  D3DCOLOR_COLORVALUE( 1.0, 0.0, 0.0, 1.0 ) },
    { 0.5f, 0.0f, 0.0f,  D3DCOLOR_COLORVALUE( 0.0, 1.0, 0.0, 1.0 ) },
    {-0.5f, 0.0f, 0.0f,  D3DCOLOR_COLORVALUE( 0.0, 0.0, 1.0, 1.0 ) },
	{ 0.0f,-0.5f, 0.0f,  D3DCOLOR_COLORVALUE( 1.0, 1.0, 0.0, 1.0 ) },
    { 0.0f, 0.5f, 0.0f,  D3DCOLOR_COLORVALUE( 0.0, 1.0, 1.0, 1.0 ) },
};

Vertex g_lineList[] = 
{
    {-1.0f,  0.0f, 0.0f, D3DCOLOR_COLORVALUE( 1.0, 0.0, 0.0, 1.0 ) }, // Line #1
    { 0.0f,  1.0f, 0.0f, D3DCOLOR_COLORVALUE( 1.0, 0.0, 0.0, 1.0 ) },

    { 0.5f,  1.0f, 0.0f, D3DCOLOR_COLORVALUE( 0.0, 1.0, 0.0, 1.0 ) }, // Line #2
    { 0.5f, -1.0f, 0.0f, D3DCOLOR_COLORVALUE( 0.0, 1.0, 0.0, 1.0 ) },

	{ 1.0f, -0.5f, 0.0f, D3DCOLOR_COLORVALUE( 0.0, 0.0, 1.0, 1.0 ) }, // Line #3
    {-1.0f, -0.5f, 0.0f, D3DCOLOR_COLORVALUE( 0.0, 0.0, 1.0, 1.0 ) },
};

Vertex g_lineStrip[] = 
{
    { 0.5f, 0.5f, 0.0f, D3DCOLOR_COLORVALUE( 1.0, 0.0, 0.0, 1.0 ) },
    { 1.0f, 0.0f, 0.0f, D3DCOLOR_COLORVALUE( 0.0, 1.0, 0.0, 1.0 ) },
    { 0.0f,-1.0f, 0.0f, D3DCOLOR_COLORVALUE( 0.0, 0.0, 1.0, 1.0 ) },
    {-1.0f, 0.0f, 0.0f, D3DCOLOR_COLORVALUE( 1.0, 1.0, 0.0, 1.0 ) },
	{ 0.0f, 0.0f, 0.0f, D3DCOLOR_COLORVALUE( 0.0, 1.0, 1.0, 1.0 ) },
    { 0.0f, 1.0f, 0.0f, D3DCOLOR_COLORVALUE( 1.0, 0.0, 0.0, 1.0 ) },
};

Vertex g_triangleList[] =
{
	{-1.0f, 0.0f, 0.0f,  D3DCOLOR_COLORVALUE( 1.0, 0.0, 0.0, 1.0 ) }, // Triangle #1
	{ 0.0f, 1.0f, 0.0f,  D3DCOLOR_COLORVALUE( 0.0, 1.0, 0.0, 1.0 ) },
	{ 1.0f, 0.0f, 0.0f,  D3DCOLOR_COLORVALUE( 0.0, 0.0, 1.0, 1.0 ) },

	{-0.5f,-1.0f, 0.0f,  D3DCOLOR_COLORVALUE( 1.0, 1.0, 0.0, 1.0 ) }, // Triangle #2
	{ 0.0f,-0.5f, 0.0f,  D3DCOLOR_COLORVALUE( 0.0, 1.0, 1.0, 1.0 ) },
	{ 0.5f,-1.0f, 0.0f,  D3DCOLOR_COLORVALUE( 1.0, 0.0, 0.0, 1.0 ) },
};

Vertex g_triangleStrip[] = 
{
    {-2.0f, 0.0f, 0.0f,  D3DCOLOR_COLORVALUE( 1.0, 0.0, 0.0, 1.0 ) },
    {-1.0f, 1.0f, 0.0f,  D3DCOLOR_COLORVALUE( 0.0, 1.0, 0.0, 1.0 ) },
    {-1.0f, 0.0f, 0.0f,  D3DCOLOR_COLORVALUE( 0.0, 0.0, 1.0, 1.0 ) },
    { 0.0f, 1.0f, 0.0f,  D3DCOLOR_COLORVALUE( 1.0, 1.0, 0.0, 1.0 ) },
    { 0.0f, 0.0f, 0.0f,  D3DCOLOR_COLORVALUE( 0.0, 1.0, 1.0, 1.0 ) },
    { 1.0f, 1.0f, 0.0f,  D3DCOLOR_COLORVALUE( 1.0, 0.0, 1.0, 1.0 ) },
	{ 1.0f, 0.0f, 0.0f,  D3DCOLOR_COLORVALUE( 1.0, 0.0, 0.0, 1.0 ) },
	{ 2.0f, 1.0f, 0.0f,  D3DCOLOR_COLORVALUE( 0.0, 1.0, 0.0, 1.0 ) }
};

Vertex g_triangleFan[] = 
{
    { 0.0f,-1.0f, 0.0f,  D3DCOLOR_COLORVALUE( 1.0, 0.0, 0.0, 1.0 ) },
    {-1.0f, 0.0f, 0.0f,  D3DCOLOR_COLORVALUE( 0.0, 1.0, 0.0, 1.0 ) },
    {-0.5f, 0.5f, 0.0f,  D3DCOLOR_COLORVALUE( 0.0, 0.0, 1.0, 1.0 ) },
    { 0.0f, 1.0f, 0.0f,  D3DCOLOR_COLORVALUE( 1.0, 1.0, 0.0, 1.0 ) },
    { 0.5f, 0.5f, 0.0f,  D3DCOLOR_COLORVALUE( 0.0, 1.0, 1.0, 1.0 ) },
    { 1.0f, 0.0f, 0.0f,  D3DCOLOR_COLORVALUE( 1.0, 0.0, 1.0, 1.0 ) }
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
                             "Direct3D (DX9) - Primitive Types",
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
				{
					if( g_currentPrimitive == D3DPT_POINTLIST )
						g_currentPrimitive = D3DPT_LINELIST;
					else if( g_currentPrimitive == D3DPT_LINELIST )
						g_currentPrimitive = D3DPT_LINESTRIP;
					else if( g_currentPrimitive == D3DPT_LINESTRIP )
						g_currentPrimitive = D3DPT_TRIANGLELIST;
					else if( g_currentPrimitive == D3DPT_TRIANGLELIST )
						g_currentPrimitive = D3DPT_TRIANGLESTRIP;
					else if( g_currentPrimitive == D3DPT_TRIANGLESTRIP )
						g_currentPrimitive = D3DPT_TRIANGLEFAN;
					else if( g_currentPrimitive == D3DPT_TRIANGLEFAN )
						g_currentPrimitive = D3DPT_POINTLIST;
				}
				break;

				case VK_F2:
                    g_bRenderInWireFrame = !g_bRenderInWireFrame;
                    if( g_bRenderInWireFrame == true )
                        g_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
                    else
                        g_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
			}
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
// Desc: Initializes Direct3D under DirectX 9.0
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

	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	D3DXMATRIX mProjection;
    D3DXMatrixPerspectiveFovLH( &mProjection, D3DXToRadian( 45.0f ), 1.0f, 1.0f, 100.0f );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &mProjection );

	Vertex *pVertices = NULL;

	//
	// Point List
	//

	g_pd3dDevice->CreateVertexBuffer( 5*sizeof(Vertex), 0, D3DFVF_MY_VERTEX,
									  D3DPOOL_DEFAULT, &g_pPointList_VB,
									  NULL );

	pVertices = NULL;
	g_pPointList_VB->Lock( 0, sizeof(g_pointList), (void**)&pVertices, 0 );
    memcpy( pVertices, g_pointList, sizeof(g_pointList) );
    g_pPointList_VB->Unlock();

	//
	// Line List
	//

	g_pd3dDevice->CreateVertexBuffer( 6*sizeof(Vertex), 0, D3DFVF_MY_VERTEX,
									  D3DPOOL_DEFAULT, &g_pLineList_VB,
									  NULL );

	pVertices = NULL;
	g_pLineList_VB->Lock( 0, sizeof(g_lineList), (void**)&pVertices, 0 );
    memcpy( pVertices, g_lineList, sizeof(g_lineList) );
    g_pLineList_VB->Unlock();

	//
	// Line Strip
	//

	g_pd3dDevice->CreateVertexBuffer( 6*sizeof(Vertex), 0, D3DFVF_MY_VERTEX,
									  D3DPOOL_DEFAULT, &g_pLineStrip_VB,
									  NULL );

	pVertices = NULL;
	g_pLineStrip_VB->Lock( 0, sizeof(g_lineStrip), (void**)&pVertices, 0 );
    memcpy( pVertices, g_lineStrip, sizeof(g_lineStrip) );
    g_pLineStrip_VB->Unlock();

	//
	// Triangle List
	//

	g_pd3dDevice->CreateVertexBuffer( 6*sizeof(Vertex), 0, D3DFVF_MY_VERTEX,
									  D3DPOOL_DEFAULT, &g_pTriangleList_VB,
									  NULL );

	pVertices = NULL;
	g_pTriangleList_VB->Lock( 0, sizeof(g_triangleList), (void**)&pVertices, 0 );
    memcpy( pVertices, g_triangleList, sizeof(g_triangleList) );
    g_pTriangleList_VB->Unlock();

	//
	// Triangle Strip
	//

	g_pd3dDevice->CreateVertexBuffer( 8*sizeof(Vertex), 0, D3DFVF_MY_VERTEX,
									  D3DPOOL_DEFAULT, &g_pTriangleStrip_VB,
									  NULL );

	pVertices = NULL;
	g_pTriangleStrip_VB->Lock( 0, sizeof(g_triangleStrip), (void**)&pVertices, 0 );
    memcpy( pVertices, g_triangleStrip, sizeof(g_triangleStrip) );
    g_pTriangleStrip_VB->Unlock();

	//
	// Triangle Fan
	//

	g_pd3dDevice->CreateVertexBuffer( 6*sizeof(Vertex), 0, D3DFVF_MY_VERTEX,
									  D3DPOOL_DEFAULT, &g_pTriangleFan_VB,
									  NULL );

	pVertices = NULL;
	g_pTriangleFan_VB->Lock( 0, sizeof(g_triangleFan), (void**)&pVertices, 0 );
    memcpy( pVertices, g_triangleFan, sizeof(g_triangleFan) );
    g_pTriangleFan_VB->Unlock();
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: Release all Direct3D resources.
//-----------------------------------------------------------------------------
void shutDown( void )
{
	if( g_pPointList_VB != NULL ) 
        g_pPointList_VB->Release(); 

	if( g_pLineList_VB != NULL ) 
        g_pLineList_VB->Release(); 

	if( g_pLineStrip_VB != NULL ) 
        g_pLineStrip_VB->Release(); 

    if( g_pTriangleList_VB != NULL ) 
        g_pTriangleList_VB->Release(); 

	if( g_pTriangleStrip_VB != NULL ) 
        g_pTriangleStrip_VB->Release(); 

	if( g_pTriangleFan_VB != NULL ) 
        g_pTriangleFan_VB->Release(); 

    if( g_pd3dDevice != NULL )
        g_pd3dDevice->Release();

    if( g_pD3D != NULL )
        g_pD3D->Release();
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: Render or draw our scene to the monitor.
//-----------------------------------------------------------------------------
void render( void )
{
    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                         D3DCOLOR_COLORVALUE(0.0f,0.0f,0.0f,1.0f), 1.0f, 0 );

	g_pd3dDevice->BeginScene();

	D3DXMATRIX mWorld;
	D3DXMatrixTranslation( &mWorld, 0.0f, 0.0f, 5.0f );
    g_pd3dDevice->SetTransform( D3DTS_WORLD, &mWorld );

	switch( g_currentPrimitive )
	{
		case D3DPT_POINTLIST:
			g_pd3dDevice->SetStreamSource( 0, g_pPointList_VB, 0, sizeof(Vertex) );
			g_pd3dDevice->SetFVF( D3DFVF_MY_VERTEX );
			g_pd3dDevice->DrawPrimitive( D3DPT_POINTLIST, 0, 5 );
		break;

		case D3DPT_LINELIST:
			g_pd3dDevice->SetStreamSource( 0, g_pLineList_VB, 0, sizeof(Vertex) );
			g_pd3dDevice->SetFVF( D3DFVF_MY_VERTEX );
			g_pd3dDevice->DrawPrimitive( D3DPT_LINELIST, 0, 3);
		break;

		case D3DPT_LINESTRIP:
			g_pd3dDevice->SetStreamSource( 0, g_pLineStrip_VB, 0, sizeof(Vertex) );
			g_pd3dDevice->SetFVF( D3DFVF_MY_VERTEX );
			g_pd3dDevice->DrawPrimitive( D3DPT_LINESTRIP, 0, 5);
		break;

		case D3DPT_TRIANGLELIST:
			g_pd3dDevice->SetStreamSource( 0, g_pTriangleList_VB, 0, sizeof(Vertex) );
			g_pd3dDevice->SetFVF( D3DFVF_MY_VERTEX );
			g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, 2 );
		break;

		case D3DPT_TRIANGLESTRIP:
			g_pd3dDevice->SetStreamSource( 0, g_pTriangleStrip_VB, 0, sizeof(Vertex) );
			g_pd3dDevice->SetFVF( D3DFVF_MY_VERTEX );
			g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 6 );
		break;

		case D3DPT_TRIANGLEFAN:
			g_pd3dDevice->SetStreamSource( 0, g_pTriangleFan_VB, 0, sizeof(Vertex) );
			g_pd3dDevice->SetFVF( D3DFVF_MY_VERTEX );
			g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLEFAN, 0, 4 );
		break;

		default: 
			break;
	}

	g_pd3dDevice->EndScene();
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}

