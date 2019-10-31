//-----------------------------------------------------------------------------
//           Name: dx9_texture_filtering.cpp
//         Author: Kevin Harris
//  Last Modified: 03/24/05
//    Description: This sample demonstrates how to filter textures with 
//                 Direct3D.
//
//                 Direct3D texture filtering is controlled by calling 
//                 SetSamplerState() and setting either D3DSAMP_MINFILTER, 
//                 D3DSAMP_MAGFILTER, or D3DSAMP_MIPFILTER to one of the 
//                 following filtering modes:
//
//                 D3DTEXF_NONE
//                 D3DTEXF_POINT
//                 D3DTEXF_LINEAR
//                 D3DTEXF_ANISOTROPIC
//                 D3DTEXF_FLATCUBIC
//                 D3DTEXF_GAUSSIANCUBIC
//
//   Control Keys: F1 - Change Minification filter
//                 F2 - Change Magnification filter
//                 F3 - Change Mip-Map filter
//                 F4 - Increase Mip-Map LOD Bias
//                 F5 - Decrease Mip-Map LOD Bias
//                 F6 - Increase Max Anisotropy value
//                 F7 - Decrease Max Anisotropy value
//                 Left Mouse Button - Spin the view
//                 Up Arrow - Move the test quad closer
//                 Down Arrow - Move the test quad away
//
// ----------------------------------------------------------------------------
// 
// D3DSAMP_MINFILTER
//
// Accepted Values:
//
//  D3DTEXF_NONE   ?
//  D3DTEXF_POINT
//  D3DTEXF_LINEAR
//  D3DTEXF_ANISOTROPIC
//
// Default Value:
//
//  D3DTEXF_POINT
//
// ----------------------------------------------------------------------------
// 
// D3DSAMP_MAGFILTER
//
// Accepted Values:
//
//  D3DTEXF_NONE   ?
//  D3DTEXF_POINT
//  D3DTEXF_LINEAR
//  D3DTEXF_ANISOTROPIC
//  D3DTEXF_FLATCUBIC       (not covered in this sample)
//  D3DTEXF_GAUSSIANCUBIC   (not covered in this sample)
//
// Default Value:
//
//  D3DTEXF_POINT
//
// ----------------------------------------------------------------------------
// 
// D3DSAMP_MIPFILTER
//
// Accepted Values:
//
//  D3DTEXF_NONE
//  D3DTEXF_POINT
//  D3DTEXF_LINEAR
//  D3DTEXF_ANISOTROPIC
//
// Default Value:
//
//  D3DTEXF_NONE
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
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
LPD3DXFONT              g_pd3dxFont     = NULL;
LPDIRECT3DVERTEXBUFFER9 g_pVertexBuffer = NULL;
LPDIRECT3DTEXTURE9      g_pTexture      = NULL;

float g_fDistance = 4.0f;
float g_fSpinX    = 0.0f;
float g_fSpinY    = 0.0f;

struct Vertex
{
    float x, y, z;
    float tu, tv;

    enum FVF
    {
        FVF_Flags = D3DFVF_XYZ | D3DFVF_TEX1
    };
};

Vertex g_quadVertices[] =
{
	{-1.0f, 1.0f, 0.0f,  0.0f,0.0f },
	{ 1.0f, 1.0f, 0.0f,  1.0f,0.0f },
	{-1.0f,-1.0f, 0.0f,  0.0f,1.0f },
	{ 1.0f,-1.0f, 0.0f,  1.0f,1.0f }
};

enum FilterTypes
{
	FILTER_TYPE_NONE = 0,
	FILTER_TYPE_POINT,
	FILTER_TYPE_LINEAR,
	FILTER_TYPE_ANISOTROPIC
};

int	  g_MinFilterType  = FILTER_TYPE_NONE;
int	  g_MagFilterType  = FILTER_TYPE_NONE;
int	  g_MipFilterType  = FILTER_TYPE_NONE;
int   g_nAnisotropy    = 1;
float g_fMipMapLodBias = 0.0f;
bool  g_bChangeFilters = true;

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void loadTexture(void);
void createFont(void);
void init(void);
void shutDown(void);
void render(void);
void setMagnificationFilter(void);
void setMinificationFilter(void);
void setMipMapFilter(void);

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
                             "Direct3D (DX9) - Texturing Filtering",
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
					++g_MinFilterType;
					if(g_MinFilterType > 3)
						g_MinFilterType = 0;
					g_bChangeFilters = true;
					break;

				case VK_F2:
					++g_MagFilterType;
					if(g_MagFilterType > 3)
						g_MagFilterType = 0;
					g_bChangeFilters = true;
					break;
           
                case VK_F3:
                    ++g_MipFilterType;
					if(g_MipFilterType > 3)
						g_MipFilterType = 0;
					g_bChangeFilters = true;
					break;

				case VK_F4:
                    g_fMipMapLodBias += 1.0f;
					g_bChangeFilters = true;
                    break;

				case VK_F5:
                    g_fMipMapLodBias -= 1.0f;
					g_bChangeFilters = true;
					break;

                case VK_F6:
                    g_nAnisotropy += 1;
					g_bChangeFilters = true;
                    break;

				case VK_F7:
                    g_nAnisotropy -= 1;
					g_bChangeFilters = true;
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
	D3DXCreateTextureFromFile( g_pd3dDevice, "test.bmp", &g_pTexture );

	g_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_NONE);
	g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_NONE);
    g_pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
}

//-----------------------------------------------------------------------------
// Name: createFont()
// Desc: 
//-----------------------------------------------------------------------------
void createFont( void )
{
    //
    // To create a Windows friendly font using only a point size, an 
    // application must calculate the logical height of the font.
    // 
    // This is because functions like CreateFont() and CreateFontIndirect() 
    // only use logical units to specify height.
    //
    // Here's the formula to find the height in logical pixels:
    //
    //             -( point_size * LOGPIXELSY )
    //    height = ----------------------------
    //                          72
    //

    HRESULT hr;
    HDC hDC;
    //HFONT hFont;
    int nHeight;
    int nPointSize = 9;
    //char strFontName[] = "Arial";

    hDC = GetDC( NULL );

    nHeight = -( MulDiv( nPointSize, GetDeviceCaps(hDC, LOGPIXELSY), 72 ) );

    ReleaseDC( NULL, hDC );

    // Create a font for statistics and help output
    hr = D3DXCreateFont( g_pd3dDevice, nHeight, 0, FW_BOLD, 0, FALSE, 
                         DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, 
                         DEFAULT_PITCH | FF_DONTCARE, TEXT("Arial"), 
                         &g_pd3dxFont );

    if( FAILED( hr ) )
        MessageBox(NULL,"Call to D3DXCreateFont failed!", "ERROR",MB_OK|MB_ICONEXCLAMATION);
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
    createFont();

	g_pd3dDevice->CreateVertexBuffer( 4*sizeof(Vertex), D3DUSAGE_WRITEONLY, 
                                      Vertex::FVF_Flags, D3DPOOL_DEFAULT, 
                                      &g_pVertexBuffer, NULL );
    void *pVertices = NULL;

    g_pVertexBuffer->Lock( 0, sizeof(g_quadVertices), (void**)&pVertices, 0 );
    memcpy( pVertices, g_quadVertices, sizeof(g_quadVertices) );
    g_pVertexBuffer->Unlock();

    D3DXMATRIX matProj;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DXToRadian( 45.0f ), 
                                640.0f / 480.0f, 0.1f, 100.0f );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )
{
    if( g_pd3dxFont != NULL )
        g_pd3dxFont->Release();

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
// Name : setMinificationFilter()
// Desc : 
//-----------------------------------------------------------------------------
void setMinificationFilter( void )
{
    if( g_MinFilterType == 0 )
        g_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_NONE);

    if( g_MinFilterType == 1 )
        g_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);

    if( g_MinFilterType == 2 )
        g_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);

    if( g_MinFilterType == 3 )
        g_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
}

//-----------------------------------------------------------------------------
// Name : setMagnificationFilter()
// Desc : 
//-----------------------------------------------------------------------------
void setMagnificationFilter( void )
{
    if( g_MagFilterType == 0 )
        g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_NONE);

    if( g_MagFilterType == 1 )
        g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);

    if( g_MagFilterType == 2 )
        g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

    if( g_MagFilterType == 3 )
        g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
}

//-----------------------------------------------------------------------------
// Name : setMipMapFilter()
// Desc : 
//-----------------------------------------------------------------------------
void setMipMapFilter( void )
{
    if( g_MipFilterType == 0 )
        g_pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

    if( g_MipFilterType == 1 )
        g_pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);

    if( g_MipFilterType == 2 )
        g_pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

    if( g_MipFilterType == 3 )
        g_pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_ANISOTROPIC);
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

    D3DXMatrixTranslation( &matTrans, 0.0f, 0.0f, g_fDistance );

	D3DXMatrixRotationYawPitchRoll( &matRot, 
		                            D3DXToRadian(g_fSpinX), 
		                            D3DXToRadian(g_fSpinY), 
		                            0.0f );
    matWorld = matRot * matTrans;
    g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

    g_pd3dDevice->BeginScene();

    if( g_bChangeFilters == true )
	{
		setMinificationFilter();
		setMagnificationFilter();
		setMipMapFilter();
		g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAXANISOTROPY, g_nAnisotropy );
		g_pd3dDevice->SetSamplerState(0, D3DSAMP_MIPMAPLODBIAS, *((LPDWORD)(&g_fMipMapLodBias)));
		g_bChangeFilters = false;
	}

	g_pd3dDevice->SetTexture( 0, g_pTexture );
	
    g_pd3dDevice->SetStreamSource( 0, g_pVertexBuffer, 0, sizeof(Vertex) );
    g_pd3dDevice->SetFVF( Vertex::FVF_Flags );
	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,  0, 2 );

    //
	// Output the current settings...
	//

	RECT destRect1;
	SetRect( &destRect1, 5, 5, 0, 0 );

	RECT destRect2;
	SetRect( &destRect2, 5, 20, 0, 0 );

    RECT destRect3;
    SetRect( &destRect3, 5, 35, 0, 0 );

	static char strMinFilter[255];
	static char strMagFilter[255];
    static char strMipFilter[255];

	if( g_MinFilterType == FILTER_TYPE_NONE )
        sprintf( strMinFilter, "D3DSAMP_MINFILTER = D3DTEXF_NONE    (Change: F1)" );
	else if( g_MinFilterType == FILTER_TYPE_POINT )
		sprintf( strMinFilter, "D3DSAMP_MINFILTER = D3DTEXF_POINT    (Change: F1)" );
	else if( g_MinFilterType == FILTER_TYPE_LINEAR )
		sprintf( strMinFilter, "D3DSAMP_MINFILTER = D3DTEXF_LINEAR    (Change: F1)" );
	else if( g_MinFilterType == FILTER_TYPE_ANISOTROPIC )
		sprintf( strMinFilter, "D3DSAMP_MINFILTER = D3DTEXF_ANISOTROPIC    (Change: F1)" );

	if( g_MagFilterType == FILTER_TYPE_NONE )
		sprintf( strMagFilter, "D3DSAMP_MAGFILTER = D3DTEXF_NONE    (Change: F2)" );
	else if( g_MagFilterType == FILTER_TYPE_POINT )
		sprintf( strMagFilter, "D3DSAMP_MAGFILTER = D3DTEXF_POINT    (Change: F2)" );
	else if( g_MagFilterType == FILTER_TYPE_LINEAR )
		sprintf( strMagFilter, "D3DSAMP_MAGFILTER = D3DTEXF_LINEAR    (Change: F2)" );
	else if( g_MagFilterType == FILTER_TYPE_ANISOTROPIC )
		sprintf( strMagFilter, "D3DSAMP_MAGFILTER = D3DTEXF_ANISOTROPIC    (Change: F2)" );

    if( g_MipFilterType == FILTER_TYPE_NONE )
        sprintf( strMipFilter, "D3DSAMP_MIPFILTER = D3DTEXF_NONE    (Change: F3)" );
    else if( g_MipFilterType == FILTER_TYPE_POINT )
        sprintf( strMipFilter, "D3DSAMP_MIPFILTER = D3DTEXF_POINT    (Change: F3)" );
    else if( g_MipFilterType == FILTER_TYPE_LINEAR )
        sprintf( strMipFilter, "D3DSAMP_MIPFILTER = D3DTEXF_LINEAR    (Change: F3)" );
    else if( g_MipFilterType == FILTER_TYPE_ANISOTROPIC )
        sprintf( strMipFilter, "D3DSAMP_MIPFILTER = D3DTEXF_ANISOTROPIC    (Change: F3)" );

	g_pd3dxFont->DrawText( NULL, strMinFilter, -1, &destRect1, DT_NOCLIP, 
		                   D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );

	g_pd3dxFont->DrawText( NULL, strMagFilter, -1, &destRect2, DT_NOCLIP, 
		                   D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );

    g_pd3dxFont->DrawText( NULL, strMipFilter, -1, &destRect3, DT_NOCLIP, 
                           D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );

    g_pd3dDevice->EndScene();
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}
