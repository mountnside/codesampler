//------------------------------------------------------------------------------
//           Name: dx9u_hlsl_fx_post_process.cpp
//         Author: Michael Guerrero
//  Last Modified: 05/05/06
//    Description: This sample demonstrates how to easily render off-screen, 
//                 apply post processing effects and render the finished image 
//                 onscreen. As a demonstration, the sample uses a Sobel edge 
//                 detection shader to render a post-processed version a 
//                 textured quad.
//
//   Control Keys: Space Bar - Toggle usage of Sobel Edge Detection shader
//------------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <cassert>
#include "resource.h"

//------------------------------------------------------------------------------
// GLOBALS
//------------------------------------------------------------------------------
HWND                    g_hWnd                    = NULL;
LPDIRECT3D9             g_pD3D                    = NULL;
LPDIRECT3DDEVICE9       g_pd3dDevice              = NULL;
LPDIRECT3DVERTEXBUFFER9 g_pVertexBuffer           = NULL;
LPDIRECT3DVERTEXBUFFER9 g_pFullScreemVertexBuffer = NULL;
LPDIRECT3DTEXTURE9      g_pTexture                = NULL;
LPD3DXEFFECT            g_pEffect                 = NULL;

// Off-screen render target
IDirect3DTexture9* g_pRenderTargetTexture;
IDirect3DSurface9* g_pRenderTargetSurface;
IDirect3DSurface9* g_pFrameBufferSurface;

D3DXMATRIX g_matWorld;
D3DXMATRIX g_matView;
D3DXMATRIX g_matProj;
float      g_fSpinX = 0.0f;
float      g_fSpinY = 0.0f;

bool g_bEnableSobelShader = true;

struct Vertex
{
    float x, y, z;
    float tu, tv;
};

Vertex g_quadVertices[] =
{
	{-1.0f, 1.0f, 0.0f,  0.0f,0.0f },
	{ 1.0f, 1.0f, 0.0f,  1.0f,0.0f },
	{-1.0f,-1.0f, 0.0f,  0.0f,1.0f },
	{ 1.0f,-1.0f, 0.0f,  1.0f,1.0f }
};


#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_TEX1)

//------------------------------------------------------------------------------
// PROTOTYPES
//------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void init(void);
void render(void);
void renderRegularQuad(void);
void renderOffscreen(void);
void shutDown(void);
void initEffect(void);
void setSobelTechniqueVariables(void);

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
                             "Direct3D (DX9) - Post Processing Using HLSL - Toggle effect using the SPACE BAR",
						     WS_OVERLAPPEDWINDOW | WS_VISIBLE,
					         0, 0, 640, 480, NULL, NULL, hInstance, NULL );

	if( g_hWnd == NULL )
		return E_FAIL;

    ShowWindow( g_hWnd, nCmdShow );
    UpdateWindow( g_hWnd );

	init();
	initEffect();

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
				case VK_SPACE:
					g_bEnableSobelShader = !g_bEnableSobelShader;
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

	g_pd3dDevice->CreateVertexBuffer( 4*sizeof(Vertex), D3DUSAGE_WRITEONLY, 
                                      D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, 
                                      &g_pVertexBuffer, NULL );
    void *pVertices = NULL;

    g_pVertexBuffer->Lock( 0, sizeof(g_quadVertices), (void**)&pVertices, 0 );
    memcpy( pVertices, g_quadVertices, sizeof(g_quadVertices) );
    g_pVertexBuffer->Unlock();

	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	g_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	g_pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	D3DXMatrixPerspectiveFovLH( &g_matProj, D3DXToRadian( 45.0f ), 
                                640.0f / 480.0f, 0.1f, 100.0f );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &g_matProj );

	D3DXMatrixIdentity( &g_matView ); // This sample is not really making use of a view matrix	
}

//------------------------------------------------------------------------------
// Name: initEffect()
// Desc: Initialize an Fx effect.
//------------------------------------------------------------------------------
void initEffect( void )
{
	//
	// Create a test texture for our effect to use...
	//

	D3DXCreateTextureFromFile( g_pd3dDevice, "checker_with_numbers.bmp", &g_pTexture );

	HRESULT hr;
	LPD3DXBUFFER pBufferErrors = NULL;

	hr = D3DXCreateEffectFromFile( g_pd3dDevice, 
		                           "dx9u_hlsl_fx_post_process.fx",
		                           NULL, 
		                           NULL, 
		                           0, 
		                           NULL, 
		                           &g_pEffect, 
		                           &pBufferErrors );

	if( FAILED(hr) )
	{
		LPVOID pCompilErrors = pBufferErrors->GetBufferPointer();
		MessageBox(NULL, (const char*)pCompilErrors, "Fx Compile Error",
			MB_OK|MB_ICONEXCLAMATION);
	}

	// Retrieve info from the current frame buffer
	if (FAILED(g_pd3dDevice->GetRenderTarget(0, &g_pFrameBufferSurface)))
		assert(!"Unable to get render target");	

	// get the description for the texture we want to filter
	D3DSURFACE_DESC ddsd;
	
	if (FAILED(g_pFrameBufferSurface->GetDesc(&ddsd)))
		assert(!"Unable to get LevelDesc");	

	if (FAILED(g_pd3dDevice->CreateTexture(ddsd.Width,
		                                   ddsd.Height,
		                                   1,
		                                   D3DUSAGE_RENDERTARGET,
		                                   ddsd.Format,
		                                   D3DPOOL_DEFAULT,
		                                   &g_pRenderTargetTexture,
		                                   0)))
	{
		assert(!"Unable to create texture");
		return;
	}			

	if (FAILED(g_pRenderTargetTexture->GetSurfaceLevel(0, &g_pRenderTargetSurface)))
	{
		assert(!"Unable to get SurfaceLevel");
		return;
	}

	g_pd3dDevice->CreateVertexBuffer( 4*sizeof(Vertex), D3DUSAGE_WRITEONLY, 
		                              D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, 
		                              &g_pFullScreemVertexBuffer, NULL );

	void *pVertices = NULL;

	g_pFullScreemVertexBuffer->Lock( 0, sizeof(g_quadVertices), (void**)&pVertices, 0 );
	memcpy( pVertices, g_quadVertices, sizeof(g_quadVertices) );
	g_pFullScreemVertexBuffer->Unlock();
	
	g_pEffect->SetFloat( "du", 1.0f / (float)ddsd.Width);
	g_pEffect->SetFloat( "dv", 1.0f / (float)ddsd.Height);
}

//------------------------------------------------------------------------------
// Name: shutDown()
// Desc: Releases all previously initialized objects
//------------------------------------------------------------------------------
void shutDown( void )
{
	if( g_pTexture != NULL ) 
    {
		g_pTexture->Release(); 
        g_pTexture = NULL;
	}
	
	if( g_pEffect != NULL )
	{
		g_pEffect->Release(); 
        g_pEffect = NULL;
	}

    if( g_pVertexBuffer != NULL ) 
    {
        g_pVertexBuffer->Release(); 
        g_pVertexBuffer = NULL; 
    }

    if( g_pd3dDevice != NULL ) 
    {
        g_pd3dDevice->Release(); 
        g_pd3dDevice = NULL; 
    }

    if( g_pD3D != NULL ) 
    {
        g_pD3D->Release(); 
        g_pD3D = NULL; 
    }
}

//------------------------------------------------------------------------------
// Name: renderRegularQuad()
// Desc: 
//------------------------------------------------------------------------------
void renderRegularQuad( void )
{
    //
	// Render our textured quad as we normally would without a shader...
	//

	D3DXMATRIX matTrans;
	D3DXMATRIX matRot;

	D3DXMatrixTranslation( &matTrans, 0.0f, 0.0f, 4.0f );
	D3DXMatrixRotationYawPitchRoll( &matRot, 
									D3DXToRadian(g_fSpinX), 
									D3DXToRadian(g_fSpinY), 
									0.0f );
	g_matWorld = matRot * matTrans;
	g_pd3dDevice->SetTransform( D3DTS_WORLD, &g_matWorld );

	g_pd3dDevice->SetTexture( 0, g_pTexture );

	g_pd3dDevice->SetStreamSource( 0, g_pFullScreemVertexBuffer, 0, sizeof(Vertex) );
	g_pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );
	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
}

//------------------------------------------------------------------------------
// Name: renderOffscreen()
// Desc: Draws the scene to an off-screen buffer
//------------------------------------------------------------------------------
void renderOffscreen( void )
{
	HRESULT hr;
	hr = g_pd3dDevice->SetRenderTarget(0, g_pRenderTargetSurface);
	hr = g_pd3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER | D3DCLEAR_TARGET, 
		                     D3DCOLOR_XRGB( 0xFF, 0x0, 0x0 ), 1.0f, 0);
	assert(hr == D3D_OK);

	g_pd3dDevice->BeginScene();

	renderRegularQuad();

	// Restore the render target to the frame buffer to output to the screen
	hr = g_pd3dDevice->SetRenderTarget(0, g_pFrameBufferSurface);
	assert(hr == D3D_OK);

	g_pd3dDevice->EndScene();
}

//------------------------------------------------------------------------------
// Name: render()
// Desc: Draws the scene
//------------------------------------------------------------------------------
void render( void )
{
	if( g_bEnableSobelShader == true )
	{
		renderOffscreen();
	}

    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                         D3DCOLOR_COLORVALUE(0.0f,0.0f,0.0f,1.0f), 1.0f, 0 );

	g_pd3dDevice->BeginScene();

	if( g_bEnableSobelShader == true )
	{
		//
		// Render our textured quad using a Sobel Edge Detection shader...
		//

		g_pEffect->SetTechnique( "TechniqueSobel" );
		g_pEffect->SetTexture( "offscreenTexture", g_pRenderTargetTexture );

		// The 0 specifies that ID3DXEffect::Begin and ID3DXEffect::End will 
		// save and restore all state modified by the effect.
		UINT uPasses;
		g_pEffect->Begin( &uPasses, 0 );

		for( UINT uPass = 0; uPass < uPasses; ++uPass )
		{
			g_pEffect->BeginPass( uPass );

			//
			// With each pass, render geometry as you normally would...
			//

			g_pd3dDevice->SetStreamSource( 0, g_pFullScreemVertexBuffer, 0, sizeof(Vertex) );
			g_pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );
			g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );

			g_pEffect->EndPass();
		}

		g_pEffect->End();
	}
	else
	{
		//
		// Render our textured quad without a shader...
		//

		renderRegularQuad();
	}

	g_pd3dDevice->EndScene();
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}
