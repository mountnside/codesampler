//-----------------------------------------------------------------------------
//           Name: dx8_texture_filtering.cpp
//         Author: Kevin Harris
//  Last Modified: 03/03/04
//    Description: This sample demonstrates how to filter textures with 
//                 Direct3D.
//
//                 Direct3D texture filtering is controlled by calling 
//                 SetTextureStageState() and setting either D3DTSS_MINFILTER, 
//                 D3DTSS_MAGFILTER, or D3DTSS_MIPFILTER to one of the 
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
//
// Note: This sample doesn't support Mip-Mapping yet, so F4 - F6 will have no
//       noticeable effect. They're only listed here for completeness and 
//       future expansion.
//
// ----------------------------------------------------------------------------
// 
// D3DTSS_MINFILTER
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
// D3DTSS_MAGFILTER
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
// D3DTSS_MIPFILTER
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

#include <d3d8.h>
#include <d3dx8.h>
#include <mmsystem.h>
#include "resource.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND                    g_hWnd          = NULL;
LPDIRECT3D8             g_pD3D          = NULL;
LPDIRECT3DDEVICE8       g_pd3dDevice    = NULL;
LPDIRECT3DVERTEXBUFFER8 g_pVertexBuffer = NULL;
LPDIRECT3DTEXTURE8      g_pTexture      = NULL;

float  g_fElpasedTime;
double g_dCurrentTime;
double g_dLastTime;

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

enum FilterTypes
{
	FILTER_TYPE_NONE = 0,
	FILTER_TYPE_POINT,
	FILTER_TYPE_LINEAR,
	FILTER_TYPE_ANISOTROPIC
};

int	  g_MinFilterType  = FILTER_TYPE_NONE;
int	  g_MagFilterType  = FILTER_TYPE_NONE;
int	  g_mipFilterType  = FILTER_TYPE_NONE;
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
                             "Direct3D (DX8) - Texturing Filtering",
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
        case WM_KEYDOWN:
		{
			switch( wParam )
			{
				case VK_ESCAPE:
					PostQuitMessage(0);
					break;

                case 112: // F1
					++g_MinFilterType;
					if(g_MinFilterType > 3)
						g_MinFilterType = 0;
					g_bChangeFilters = true;
					break;

				case 113: // F2
					++g_MagFilterType;
					if(g_MagFilterType > 3)
						g_MagFilterType = 0;
					g_bChangeFilters = true;
					break;
           
                case 114: // F3
                    ++g_mipFilterType;
					if(g_mipFilterType > 3)
						g_mipFilterType = 0;
					g_bChangeFilters = true;
					break;

				case 115: // F4
                    g_fMipMapLodBias += 1.0f;
					g_bChangeFilters = true;
                    break;

				case 116: // F5
                    g_fMipMapLodBias -= 1.0f;
					g_bChangeFilters = true;
					break;

                case 117: // F6
                    g_nAnisotropy += 1;
					g_bChangeFilters = true;
                    break;

				case 118: // F7
                    g_nAnisotropy -= 1;
					g_bChangeFilters = true;
					break;
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
// Name: loadTexture()
// Desc: 
//-----------------------------------------------------------------------------
void loadTexture(void)
{
	D3DXCreateTextureFromFile( g_pd3dDevice, "testpattern.bmp", &g_pTexture );

	g_pd3dDevice->CreateTexture( 128, 128, 0, 0, D3DFMT_UNKNOWN,
		                         D3DPOOL_MANAGED, &g_pTexture );
}

//-----------------------------------------------------------------------------
// Name: init()
// Desc: 
//-----------------------------------------------------------------------------
void init( void )
{
    g_pD3D = Direct3DCreate8( D3D_SDK_VERSION );

    D3DDISPLAYMODE d3ddm;

    g_pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm );

    D3DPRESENT_PARAMETERS d3dpp;

    ZeroMemory( &d3dpp, sizeof(d3dpp) );
    d3dpp.Windowed               = TRUE;
    d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat       = d3ddm.Format;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

    g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, g_hWnd,
                          D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                          &d3dpp, &g_pd3dDevice );

	loadTexture();

	g_pd3dDevice->CreateVertexBuffer( 4*sizeof(Vertex), D3DUSAGE_WRITEONLY, 
                                      D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, 
                                      &g_pVertexBuffer );
    void *pVertices = NULL;

    g_pVertexBuffer->Lock( 0, sizeof(g_quadVertices), (BYTE**)&pVertices, 0 );
    memcpy( pVertices, g_quadVertices, sizeof(g_quadVertices) );
    g_pVertexBuffer->Unlock();

    D3DXMATRIX matProj;
    D3DXMatrixPerspectiveFovLH( &matProj, 45.0f, 640.0f / 480.0f, 0.1f, 100.0f );
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

    D3DXMatrixTranslation( &matTrans, 0.0f, 0.0f, 3.0f );

	D3DXMatrixRotationYawPitchRoll( &matRot, 
		                            D3DXToRadian(fXrot), 
		                            D3DXToRadian(fYrot), 
		                            D3DXToRadian(fZrot) );

    matWorld = matRot * matTrans;
    g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

    g_pd3dDevice->BeginScene();

    if( g_bChangeFilters == true )
	{
		setMinificationFilter();
		setMagnificationFilter();
		setMipMapFilter();
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_MAXANISOTROPY, g_nAnisotropy );
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_MIPMAPLODBIAS, *((LPDWORD)(&g_fMipMapLodBias)));
		g_bChangeFilters = false;
	}

	g_pd3dDevice->SetTexture( 0, g_pTexture );
	
    g_pd3dDevice->SetStreamSource( 0, g_pVertexBuffer, sizeof(Vertex) );
    g_pd3dDevice->SetVertexShader( D3DFVF_CUSTOMVERTEX );
	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,  0, 2 );

    g_pd3dDevice->EndScene();
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}

//-----------------------------------------------------------------------------
// Name : setMinificationFilter()
// Desc : 
//-----------------------------------------------------------------------------
void setMinificationFilter( void )
{
	if( g_MinFilterType == 0 )
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_NONE);
	
	if( g_MinFilterType == 1 )
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_POINT);

	if( g_MinFilterType == 2 )
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);

	if( g_MinFilterType == 3 )
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_ANISOTROPIC);
}

//-----------------------------------------------------------------------------
// Name : setMagnificationFilter()
// Desc : 
//-----------------------------------------------------------------------------
void setMagnificationFilter( void )
{
	if( g_MagFilterType == 0 )
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_NONE);
	
	if( g_MagFilterType == 1 )
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_POINT);

	if( g_MagFilterType == 2 )
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);

	if( g_MagFilterType == 3 )
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_ANISOTROPIC);
}

//-----------------------------------------------------------------------------
// Name : setMipMapFilter()
// Desc : 
//-----------------------------------------------------------------------------
void setMipMapFilter( void )
{
	if( g_mipFilterType == 0 )
        g_pd3dDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_NONE);

	if( g_mipFilterType == 1 )
	    g_pd3dDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_POINT);
	
	if( g_mipFilterType == 2 )
	    g_pd3dDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
	
	if( g_mipFilterType == 3 )
	    g_pd3dDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_ANISOTROPIC);
}
