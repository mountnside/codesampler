//-----------------------------------------------------------------------------
//           Name: dx8_texture_mipmapping.cpp
//         Author: Kevin Harris
//  Last Modified: 03/03/04
//    Description: This sample demonstrates how to mip-map textures with 
//                 Direct3D.
//
//   Control Keys: F1 - Change Minification filter
//                 F2 - Change Magnification filter
//                 F3 - Change Mip-Map filter
//                 F4 - Increase Mip-Map LOD Bias
//                 F5 - Decrease Mip-Map LOD Bias
//                 F6 - Increase Max Anisotropy value
//                 F7 - Decrease Max Anisotropy value
//
//                 Up         - View moves forward
//                 Down       - View moves backward
//                 Left       - View strafes left
//                 Right      - View strafes Right
//                 Left Mouse - Perform looking
//                 Mouse      - Look about the scene
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
HWND                    g_hWnd           = NULL;
LPDIRECT3D8             g_pD3D           = NULL;
LPDIRECT3DDEVICE8       g_pd3dDevice     = NULL;
LPDIRECT3DVERTEXBUFFER8 g_pVertexBuffer  = NULL;
LPDIRECT3DTEXTURE8      g_pMipMapTexture = NULL;

struct Vertex
{
    float x, y, z;
    float tu, tv;
};

Vertex g_quadVertices[] =
{
	{-1.0f, 0.0f,-1.0f,  0.0f, 1.0f },
	{-1.0f, 0.0f, 1.0f,  0.0f, 0.0f },
	{ 1.0f, 0.0f,-1.0f,  1.0f, 1.0f },
	{ 1.0f, 0.0f, 1.0f,  1.0f, 0.0f }
};

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_TEX1)

enum FilterTypes
{
	FILTER_TYPE_NONE = 0,
	FILTER_TYPE_POINT,
	FILTER_TYPE_LINEAR,
	FILTER_TYPE_ANISOTROPIC
};

int	  g_MagFilterType  = FILTER_TYPE_LINEAR;
int	  g_MinFilterType  = FILTER_TYPE_LINEAR;
int	  g_mipFilterType  = FILTER_TYPE_LINEAR;
int   g_nAnisotropy    = 1;
float g_fMipMapLodBias = 0.0f;
bool  g_bChangeFilters = true;

float  g_fMoveSpeed = 25.0f;
float  g_fElpasedTime;
double g_dCurTime;
double g_dLastTime;

D3DXVECTOR3	g_vEye(0.0f, 10.0f, 0.0f);  // Camera Position
D3DXVECTOR3	g_vLook(0.5f, -0.4f, 0.5f); // Camera Look Vector
D3DXVECTOR3	g_vUp(0.0f, 1.0f, 0.0f);    // Camera Up Vector
D3DXVECTOR3	g_vRight(1.0f, 0.0f, 0.0f); // Camera Right Vector

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void init(void);
void shutDown(void);
void render(void);
void loadMipMapTexture(void);
void setMagnificationFilter(void);
void setMinificationFilter(void);
void setMipMapFilter(void);
void updateViewMatrix(void);

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
                             "Direct3D (DX8) - Texture Mipmapping",
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
			g_dCurTime     = timeGetTime();
			g_fElpasedTime = (float)((g_dCurTime - g_dLastTime) * 0.001);
			g_dLastTime    = g_dCurTime;

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
	D3DXVECTOR3 tmpLook  = g_vLook;
	D3DXVECTOR3 tmpRight = g_vRight;
	D3DXMATRIX mtxRot;

	static POINT ptLastMousePosit;
	static POINT ptCurrentMousePosit;
	static bool bMousing;

    switch( msg )
	{
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
				int nYDiff = (ptCurrentMousePosit.y - ptLastMousePosit.y);
				int nXDiff = (ptCurrentMousePosit.x - ptLastMousePosit.x);

				if( nYDiff != 0 )
				{
					D3DXMatrixRotationAxis( &mtxRot, &g_vRight, D3DXToRadian((float)nYDiff / 3.0f));
					D3DXVec3TransformCoord( &g_vLook, &g_vLook, &mtxRot );
					D3DXVec3TransformCoord( &g_vUp, &g_vUp, &mtxRot );
				}

				if( nXDiff != 0 )
				{
					D3DXMatrixRotationAxis( &mtxRot, &D3DXVECTOR3(0,1,0), D3DXToRadian((float)nXDiff / 3.0f) );
					D3DXVec3TransformCoord( &g_vLook, &g_vLook, &mtxRot );
					D3DXVec3TransformCoord( &g_vUp, &g_vUp, &mtxRot );
				}
			}
			
			ptLastMousePosit.x = ptCurrentMousePosit.x;
            ptLastMousePosit.y = ptCurrentMousePosit.y;
		}
		break;

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
                    ++g_mipFilterType;
					if(g_mipFilterType > 3)
						g_mipFilterType = 0;
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

				case VK_UP:
					g_vEye += tmpLook*g_fMoveSpeed*g_fElpasedTime;
					break;

				case VK_DOWN:
					g_vEye -= tmpLook*g_fMoveSpeed*g_fElpasedTime;
					break;

				case VK_LEFT:
					g_vEye -= tmpRight*g_fMoveSpeed*g_fElpasedTime;
					break;

				case VK_RIGHT:
					g_vEye += tmpRight*g_fMoveSpeed*g_fElpasedTime;
					break;

				case VK_HOME:
					g_vEye.y += g_fMoveSpeed*g_fElpasedTime; 
					break;

				case VK_END:
					g_vEye.y -= g_fMoveSpeed*g_fElpasedTime;
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
// Name: loadMipMapTexture()
// Desc: 
//-----------------------------------------------------------------------------
void loadMipMapTexture( void )
{
	// 256 = red
	// 128 = blue
	//  64 = green
	//  32 = yellow
	//  16 = purple
	//   8 = light blue

	LPDIRECT3DTEXTURE8 pTexArray[6];

	// Load our 6 custom made mip-map textures as regular textures
    D3DXCreateTextureFromFile( g_pd3dDevice, "tex256.bmp", &pTexArray[0] );
	D3DXCreateTextureFromFile( g_pd3dDevice, "tex128.bmp", &pTexArray[1] );
	D3DXCreateTextureFromFile( g_pd3dDevice, "tex64.bmp",  &pTexArray[2] );
	D3DXCreateTextureFromFile( g_pd3dDevice, "tex32.bmp",  &pTexArray[3] );
	D3DXCreateTextureFromFile( g_pd3dDevice, "tex16.bmp",  &pTexArray[4] );
	D3DXCreateTextureFromFile( g_pd3dDevice, "tex8.bmp",   &pTexArray[5] );
	
	// Now, create a mip-map texture with 6 mip-map levels
	D3DXCreateTextureFromFile( g_pd3dDevice, "texbase256.bmp", &g_pMipMapTexture );
	g_pd3dDevice->CreateTexture( 256, 256, 6, 0, D3DFMT_UNKNOWN,
								 D3DPOOL_MANAGED, &g_pMipMapTexture );

	IDirect3DSurface8 *pDestSurface;
	IDirect3DSurface8 *pSrcSurface;

	int nNumLevels = g_pMipMapTexture->GetLevelCount();
	int i;

	for( i = 0; i < 6; ++i )
	{
		g_pMipMapTexture->GetSurfaceLevel( i, &pDestSurface );
		pTexArray[i]->GetSurfaceLevel( 0, &pSrcSurface );

		// Load our custom mip-map texture into the appropiate level by 
		// copying over or erasing the existing texture on that surface.
		g_pd3dDevice->CopyRects( pSrcSurface, NULL, 0, pDestSurface,NULL );

		pSrcSurface->Release();
		pDestSurface->Release();
	}

	for( i = 0; i < 6; ++i )
		pTexArray[i]->Release();
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

	loadMipMapTexture();

	g_pd3dDevice->CreateVertexBuffer( 4*sizeof(Vertex),0, D3DFVF_CUSTOMVERTEX,
                                      D3DPOOL_DEFAULT, &g_pVertexBuffer );
	void *pVertices = NULL;

    g_pVertexBuffer->Lock( 0, sizeof(g_quadVertices), (BYTE**)&pVertices, 0 );
    memcpy( pVertices, g_quadVertices, sizeof(g_quadVertices) );
    g_pVertexBuffer->Unlock();

    g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );

    D3DXMATRIX matProj;
    D3DXMatrixPerspectiveFovLH( &matProj, 45.0f, 640.0f / 480.0f, 0.1f, 500.0f );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )
{
	if( g_pMipMapTexture != NULL )
        g_pMipMapTexture->Release();

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

	updateViewMatrix();

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

    g_pd3dDevice->SetTexture( 0, g_pMipMapTexture );
    g_pd3dDevice->SetStreamSource( 0, g_pVertexBuffer, sizeof(Vertex) );
    g_pd3dDevice->SetVertexShader( D3DFVF_CUSTOMVERTEX );

    D3DXMATRIX matFloor;
	float x = 0.0f;
	float z = 0.0f;

	for( int i = 0; i < 25; ++i )
    {
		for( int j = 0; j < 25; ++j )
		{
			D3DXMatrixTranslation( &matFloor, x, 0.0f, z );
			g_pd3dDevice->SetTransform( D3DTS_WORLD, &matFloor );
			g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2);

			x += 2.0f;
		}
		x  =  0.0f;
		z += 2.0f;
	}

    g_pd3dDevice->EndScene();
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
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

//-----------------------------------------------------------------------------
// Name : updateViewMatrix()
// Desc : 
//-----------------------------------------------------------------------------
void updateViewMatrix( void )
{
	D3DXMATRIX view;
	D3DXMatrixIdentity( &view );

	D3DXVec3Normalize( &g_vLook, &g_vLook );
	D3DXVec3Cross( &g_vRight, &g_vUp, &g_vLook );
	D3DXVec3Normalize( &g_vRight, &g_vRight );
	D3DXVec3Cross( &g_vUp, &g_vLook, &g_vRight );
	D3DXVec3Normalize( &g_vUp, &g_vUp );

	view._11 = g_vRight.x;
    view._12 = g_vUp.x;
    view._13 = g_vLook.x;
	view._14 = 0.0f;

	view._21 = g_vRight.y;
    view._22 = g_vUp.y;
    view._23 = g_vLook.y;
	view._24 = 0.0f;

	view._31 = g_vRight.z;
    view._32 = g_vUp.z;
    view._33 = g_vLook.z;
	view._34 = 0.0f;

	view._41 = -D3DXVec3Dot( &g_vEye, &g_vRight );
	view._42 = -D3DXVec3Dot( &g_vEye, &g_vUp );
	view._43 = -D3DXVec3Dot( &g_vEye, &g_vLook );
	view._44 =  1.0f;

	g_pd3dDevice->SetTransform( D3DTS_VIEW, &view ); 
}
