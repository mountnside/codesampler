//-----------------------------------------------------------------------------
//           Name: dx8_vs11_shader_simple.cpp
//         Author: Kevin Harris
//  Last Modified: 03/03/04
//    Description: This sample demonstrates how to write a vertex shader or 
//                 vertex program using the Version 1.1 shader implementation 
//                 for Direct3D.
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
DWORD					g_hVertexShader = 0xffffffff;

D3DXMATRIX g_matWorld;
D3DXMATRIX g_matView;
D3DXMATRIX g_matProj;
float      g_fSpinX = 0.0f;
float      g_fSpinY = 0.0f;

struct Vertex
{
    float x, y, z;
    DWORD color;
};

#define D3DFVF_CUSTOMVERTEX ( D3DFVF_XYZ | D3DFVF_DIFFUSE )

Vertex g_quadVertices[] =
{
   //  x     y     z       color
   { -1.0f,-1.0f, 0.0f, 0xffffff00, }, // Bottom-Left,  color = yellow
   {  1.0f,-1.0f, 0.0f, 0xff00ff00, }, // Bottom-Right, color = green
   {  1.0f, 1.0f, 0.0f, 0xffff0000, }, // Top-Right,    color = red
   { -1.0f, 1.0f, 0.0f, 0xff0000ff, }  // Top-Left,     color = blue
};

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void init(void);
void render(void);
void shutDown(void);
void initShader(void);
void setShaderConstants(void);

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
                             "Direct3D (DX8) - Simple vs.1.1 Vertex Shader",
						     WS_OVERLAPPEDWINDOW | WS_VISIBLE,
					         0, 0, 640, 480, NULL, NULL, hInstance, NULL );

	if( g_hWnd == NULL )
		return E_FAIL;

    ShowWindow( g_hWnd, nCmdShow );
    UpdateWindow( g_hWnd );

	init();
	initShader();

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

	g_pd3dDevice->CreateVertexBuffer( 4*sizeof(Vertex), D3DUSAGE_WRITEONLY, 
                                      D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, 
                                      &g_pVertexBuffer );
    void *pVertices = NULL;

    g_pVertexBuffer->Lock( 0, sizeof(g_quadVertices), (BYTE**)&pVertices, 0 );
    memcpy( pVertices, g_quadVertices, sizeof(g_quadVertices) );
    g_pVertexBuffer->Unlock();

	g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
	g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

	D3DXMatrixPerspectiveFovLH( &g_matProj, 45.0f, 640.0f / 480.0f, 0.1f, 100.0f );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &g_matProj );
}


//-----------------------------------------------------------------------------
// Name: initShader()
// Desc: Assemble the shader 
//-----------------------------------------------------------------------------
void initShader( void )
{
	DWORD shaderDeclaration[] =
    {
        D3DVSD_STREAM(0),
        D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT3),  // D3DVSDE_POSITION, Register v0
        D3DVSD_REG(D3DVSDE_DIFFUSE, D3DVSDT_D3DCOLOR), // D3DVSDE_DIFFUSE, Register v5
        D3DVSD_END()
    };

	ID3DXBuffer *pCode = NULL;

    D3DXAssembleShaderFromFile( "dx8_vs11_shader_simple.vsh", 0, NULL, &pCode, NULL );

    g_pd3dDevice->CreateVertexShader( shaderDeclaration, 
		                              (DWORD*)pCode->GetBufferPointer(),
                                      &g_hVertexShader, 0 );

	if( pCode != NULL )
    {
        pCode->Release(); 
        pCode = NULL; 
    }
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: Releases all previously initialized objects
//-----------------------------------------------------------------------------
void shutDown( void )
{
	if( g_hVertexShader != 0xffffffff )
	{
		g_pd3dDevice->DeleteVertexShader( g_hVertexShader );
		g_hVertexShader = 0xffffffff;
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

//-----------------------------------------------------------------------------
// Name: setShaderConstants()
// Desc: 
//-----------------------------------------------------------------------------
void setShaderConstants( void )
{
    D3DXMATRIX matTrans;
	D3DXMATRIX matRot;

    D3DXMatrixTranslation( &matTrans, 0.0f, 0.0f, 3.0f );
	D3DXMatrixRotationYawPitchRoll( &matRot, 
		                            D3DXToRadian(g_fSpinX), 
		                            D3DXToRadian(g_fSpinY), 
		                            0.0f );
    g_matWorld = matRot * matTrans;

    D3DXMatrixIdentity( &g_matView ); // This sample is not really making use of a view matrix

    D3DXMATRIX modelViewProjection = g_matWorld * g_matView * g_matProj;
	D3DXMatrixTranspose( &modelViewProjection, &modelViewProjection );

    // Load the combined model-view-projection matrix in registers c[0]-c[3]
    g_pd3dDevice->SetVertexShaderConstant( 0, &modelViewProjection, 4 );
    
    // Load a constant color into register c[4]
    float fColor[4] = { 0.0f, 1.0f, 0.0f, 0.0f };
    g_pd3dDevice->SetVertexShaderConstant( 4, fColor, 1 );
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: Draws the scene
//-----------------------------------------------------------------------------
void render( void )
{
    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                         D3DCOLOR_COLORVALUE(0.0f,0.0f,0.0f,1.0f), 1.0f, 0 );

    g_pd3dDevice->BeginScene();

	g_pd3dDevice->SetStreamSource( 0, g_pVertexBuffer, sizeof(Vertex) );

    setShaderConstants();

	g_pd3dDevice->SetVertexShader( g_hVertexShader );
    g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLEFAN, 0, 2 );

    g_pd3dDevice->EndScene();
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}
