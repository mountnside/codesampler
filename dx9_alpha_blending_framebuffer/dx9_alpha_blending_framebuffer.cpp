//-----------------------------------------------------------------------------
//           Name: dx9_alpha_blending_framebuffer.cpp
//         Author: Kevin Harris
//  Last Modified: 03/25/05
//    Description: This sample demonstrates how to perform alpha-blending in 
//                 the frame-buffer. The sample renders a textured cube which 
//                 is alpha-blended into the frame-buffer in such a way as to 
//                 create a translucent effect. 
//
//   Control Keys: b - Toggle blending
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <mmsystem.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "resource.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND                    g_hWnd          = NULL;
LPDIRECT3D9             g_pD3D          = NULL;
LPDIRECT3DDEVICE9       g_pd3dDevice    = NULL;
LPDIRECT3DVERTEXBUFFER9 g_pVertexBuffer = NULL;
LPDIRECT3DTEXTURE9      g_pTexture      = NULL;

bool g_bBlending = true;

float  g_fElpasedTime;
double g_dCurrentTime;
double g_dLastTime;

#define D3DFVF_CUSTOMVERTEX ( D3DFVF_XYZ | D3DFVF_TEX1 )

struct Vertex
{
    float x, y, z;
    float tu, tv;
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
                             "Direct3D (DX9) - Alpha Blending with the Frame Buffer",
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
        {
            g_dCurrentTime = timeGetTime();
            g_fElpasedTime = (float)((g_dCurrentTime - g_dLastTime) * 0.001);
            g_dLastTime    = g_dCurrentTime;

            render();
        }
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
        case WM_CHAR:
		{
			switch( wParam )
			{
                case 'b':
                case 'B':
                    g_bBlending = !g_bBlending;
                    break;
            }
        }
        break;

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
void loadTexture(void)
{
	D3DXCreateTextureFromFile( g_pd3dDevice, "glass.bmp", &g_pTexture );

	g_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
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

	g_pd3dDevice->CreateVertexBuffer( 24*sizeof(Vertex),0, D3DFVF_CUSTOMVERTEX,
                                      D3DPOOL_DEFAULT, &g_pVertexBuffer, NULL );
	void *pVertices = NULL;

    g_pVertexBuffer->Lock( 0, sizeof(g_cubeVertices), (void**)&pVertices, 0 );
    memcpy( pVertices, g_cubeVertices, sizeof(g_cubeVertices) );
    g_pVertexBuffer->Unlock();
	
	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

    D3DXMATRIX matProj;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DXToRadian( 45.0f ), 
                                640.0f / 480.0f, 0.1f, 100.0f );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )
{
    if( g_pTexture != NULL ) 
        g_pTexture->Release();

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
    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                         D3DCOLOR_COLORVALUE(0.0f,0.0f,0.0f,1.0f), 1.0f, 0 );

    static float fXrot = 0.0f;
	static float fYrot = 0.0f;
	static float fZrot = 0.0f;

	fXrot += 10.1f * g_fElpasedTime;
	fYrot += 10.2f * g_fElpasedTime;
	fZrot += 10.3f * g_fElpasedTime;
	
    D3DXMATRIX matWorld;
    D3DXMATRIX matTrans;
	D3DXMATRIX matRot;

    D3DXMatrixTranslation( &matTrans, 0.0f, 0.0f, 5.0f );

	D3DXMatrixRotationYawPitchRoll( &matRot, 
		                            D3DXToRadian(fXrot), 
		                            D3DXToRadian(fYrot), 
		                            D3DXToRadian(fZrot) );
    matWorld = matRot * matTrans;
    g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

    g_pd3dDevice->BeginScene();

	if( g_bBlending == true )
	{
        g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
        g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, FALSE );

        g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
        g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
        g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
	}
	else
	{
		g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
		g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE ); 
	}

    g_pd3dDevice->SetTexture( 0, g_pTexture );
    g_pd3dDevice->SetStreamSource( 0, g_pVertexBuffer, 0, sizeof(Vertex) );
    g_pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );

	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,  0, 2 );
	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,  4, 2 );
	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,  8, 2 );
	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 12, 2 );
	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 16, 2 );
	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 20, 2 );

    g_pd3dDevice->EndScene();
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}
