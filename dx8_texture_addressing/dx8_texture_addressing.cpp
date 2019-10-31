//-----------------------------------------------------------------------------
//           Name: dx8_texture_addressing.cpp
//         Author: Kevin Harris
//  Last Modified: 03/03/04
//    Description: This sample demonstrates the five methods of texture  
//                 addressing that are available under Direct3D:
//
//                 D3DTADDRESS_WRAP
//                 D3DTADDRESS_CLAMP
//                 D3DTADDRESS_MIRROR
//                 D3DTADDRESS_BORDER
//                 D3DTADDRESS_MIRRORONCE
//
//   Control Keys: F1 - Changes addressing method for the U coordinates
//                 F2 - Changes addressing method for the V coordinates
//                 F3 - Changes addressing method for the W coordinates 
//
// Note: Changes to F3 will only be visible if the sample loads a 
//       volume-texture, which this one doesn't. It's only listed here for 
//       completeness and future expansion.
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <d3d8.h>
#include <d3dx8.h>
#include "resource.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND                    g_hWnd           = NULL;
LPDIRECT3D8             g_pD3D           = NULL;
LPDIRECT3DDEVICE8       g_pd3dDevice     = NULL;
LPDIRECT3DVERTEXBUFFER8 g_pVertexBuffer  = NULL;
LPDIRECT3DTEXTURE8      g_pTexture       = NULL;
LPDIRECT3DTEXTURE8      g_pMipMapTexture = NULL;

struct Vertex
{
    float x, y, z;
    float tu, tv;
};

Vertex g_quadVertices[] =
{
	{-1.0f, 1.0f, 0.0f,  0.0f,0.0f },
	{ 1.0f, 1.0f, 0.0f,  2.0f,0.0f },
	{-1.0f,-1.0f, 0.0f,  0.0f,2.0f },
	{ 1.0f,-1.0f, 0.0f,  2.0f,2.0f }
};

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_TEX1)

enum AddressingMethods
{
	ADDRESSING_METHOD_WRAP = 0,
	ADDRESSING_METHOD_CLAMP,
	ADDRESSING_METHOD_MIRROR,
	ADDRESSING_METHOD_BORDER,
	ADDRESSING_METHOD_MIRRORONCE
};

int	 g_addressingMethod_U = ADDRESSING_METHOD_WRAP;
int	 g_addressingMethod_V = ADDRESSING_METHOD_WRAP;
int	 g_addressingMethod_W = ADDRESSING_METHOD_WRAP;
bool g_bChangeAddressingMethod = true;

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
void setAddresingFor_V_Coordinate(void);
void setAddresingFor_U_Coordinate(void);
void setAddresingFor_W_Coordinate(void);

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
                             "Direct3D (DX8) - Texture Addressing",
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
					++g_addressingMethod_U;
					if(g_addressingMethod_U > 4)
						g_addressingMethod_U = 0;
					g_bChangeAddressingMethod = true;
					break;

				case VK_F2:
					++g_addressingMethod_V;
					if(g_addressingMethod_V > 4)
						g_addressingMethod_V = 0;
					g_bChangeAddressingMethod = true;
					break;
           
                case VK_F3:
                    ++g_addressingMethod_W;
					if(g_addressingMethod_W > 4)
						g_addressingMethod_W = 0;
					g_bChangeAddressingMethod = true;
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
	D3DXCreateTextureFromFile( g_pd3dDevice, "five.bmp", &g_pTexture );

	g_pd3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
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
    d3dpp.SwapEffect             = D3DSWAPEFFECT_FLIP;
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
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: Releases all previously initialized objects
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
// Desc: Draws the scene
//-----------------------------------------------------------------------------
void render( void )
{
    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                         D3DCOLOR_COLORVALUE(0.0f,0.0f,0.0f,1.0f), 1.0f, 0 );

    D3DXMATRIX matWorld;
    D3DXMatrixTranslation( &matWorld, 0.0f, 0.0f, 3.0f );
    g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

    g_pd3dDevice->BeginScene();

    if( g_bChangeAddressingMethod == true )
	{
		setAddresingFor_U_Coordinate();
		setAddresingFor_V_Coordinate();
		setAddresingFor_W_Coordinate();
		g_bChangeAddressingMethod = false;
	}

	g_pd3dDevice->SetTexture( 0, g_pTexture );
    g_pd3dDevice->SetStreamSource( 0, g_pVertexBuffer, sizeof(Vertex) );
    g_pd3dDevice->SetVertexShader( D3DFVF_CUSTOMVERTEX );
	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );

    g_pd3dDevice->EndScene();
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}

//-----------------------------------------------------------------------------
// Name : setAddresingFor_U_Coordinate()
// Desc : 
//-----------------------------------------------------------------------------
void setAddresingFor_U_Coordinate( void )
{  
	if( g_addressingMethod_U == 0 )
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP );

	if( g_addressingMethod_U == 1 )
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP );

	if( g_addressingMethod_U == 2 )
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_MIRROR );

	if( g_addressingMethod_U == 3 )
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_BORDER );

	if( g_addressingMethod_U == 4 )
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_MIRRORONCE );
}

//-----------------------------------------------------------------------------
// Name : setAddresingFor_V_Coordinate()
// Desc : 
//-----------------------------------------------------------------------------
void setAddresingFor_V_Coordinate( void )
{  
	if( g_addressingMethod_V == 0 )
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP );

	if( g_addressingMethod_V == 1 )
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP );

	if( g_addressingMethod_V == 2 )
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_MIRROR );

	if( g_addressingMethod_V == 3 )
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_BORDER );

	if( g_addressingMethod_V == 4 )
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_MIRRORONCE );
}

//-----------------------------------------------------------------------------
// Name : setAddresingFor_W_Coordinate()
// Desc : 
//-----------------------------------------------------------------------------
void setAddresingFor_W_Coordinate( void )
{
	if( g_addressingMethod_W == 0 )
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSW, D3DTADDRESS_WRAP );

	if( g_addressingMethod_W == 1 )
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP );

	if( g_addressingMethod_W == 2 )
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSW, D3DTADDRESS_MIRROR );

	if( g_addressingMethod_W == 3 )
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSW, D3DTADDRESS_BORDER );

	if( g_addressingMethod_W == 4 )
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSW, D3DTADDRESS_MIRRORONCE );
}
