//-----------------------------------------------------------------------------
//           Name: dx8_cg_simple_vs2ps.cpp
//         Author: Kevin Harris
//  Last Modified: 03/03/04
//    Description: This sample demonstrates how to write both a simple Vertex 
//                 and Pixel Shader with Cg under Direct3D. The two Shaders 
//                 also have matching connectors and can be used simultaneously 
//                 on the same piece of geometry. The sample should be 
//                 considered a starting point or framework for more advanced 
//                 samples.
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <assert.h>
#include <d3d8.h>
#include <d3dx8.h>
#include <Cg/Cg.h>
#include <Cg/CgD3D8.h>
#include "resource.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND                    g_hWnd          = NULL;
LPDIRECT3D8             g_pD3D          = NULL;
LPDIRECT3DDEVICE8       g_pd3dDevice    = NULL;
LPDIRECT3DVERTEXBUFFER8 g_pVertexBuffer = NULL;

CGcontext   g_CGcontext;
CGprogram   g_CGprogram_vertex;
CGprogram   g_CGprogram_pixel;
CGparameter g_CGparam_worldViewProj;
CGparameter g_CGparam_constColor;

D3DXMATRIX g_matWorld;
D3DXMATRIX g_matView;
D3DXMATRIX g_matProj;
float      g_fSpinX = 0.0f;
float      g_fSpinY = 0.0f;

struct Vertex
{
    float x, y, z;
    DWORD color;

	enum FVF
	{
		FVF_Flags = D3DFVF_XYZ | D3DFVF_DIFFUSE 
	};
};

Vertex g_quadVertices[] =
{
	//  x     y     z       color
	{ -1.0f,-1.0f, 0.0f, 0xffffff00, }, // Bottom-Left
	{  1.0f,-1.0f, 0.0f, 0xffff00ff, }, // Bottom-Right
	{  1.0f, 1.0f, 0.0f, 0xff00ffff, }, // Top-Right
	{ -1.0f, 1.0f, 0.0f, 0xff0000ff, }  // Top-Left
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
		                     "Direct3D (DX8) - Simple Vertex & Pixel Shader Using CG",
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
		                              Vertex::FVF_Flags, D3DPOOL_DEFAULT, 
                                      &g_pVertexBuffer );
    void *pVertices = NULL;

    g_pVertexBuffer->Lock( 0, sizeof(g_quadVertices), (BYTE**)&pVertices, 0 );
    memcpy( pVertices, g_quadVertices, sizeof(g_quadVertices) );
    g_pVertexBuffer->Unlock();

	g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
	g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

	D3DXMatrixPerspectiveFovLH( &g_matProj, 45.0f, 640.0f / 480.0f, 0.1f, 100.0f );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &g_matProj );
}

//-----------------------------------------------------------------------------
// Name: initShader()
// Desc: Load the vertex shader 
//-----------------------------------------------------------------------------
void initShader( void )
{
	// Create the context...
	g_CGcontext = cgCreateContext();
	
	//
	// Since we're using Cg's expanded interface, we'll need to pass the 
	// Direct3D device to Cg.
	//

	cgD3D8SetDevice( g_pd3dDevice );

	//
	// Determine the best vertex and pixel profiles to use...
	//
	
	CGprofile vertexProfile = cgD3D8GetLatestVertexProfile();
	CGprofile pixelProfile  = cgD3D8GetLatestPixelProfile();

	//
	// Grab the optimal options for each profile...
	//
	
	const char *vertexOptions[] = 
	{
		cgD3D8GetOptimalOptions(vertexProfile), 0 
	};

	const char *pixelOptions[] = 
	{
		cgD3D8GetOptimalOptions(pixelProfile), 0 
	};

	//
	// Create the vertex and pixel shaders...
	//

	g_CGprogram_vertex = cgCreateProgramFromFile( g_CGcontext,
		                                          CG_SOURCE,
		                                          "vertex_shader.cg",
	                                              vertexProfile,
										          "main",
										          vertexOptions );

	g_CGprogram_pixel = cgCreateProgramFromFile( g_CGcontext,
		                                         CG_SOURCE,
		                                         "pixel_shader.cg",
	                                             pixelProfile,
										         "main",
										         pixelOptions );

	//
	// If your program uses explicit binding semantics (like this one), 
	// you can create a vertex declaration using those semantics.
	//

	DWORD declaration[] = 
	{
		D3DVSD_STREAM(0),
		D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT3),
		D3DVSD_REG(D3DVSDE_DIFFUSE, D3DVSDT_D3DCOLOR),
		D3DVSD_END()
	};
	
	//
	// Ensure the resulting declaration is compatible with the shader...
	//

	assert(cgD3D8ValidateVertexDeclaration( g_CGprogram_vertex, declaration ));

	//
	// Load the programs using Cg's expanded interface...
	//
	// When the second parameter is set to TRUE, shadowing is enabled. This
	// will allow parameters like our g_CGparam_constColor to be set once and
	// reused with out constantly setting it over and over again.
	//

	cgD3D8LoadProgram( g_CGprogram_vertex, TRUE, 0, 0, declaration );
	cgD3D8LoadProgram( g_CGprogram_pixel, TRUE, 0, 0, 0 );

	//
	// Bind some parameters by name so we can set them later...
	//

	g_CGparam_worldViewProj = cgGetNamedParameter( g_CGprogram_vertex, "worldViewProj" );
	g_CGparam_constColor    = cgGetNamedParameter( g_CGprogram_vertex, "constColor" );

	//
	// Make sure our parameters have the expected size...
	//

	assert(cgD3D8TypeToSize(cgGetParameterType(g_CGparam_worldViewProj)) == 16 );
	assert(cgD3D8TypeToSize(cgGetParameterType(g_CGparam_constColor)) == 4 );

	//
	// Set uniform parameters that don't change. They only have to be set
	// once when parameter shadowing is enabled during the call to 
	// cgD3D8LoadProgram.
	//
	
	D3DXVECTOR4 vConstColor( 0.0f, 0.0f, 1.0f, 0.0f );
	cgD3D8SetUniform( g_CGparam_constColor, &vConstColor );
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: Releases all previously initialized objects
//-----------------------------------------------------------------------------
void shutDown( void )
{
	cgD3D8SetDevice(NULL);
	cgDestroyProgram(g_CGprogram_vertex);
	cgDestroyProgram(g_CGprogram_pixel);
	cgDestroyContext(g_CGcontext);

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

    D3DXMATRIX worldViewProj = g_matWorld * g_matView * g_matProj;
	D3DXMatrixTranspose( &worldViewProj, &worldViewProj );

	// Load a combined world-view-projection matrix
	cgD3D8SetUniformMatrix( g_CGparam_worldViewProj, &worldViewProj );

	//
	// Instead of setting it just once, you could load a constant color 
	// every time the shader is invoked, but that would be wasteful if was 
	// truly unchanging between shader usage.
	//

	//D3DXVECTOR4 vConstColor( 0.0f, 1.0f, 0.0f, 0.0f );
	//cgD3D8SetUniform( g_CGparam_constColor, &vConstColor );
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: Draws the scene
//-----------------------------------------------------------------------------
void render( void )
{
    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                         D3DCOLOR_COLORVALUE(0.0f,0.0f,0.0f,1.0f), 1.0f, 0 );

	setShaderConstants();

    g_pd3dDevice->BeginScene();

	cgD3D8BindProgram( g_CGprogram_vertex );
	cgD3D8BindProgram( g_CGprogram_pixel );

    g_pd3dDevice->SetStreamSource( 0, g_pVertexBuffer, sizeof(Vertex) );
    g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLEFAN, 0, 2 );

    g_pd3dDevice->EndScene();
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}

