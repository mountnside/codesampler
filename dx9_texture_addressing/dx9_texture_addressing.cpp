//-----------------------------------------------------------------------------
//           Name: dx9_texture_addressing.cpp
//         Author: Kevin Harris
//  Last Modified: 02/01/05
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
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "resource.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND                    g_hWnd           = NULL;
LPDIRECT3D9             g_pD3D           = NULL;
LPDIRECT3DDEVICE9       g_pd3dDevice     = NULL;
D3DDEVTYPE              g_deviceType     = D3DDEVTYPE_HAL;
LPD3DXFONT              g_pd3dxFont      = NULL;
LPDIRECT3DVERTEXBUFFER9 g_pVertexBuffer  = NULL;
LPDIRECT3DTEXTURE9      g_pTexture       = NULL;
LPDIRECT3DTEXTURE9      g_pMipMapTexture = NULL;

// Set the border color to purple.
DWORD g_dwBorderColor = D3DCOLOR_COLORVALUE( 1.0f, 0.0f, 1.0f, 1.0f );

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
	{ 1.0f, 1.0f, 0.0f,  3.0f,0.0f },
	{-1.0f,-1.0f, 0.0f,  0.0f,3.0f },
	{ 1.0f,-1.0f, 0.0f,  3.0f,3.0f }
};

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
bool g_bChangeAddressingMethod = true;

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
void setAddresingFor_V_Coordinate(void);
void setAddresingFor_U_Coordinate(void);

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
                             "Direct3D (DX9) - Texture Addressing",
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
void loadTexture( void )
{
	D3DXCreateTextureFromFile( g_pd3dDevice, "five.bmp", &g_pTexture );

	g_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
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

	//
	// Check Direct3D driver for hardware support...
	//

	D3DCAPS9 d3dCaps;

	if( FAILED( g_pD3D->GetDeviceCaps( D3DADAPTER_DEFAULT,
		                               D3DDEVTYPE_HAL, 
									   &d3dCaps ) ) )
	{
		// TO DO: Respond to failure of GetDeviceCaps
		return;
	}

	if( (d3dCaps.TextureAddressCaps & D3DPTADDRESSCAPS_WRAP) == 0 )
	{
		MessageBox(NULL,"Current Direct3D driver does not support "
			"D3DTADDRESS_WRAP! \n\nSwitching to reference rasterizer!",
			"WARNING",MB_OK|MB_ICONEXCLAMATION);
		g_deviceType = D3DDEVTYPE_REF;
	}

	if( (d3dCaps.TextureAddressCaps & D3DPTADDRESSCAPS_CLAMP) == 0 )
	{
		MessageBox(NULL,"Current Direct3D driver does not support "
			"D3DPTADDRESSCAPS_CLAMP! \n\nSwitching to reference rasterizer!",
			"WARNING",MB_OK|MB_ICONEXCLAMATION);
		g_deviceType = D3DDEVTYPE_REF;
	}

	if( (d3dCaps.TextureAddressCaps & D3DPTADDRESSCAPS_MIRROR) == 0 )
	{
		MessageBox(NULL,"Current Direct3D driver does not support "
			"D3DPTADDRESSCAPS_MIRROR! \n\nSwitching to reference rasterizer!",
			"WARNING",MB_OK|MB_ICONEXCLAMATION);
		g_deviceType = D3DDEVTYPE_REF;
	}

	if( (d3dCaps.TextureAddressCaps & D3DPTADDRESSCAPS_BORDER) == 0 )
	{
		MessageBox(NULL,"Current Direct3D driver does not support "
			"D3DPTADDRESSCAPS_BORDER! \n\nSwitching to reference rasterizer!",
			"WARNING",MB_OK|MB_ICONEXCLAMATION);
		g_deviceType = D3DDEVTYPE_REF;
	}

	if( (d3dCaps.TextureAddressCaps & D3DPTADDRESSCAPS_MIRRORONCE) == 0 )
	{
		MessageBox(NULL,"Current Direct3D driver does not support "
			"D3DPTADDRESSCAPS_MIRRORONCE! \n\nSwitching to reference rasterizer!",
			"WARNING",MB_OK|MB_ICONEXCLAMATION);
		g_deviceType = D3DDEVTYPE_REF;
	}

	//
	// Set up the D3DPRESENT_PARAMETERS data structure...
	//

    D3DDISPLAYMODE d3ddm;

    g_pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm );

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof(d3dpp) );

    d3dpp.Windowed               = TRUE;
    d3dpp.SwapEffect             = D3DSWAPEFFECT_FLIP;
    d3dpp.BackBufferFormat       = d3ddm.Format;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_IMMEDIATE;

    g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, g_deviceType, g_hWnd,
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
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: Releases all previously initialized objects
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
// Name : setAddresingFor_U_Coordinate()
// Desc : 
//-----------------------------------------------------------------------------
void setAddresingFor_U_Coordinate( void )
{
	if( g_addressingMethod_U == ADDRESSING_METHOD_WRAP )
		g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
	else if( g_addressingMethod_U == ADDRESSING_METHOD_CLAMP )
		g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
	else if( g_addressingMethod_U == ADDRESSING_METHOD_MIRROR )
		g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_MIRROR );
	else if( g_addressingMethod_U == ADDRESSING_METHOD_BORDER )
		g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER );
	else if( g_addressingMethod_U == ADDRESSING_METHOD_MIRRORONCE )
		g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_MIRRORONCE );
}

//-----------------------------------------------------------------------------
// Name : setAddresingFor_V_Coordinate()
// Desc : 
//-----------------------------------------------------------------------------
void setAddresingFor_V_Coordinate( void )
{
	if( g_addressingMethod_V == ADDRESSING_METHOD_WRAP )
		g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );
	else if( g_addressingMethod_V == ADDRESSING_METHOD_CLAMP )
		g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
	else if( g_addressingMethod_V == ADDRESSING_METHOD_MIRROR )
		g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_MIRROR );
	else if( g_addressingMethod_V == ADDRESSING_METHOD_BORDER )
		g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER );
	else if( g_addressingMethod_V == ADDRESSING_METHOD_MIRRORONCE )
		g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_MIRRORONCE );
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
    D3DXMatrixTranslation( &matWorld, 0.0f, 0.0f, 4.0f );
    g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

    g_pd3dDevice->BeginScene();

    if( g_bChangeAddressingMethod == true )
	{
		setAddresingFor_U_Coordinate();
		setAddresingFor_V_Coordinate();

		// Set the border color. This is used by D3DTADDRESS_BORDER.
		g_pd3dDevice->SetSamplerState( 0, D3DSAMP_BORDERCOLOR, g_dwBorderColor );

		g_bChangeAddressingMethod = false;
	}

	g_pd3dDevice->SetTexture( 0, g_pTexture );
    g_pd3dDevice->SetStreamSource( 0, g_pVertexBuffer, 0, sizeof(Vertex) );
    g_pd3dDevice->SetFVF( Vertex::FVF_Flags );
	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );

	//
	// Output the current settings...
	//

	RECT destRect1;
	SetRect( &destRect1, 5, 5, 0, 0 );

	RECT destRect2;
	SetRect( &destRect2, 5, 20, 0, 0 );

	static char strU[255];
	static char strV[255];

	if( g_addressingMethod_U == ADDRESSING_METHOD_WRAP )
        sprintf( strU, "D3DSAMP_ADDRESSU = D3DTADDRESS_WRAP    (Change: F1)" );
	else if( g_addressingMethod_U == ADDRESSING_METHOD_CLAMP )
		sprintf( strU, "D3DSAMP_ADDRESSU = D3DTADDRESS_CLAMP    (Change: F1)" );
	else if( g_addressingMethod_U == ADDRESSING_METHOD_MIRROR )
		sprintf( strU, "D3DSAMP_ADDRESSU = D3DTADDRESS_MIRROR    (Change: F1)" );
	else if( g_addressingMethod_U == ADDRESSING_METHOD_BORDER )
		sprintf( strU, "D3DSAMP_ADDRESSU = D3DTADDRESS_BORDER    (Change: F1)" );
	else if( g_addressingMethod_U == ADDRESSING_METHOD_MIRRORONCE )
		sprintf( strU, "D3DSAMP_ADDRESSU = D3DTADDRESS_MIRRORONCE    (Change: F1)" );

	if( g_addressingMethod_V == ADDRESSING_METHOD_WRAP )
		sprintf( strV, "D3DSAMP_ADDRESSV = D3DTADDRESS_WRAP    (Change: F2)" );
	else if( g_addressingMethod_V == ADDRESSING_METHOD_CLAMP )
		sprintf( strV, "D3DSAMP_ADDRESSV = D3DTADDRESS_CLAMP    (Change: F2)" );
	else if( g_addressingMethod_V == ADDRESSING_METHOD_MIRROR )
		sprintf( strV, "D3DSAMP_ADDRESSV = D3DTADDRESS_MIRROR    (Change: F2)" );
	else if( g_addressingMethod_V == ADDRESSING_METHOD_BORDER )
		sprintf( strV, "D3DSAMP_ADDRESSV = D3DTADDRESS_BORDER    (Change: F2)" );
	else if( g_addressingMethod_V == ADDRESSING_METHOD_MIRRORONCE )
		sprintf( strV, "D3DSAMP_ADDRESSV = D3DTADDRESS_MIRRORONCE    (Change: F2)" );

	g_pd3dxFont->DrawText( NULL, strU, -1, &destRect1, DT_NOCLIP, 
		                   D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );

	g_pd3dxFont->DrawText( NULL, strV, -1, &destRect2, DT_NOCLIP, 
		                   D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );

    g_pd3dDevice->EndScene();
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}
