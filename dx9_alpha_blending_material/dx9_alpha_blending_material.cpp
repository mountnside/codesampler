//-----------------------------------------------------------------------------
//           Name: dx9_alpha_blending_material.cpp
//         Author: Kevin Harris
//  Last Modified: 04/04/05
//    Description: This sample demonstrates how to perform alpha-blending using
//                 a material. This alpha-blending technique is widely used to 
//                 make entire objects fade out of existence over some amount 
//                 of time.
//
//   Control Keys: b - Toggle blending
//                 a - Reduce alpha on the materials Diffuse color
//                 A - Increase alpha on the materials Diffuse color
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
D3DMATERIAL9            g_alphaMaterial;

bool g_bBlending = true;

float g_fDistance = 4.5f;
float g_fSpinX    = 0.0f;
float g_fSpinY    = 0.0f;

struct Vertex
{
    float x, y, z;
	float nx, ny, nz;
    float tu, tv;

	enum FVF
	{
		FVF_Flags = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1
	};
};

Vertex g_cubeVertices[] =
{
//     x     y     z      nx    ny    nz     tu   tv    
    // Front Face
    {-1.0f, 1.0f,-1.0f,  0.0f, 0.0f,-1.0f,  0.0f,0.0f, },
    { 1.0f, 1.0f,-1.0f,  0.0f, 0.0f,-1.0f,  1.0f,0.0f, },
    {-1.0f,-1.0f,-1.0f,  0.0f, 0.0f,-1.0f,  0.0f,1.0f, },
    { 1.0f,-1.0f,-1.0f,  0.0f, 0.0f,-1.0f,  1.0f,1.0f, },
    // Back Face
    {-1.0f, 1.0f, 1.0f,  0.0f, 0.0f, 1.0f,  1.0f,0.0f, },
    {-1.0f,-1.0f, 1.0f,  0.0f, 0.0f, 1.0f,  1.0f,1.0f, },
    { 1.0f, 1.0f, 1.0f,  0.0f, 0.0f, 1.0f,  0.0f,0.0f, },
    { 1.0f,-1.0f, 1.0f,  0.0f, 0.0f, 1.0f,  0.0f,1.0f, },
    // Top Face
    {-1.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f,  0.0f,0.0f, },
    { 1.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f,  1.0f,0.0f, },
    {-1.0f, 1.0f,-1.0f,  0.0f, 1.0f, 0.0f,  0.0f,1.0f, },
    { 1.0f, 1.0f,-1.0f,  0.0f, 1.0f, 0.0f,  1.0f,1.0f, },
    // Bottom Face
    {-1.0f,-1.0f, 1.0f,  0.0f,-1.0f, 0.0f,  0.0f,1.0f, },
    {-1.0f,-1.0f,-1.0f,  0.0f,-1.0f, 0.0f,  0.0f,0.0f, },
    { 1.0f,-1.0f, 1.0f,  0.0f,-1.0f, 0.0f,  1.0f,1.0f, },
    { 1.0f,-1.0f,-1.0f,  0.0f,-1.0f, 0.0f,  1.0f,0.0f, },
    // Right Face
    { 1.0f, 1.0f,-1.0f,  1.0f, 0.0f, 0.0f,  0.0f,0.0f, },
    { 1.0f, 1.0f, 1.0f,  1.0f, 0.0f, 0.0f,  1.0f,0.0f, },
    { 1.0f,-1.0f,-1.0f,  1.0f, 0.0f, 0.0f,  0.0f,1.0f, },
    { 1.0f,-1.0f, 1.0f,  1.0f, 0.0f, 0.0f,  1.0f,1.0f, },
    // Left Face
    {-1.0f, 1.0f,-1.0f, -1.0f, 0.0f, 0.0f,  1.0f,0.0f, },
    {-1.0f,-1.0f,-1.0f, -1.0f, 0.0f, 0.0f,  1.0f,1.0f, },
    {-1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,  0.0f,0.0f, },
    {-1.0f,-1.0f, 1.0f, -1.0f, 0.0f, 0.0f,  0.0f,1.0f, }
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
                             "Direct3D (DX9) - Alpha Blending with a Material",
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
        case WM_CHAR:
		{
			switch( wParam )
			{
                case 'b':
                case 'B':
                    g_bBlending = !g_bBlending;
                    break;

                case 'a':
                    g_alphaMaterial.Diffuse.a -= 0.1f;
                    break;

                case 'A':
                    g_alphaMaterial.Diffuse.a += 0.1f;
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

                case 38: // Up Arrow Key
                    g_fDistance -= 0.1f;
                    break;

                case 40: // Down Arrow Key
                    g_fDistance += 0.1f;
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

    D3DXMATRIX matProj;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DXToRadian( 45.0f ), 
                                640.0f / 480.0f, 0.1f, 100.0f );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

	g_pd3dDevice->CreateVertexBuffer( 24*sizeof(Vertex), D3DUSAGE_WRITEONLY, 
		                              Vertex::FVF_Flags, D3DPOOL_DEFAULT, 
                                      &g_pVertexBuffer, NULL );
    void *pVertices = NULL;

	g_pVertexBuffer->Lock( 0, sizeof(g_cubeVertices), (void**)&pVertices, 0 );
    memcpy( pVertices, g_cubeVertices, sizeof(g_cubeVertices) );
    g_pVertexBuffer->Unlock();
	
	g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );

    // Set up our alpha material...
	ZeroMemory( &g_alphaMaterial, sizeof(D3DMATERIAL9) );
    g_alphaMaterial.Diffuse.r = 1.0f;
    g_alphaMaterial.Diffuse.g = 1.0f;
    g_alphaMaterial.Diffuse.b = 1.0f;
    g_alphaMaterial.Diffuse.a = 0.5f;
    g_pd3dDevice->SetMaterial( &g_alphaMaterial );

    // Set light 0 to be a pure white directional light
    D3DLIGHT9 light0;
    ZeroMemory( &light0, sizeof(D3DLIGHT9) );
    light0.Type = D3DLIGHT_DIRECTIONAL;
    light0.Direction = D3DXVECTOR3( 0.0f, 0.0f, 1.0f );
    light0.Diffuse.r = 1.0f;
    light0.Diffuse.g = 1.0f;
    light0.Diffuse.b = 1.0f;
	light0.Diffuse.a = 1.0f;
    g_pd3dDevice->SetLight( 0, &light0 );
    g_pd3dDevice->LightEnable( 0, TRUE );
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
                         D3DCOLOR_COLORVALUE(0.35f, 0.53f, 0.7, 1.0f), 1.0f, 0 );

    D3DXMATRIX matWorld;
    D3DXMATRIX matTrans;
	D3DXMATRIX matRot;

    D3DXMatrixTranslation( &matTrans, 0.0f, 0.0f, g_fDistance );

	D3DXMatrixRotationYawPitchRoll( &matRot, 
		                            D3DXToRadian(g_fSpinX), 
		                            D3DXToRadian(g_fSpinY), 
		                            0.0f );
    matWorld = matRot * matTrans;
    g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

    g_pd3dDevice->BeginScene();

	if( g_bBlending == true )
	{
        g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
        g_pd3dDevice->SetMaterial( &g_alphaMaterial );

        // Use material's alpha
        g_pd3dDevice->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL );
	    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );

	    // Use alpha for transparency
	    g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	    g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	}
	else
	{
		g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
	}

    //
    // Render cube...
    //

    g_pd3dDevice->SetTexture( 0, g_pTexture );
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
