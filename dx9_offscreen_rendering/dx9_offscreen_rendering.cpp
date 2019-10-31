//-----------------------------------------------------------------------------
//           Name: dx9_offscreen_rendering.cpp
//         Author: Kevin Harris
//  Last Modified: 02/01/05
//    Description: This sample demonstrates how to create dynamic textures 
//                 through off-screen rendering.
//
//                 As a demonstration, a spinning textured cube is rendered 
//                 to an off-screen surface, which is in turn, used to create 
//                 a dynamic texture. The dynamic texture is then used to 
//                 texture a second spinning cube, which will be rendered to 
//                 the application's window.
//
//   Control Keys: Left Mouse Button  - Spin the large, black cube.
//                 Right Mouse Button - Spin the textured cube being rendered 
//                                      into the p-buffer.
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
HWND                    g_hWnd             = NULL;
LPDIRECT3D9             g_pD3D             = NULL;
LPDIRECT3DDEVICE9       g_pd3dDevice       = NULL;
LPDIRECT3DVERTEXBUFFER9 g_pVertexBuffer    = NULL;
LPDIRECT3DTEXTURE9      g_pTestTexture     = NULL;
LPD3DXRENDERTOSURFACE   m_pRenderToSurface = NULL;
LPDIRECT3DTEXTURE9      g_pDynamicTexture  = NULL;
LPDIRECT3DSURFACE9      m_pTextureSurface  = NULL;

const int RENDERTOSURFACE_WIDTH  = 256;
const int RENDERTOSURFACE_HEIGHT = 256;

float  g_fSpinX_L = 0.0f;
float  g_fSpinY_L = 0.0f;
float  g_fSpinX_R = 0.0f;
float  g_fSpinY_R = 0.0f;

D3DXMATRIX g_matProjection_window;
D3DXMATRIX g_matProjection_offscreenSurface;

struct Vertex
{
    float x, y, z;
    float tu, tv;

	enum FVF
	{
		FVF_Flags = D3DFVF_XYZ | D3DFVF_TEX1
	};
};

Vertex g_cubeVertices[] =
{
	{-1.0f, 1.0f,-1.0f,  0.0f,0.0f },
	{ 1.0f, 1.0f,-1.0f,  1.0f,0.0f },
	{-1.0f,-1.0f,-1.0f,  0.0f,1.0f },
	{ 1.0f,-1.0f,-1.0f,  1.0f,1.0f },
	
	{-1.0f, 1.0f, 1.0f,  1.0f,0.0f },
	{-1.0f,-1.0f, 1.0f,  1.0f,1.0f },
	{ 1.0f, 1.0f, 1.0f,  0.0f,0.0f },
	{ 1.0f,-1.0f, 1.0f,  0.0f,1.0f },
	
	{-1.0f, 1.0f, 1.0f,  0.0f,0.0f },
	{ 1.0f, 1.0f, 1.0f,  1.0f,0.0f },
	{-1.0f, 1.0f,-1.0f,  0.0f,1.0f },
	{ 1.0f, 1.0f,-1.0f,  1.0f,1.0f },
	
	{-1.0f,-1.0f, 1.0f,  0.0f,0.0f },
	{-1.0f,-1.0f,-1.0f,  0.0f,1.0f },
	{ 1.0f,-1.0f, 1.0f,  1.0f,0.0f },
	{ 1.0f,-1.0f,-1.0f,  1.0f,1.0f },
	
	{ 1.0f, 1.0f,-1.0f,  0.0f,0.0f },
	{ 1.0f, 1.0f, 1.0f,  1.0f,0.0f },
	{ 1.0f,-1.0f,-1.0f,  0.0f,1.0f },
	{ 1.0f,-1.0f, 1.0f,  1.0f,1.0f },
	
	{-1.0f, 1.0f,-1.0f,  1.0f,0.0f },
	{-1.0f,-1.0f,-1.0f,  1.0f,1.0f },
	{-1.0f, 1.0f, 1.0f,  0.0f,0.0f },
	{-1.0f,-1.0f, 1.0f,  0.0f,1.0f }
};

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void loadTexture(void);
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
                             "Direct3D (DX9) - Off-Screen Rendering",
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
	static POINT ptLastMousePosit_L;
	static POINT ptCurrentMousePosit_L;
	static bool  bMousing_L;
	
	static POINT ptLastMousePosit_R;
	static POINT ptCurrentMousePosit_R;
	static bool  bMousing_R;

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
			ptLastMousePosit_L.x = ptCurrentMousePosit_L.x = LOWORD (lParam);
            ptLastMousePosit_L.y = ptCurrentMousePosit_L.y = HIWORD (lParam);
			bMousing_L = true;
		}
		break;

		case WM_LBUTTONUP:
		{
			bMousing_L = false;
		}
		break;

		case WM_RBUTTONDOWN:
		{
			ptLastMousePosit_R.x = ptCurrentMousePosit_R.x = LOWORD (lParam);
            ptLastMousePosit_R.y = ptCurrentMousePosit_R.y = HIWORD (lParam);
			bMousing_R = true;
		}
		break;

		case WM_RBUTTONUP:
		{
			bMousing_R = false;
		}
		break;

		case WM_MOUSEMOVE:
		{
			ptCurrentMousePosit_L.x = LOWORD (lParam);
			ptCurrentMousePosit_L.y = HIWORD (lParam);
			ptCurrentMousePosit_R.x = LOWORD (lParam);
			ptCurrentMousePosit_R.y = HIWORD (lParam);

			if( bMousing_L )
			{
				g_fSpinX_L -= (ptCurrentMousePosit_L.x - ptLastMousePosit_L.x);
				g_fSpinY_L -= (ptCurrentMousePosit_L.y - ptLastMousePosit_L.y);
			}
			
			if( bMousing_R )
			{
				g_fSpinX_R -= (ptCurrentMousePosit_R.x - ptLastMousePosit_R.x);
				g_fSpinY_R -= (ptCurrentMousePosit_R.y - ptLastMousePosit_R.y);
			}

			ptLastMousePosit_L.x = ptCurrentMousePosit_L.x;
            ptLastMousePosit_L.y = ptCurrentMousePosit_L.y;
			ptLastMousePosit_R.x = ptCurrentMousePosit_R.x;
            ptLastMousePosit_R.y = ptCurrentMousePosit_R.y;
		}
		break;
		
		case WM_CLOSE:
		{
			PostQuitMessage(0);	
		}
		break;

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
// Name: loadTexture()
// Desc: 
//-----------------------------------------------------------------------------
void loadTexture( void )	
{
    D3DXCreateTextureFromFile( g_pd3dDevice, "test.bmp", &g_pTestTexture );

	g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
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

	loadTexture();

	g_pd3dDevice->CreateVertexBuffer( 24*sizeof(Vertex),0, Vertex::FVF_Flags,
                                      D3DPOOL_DEFAULT, &g_pVertexBuffer, NULL );
	void *pVertices = NULL;

    g_pVertexBuffer->Lock( 0, sizeof(g_cubeVertices), (void**)&pVertices, 0 );
    memcpy( pVertices, g_cubeVertices, sizeof(g_cubeVertices) );
    g_pVertexBuffer->Unlock();

    g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );

    D3DXMatrixPerspectiveFovLH( &g_matProjection_window, D3DXToRadian( 45.0f ), 640.0f / 480.0f, 0.1f, 100.0f );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &g_matProjection_window );

	//
	// Create our dynamic texture for use by the "render to" surface...
	//

	HRESULT hr;

	hr = D3DXCreateTexture( g_pd3dDevice, 
		                    RENDERTOSURFACE_WIDTH, 
		                    RENDERTOSURFACE_HEIGHT, 
							1, 
							D3DUSAGE_RENDERTARGET, 
							D3DFMT_A8R8G8B8, 
							D3DPOOL_DEFAULT, 
							&g_pDynamicTexture );

	if( FAILED(hr) )
	{
		MessageBox(NULL,"Failed to create a texture with the D3DUSAGE_RENDERTARGET usage flag set!",
				"ERROR",MB_OK|MB_ICONEXCLAMATION);
		exit(-1);
    }

	//
	// Create an off-screen "render to" surface...
	//

    D3DSURFACE_DESC desc;
    g_pDynamicTexture->GetSurfaceLevel( 0, &m_pTextureSurface );
    m_pTextureSurface->GetDesc( &desc );

	hr = D3DXCreateRenderToSurface( g_pd3dDevice, 
		                            desc.Width, 
		                            desc.Height, 
		                            desc.Format, 
		                            TRUE, 
		                            D3DFMT_D16, 
		                            &m_pRenderToSurface );

    if( FAILED(hr) )
    {
        MessageBox(NULL,"Failed to create the off-screen render surface!",
				"ERROR",MB_OK|MB_ICONEXCLAMATION);
		exit(-1);
    }

	//
	// We'll need to create a second projection matrix, which take in account 
	// the off-screen surface's dimensions.
	//

    D3DXMatrixPerspectiveFovLH( &g_matProjection_offscreenSurface, 
                                D3DXToRadian( 45.0f ), 
		                        RENDERTOSURFACE_WIDTH / RENDERTOSURFACE_HEIGHT, 
								0.1f, 100.0f );
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )
{
	if( m_pRenderToSurface != NULL )
        m_pRenderToSurface->Release();

	if( g_pDynamicTexture != NULL )
        g_pDynamicTexture->Release();

	if( m_pTextureSurface != NULL )
        m_pTextureSurface->Release();

    if( g_pTestTexture != NULL )
        g_pTestTexture->Release();

    if( g_pVertexBuffer != NULL )
        g_pVertexBuffer->Release(); 

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
	D3DXMATRIX matWorld;
	D3DXMATRIX matTranslation;
	D3DXMATRIX matRotation;

	//-------------------------------------------------------------------------
	// Use the "render to" surface for off-screen rendering
	//-------------------------------------------------------------------------

	m_pRenderToSurface->BeginScene( m_pTextureSurface, NULL );

    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 
		                 D3DCOLOR_COLORVALUE(0.0f,0.0f,0.0f,1.0f), 1.0f, 0);

	//
	// We'll need to change the projection matrix on the device, to match the 
	// off-screen surface's dimensions, or the cube will look wrong.
	//

	g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &g_matProjection_offscreenSurface );

	//
	// Let the user spin the cube about with the right mouse button, so our 
	// dynamic texture will show motion.
	//
	
    D3DXMatrixTranslation( &matTranslation, 0.0f, 0.0f, 5.0f );

	D3DXMatrixRotationYawPitchRoll( &matRotation, 
		                            D3DXToRadian(g_fSpinX_R),
		                            D3DXToRadian(g_fSpinY_R),
		                            0.0f );

    matWorld = matRotation * matTranslation;
    g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

    //
	// Now the render the cube to the off-screen surface
	//

    g_pd3dDevice->SetTexture( 0, g_pTestTexture );
    g_pd3dDevice->SetStreamSource( 0, g_pVertexBuffer, 0, sizeof(Vertex) );
    g_pd3dDevice->SetFVF( Vertex::FVF_Flags );

	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,  0, 2 );
	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,  4, 2 );
	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,  8, 2 );
	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 12, 2 );
	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 16, 2 );
	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 20, 2 );

    m_pRenderToSurface->EndScene( 0 );

	//-------------------------------------------------------------------------
	// Use the Direct3D device for regular rendering...
	//-------------------------------------------------------------------------

    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                         D3DCOLOR_COLORVALUE(0.0f,0.0f,1.0f,1.0f), 1.0f, 0 );

	//
	// Change the projection matrix back, so it matches the window's size.
	//

	g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &g_matProjection_window );

	//
	// Let the user spin the cube about with the left mouse button.
	//

    D3DXMatrixTranslation( &matTranslation, 0.0f, 0.0f, 5.0f );

	D3DXMatrixRotationYawPitchRoll( &matRotation, 
		                            D3DXToRadian(g_fSpinX_L), 
		                            D3DXToRadian(g_fSpinY_L), 
		                            0.0f );

    matWorld = matRotation * matTranslation;
    g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

    g_pd3dDevice->BeginScene();

	//
	// Finally, we'll use the dynamic texture like a regular static texture.
	//
	
	g_pd3dDevice->SetTexture( 0, g_pDynamicTexture );
    g_pd3dDevice->SetStreamSource( 0, g_pVertexBuffer, 0, sizeof(Vertex) );
    g_pd3dDevice->SetFVF( Vertex::FVF_Flags );

	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,  0, 2 );
	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,  4, 2 );
	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,  8, 2 );
	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 12, 2 );
	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 16, 2 );
	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 20, 2 );

    g_pd3dDevice->EndScene();
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}

