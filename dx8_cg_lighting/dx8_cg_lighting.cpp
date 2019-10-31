//-----------------------------------------------------------------------------
//           Name: dx8_cg_lighting.cpp
//         Author: Kevin Harris
//  Last Modified: 03/03/04
//    Description: This sample demonstrates how to write a simple CG style
//                 shader that duplicates basic diffuse lighting with Direct3D
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <assert.h>
#include <d3d8.h>
#include <d3dx8.h>
#include <mmsystem.h>
#include <Cg/Cg.h>
#include <Cg/CgD3D8.h>
#include "resource.h"

//#define USE_FIXED_FUNCTION_DIRECT3D_LIGHTING

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND                    g_hWnd          = NULL;
LPDIRECT3D8             g_pD3D          = NULL;
LPDIRECT3DDEVICE8       g_pd3dDevice    = NULL;
LPDIRECT3DVERTEXBUFFER8 g_pVertexBuffer = NULL;

CGcontext   g_CGcontext;
CGprogram   g_CGprogram;
CGparameter g_CGparam_worldViewProj;
CGparameter g_CGparam_worldViewInv;
CGparameter g_CGparam_eyePosition;
CGparameter g_CGparam_lightVector;

D3DXMATRIX g_matWorld;
D3DXMATRIX g_matView;
D3DXMATRIX g_matProj;
float      g_fSpinX = 0.0f;
float      g_fSpinY = 0.0f;

struct Vertex
{
    float x, y, z;
	float nx, ny, nz;
    DWORD rgba;
    float tu, tv;

	enum FVF
	{
		FVF_Flags = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1
	};
};

Vertex g_cubeVertices[] =
{
//     x     y     z      nx    ny    nz                                rgba            tu    tv
    // Front Face
    {-1.0f, 1.0f,-1.0f,  0.0f, 0.0f,-1.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   0.0f,0.0f, },
    { 1.0f, 1.0f,-1.0f,  0.0f, 0.0f,-1.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   1.0f,0.0f, },
    {-1.0f,-1.0f,-1.0f,  0.0f, 0.0f,-1.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   0.0f,1.0f, },
    { 1.0f,-1.0f,-1.0f,  0.0f, 0.0f,-1.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   1.0f,1.0f, },
    // Back Face
    {-1.0f, 1.0f, 1.0f,  0.0f, 0.0f, 1.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   1.0f,0.0f, },
    {-1.0f,-1.0f, 1.0f,  0.0f, 0.0f, 1.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   1.0f,1.0f, },
    { 1.0f, 1.0f, 1.0f,  0.0f, 0.0f, 1.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   0.0f,0.0f, },
    { 1.0f,-1.0f, 1.0f,  0.0f, 0.0f, 1.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   0.0f,1.0f, },
    // Top Face
    {-1.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   0.0f,0.0f, },
    { 1.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   1.0f,0.0f, },
    {-1.0f, 1.0f,-1.0f,  0.0f, 1.0f, 0.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   0.0f,1.0f, },
    { 1.0f, 1.0f,-1.0f,  0.0f, 1.0f, 0.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   1.0f,1.0f, },
    // Bottom Face
    {-1.0f,-1.0f, 1.0f,  0.0f,-1.0f, 0.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   0.0f,0.0f, },
    {-1.0f,-1.0f,-1.0f,  0.0f,-1.0f, 0.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   0.0f,1.0f, },
    { 1.0f,-1.0f, 1.0f,  0.0f,-1.0f, 0.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   1.0f,0.0f, },
    { 1.0f,-1.0f,-1.0f,  0.0f,-1.0f, 0.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   1.0f,1.0f, },
    // Right Face
    { 1.0f, 1.0f,-1.0f,  1.0f, 0.0f, 0.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   0.0f,0.0f, },
    { 1.0f, 1.0f, 1.0f,  1.0f, 0.0f, 0.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   1.0f,0.0f, },
    { 1.0f,-1.0f,-1.0f,  1.0f, 0.0f, 0.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   0.0f,1.0f, },
    { 1.0f,-1.0f, 1.0f,  1.0f, 0.0f, 0.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   1.0f,1.0f, },
    // Left Face
    {-1.0f, 1.0f,-1.0f, -1.0f, 0.0f, 0.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   1.0f,0.0f, },
    {-1.0f,-1.0f,-1.0f, -1.0f, 0.0f, 0.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   1.0f,1.0f, },
    {-1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   0.0f,0.0f, },
    {-1.0f,-1.0f, 1.0f, -1.0f, 0.0f, 0.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   0.0f,1.0f, }
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
                             "Direct3D (DX8) - Lighting Vertex Shader Using CG",
						     WS_OVERLAPPEDWINDOW | WS_VISIBLE,
					         0, 0, 640, 480, NULL, NULL, hInstance, NULL );

	if( g_hWnd == NULL )
		return E_FAIL;

    ShowWindow( g_hWnd, nCmdShow );
    UpdateWindow( g_hWnd );

	init();

#ifndef USE_FIXED_FUNCTION_DIRECT3D_LIGHTING
	initShader();
#endif

	while( uMsg.message != WM_QUIT )
	{
		if( PeekMessage( &uMsg, NULL, 0, 0, PM_REMOVE ) )
		{
			TranslateMessage( &uMsg );
			DispatchMessage( &uMsg );
		}
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
	
	g_pd3dDevice->CreateVertexBuffer( 24*sizeof(Vertex), D3DUSAGE_WRITEONLY, 
		                              Vertex::FVF_Flags, D3DPOOL_DEFAULT, 
                                      &g_pVertexBuffer );
 
    void *pVertices = NULL;

    g_pVertexBuffer->Lock( 0, sizeof(g_cubeVertices), (BYTE**)&pVertices, 0 );
    memcpy( pVertices, g_cubeVertices, sizeof(g_cubeVertices) );
    g_pVertexBuffer->Unlock();

	D3DXMatrixPerspectiveFovLH( &g_matProj, 45.0f, 640.0f / 480.0f, 0.1f, 100.0f );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &g_matProj );

#ifdef USE_FIXED_FUNCTION_DIRECT3D_LIGHTING

	g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );

    // Set up a material
    D3DMATERIAL8 mtrl;
	ZeroMemory( &mtrl, sizeof(D3DMATERIAL8) );
    mtrl.Diffuse.r = 1.0f;
    mtrl.Diffuse.g = 1.0f;
    mtrl.Diffuse.b = 1.0f;
    mtrl.Diffuse.a = 1.0f;
    mtrl.Ambient.r = 1.0f;
    mtrl.Ambient.g = 1.0f;
    mtrl.Ambient.b = 1.0f;
    mtrl.Ambient.a = 1.0f;
    g_pd3dDevice->SetMaterial( &mtrl );

    // Set light 0 to be a pure white directional light
    D3DLIGHT8 light0;
    ZeroMemory( &light0, sizeof(D3DLIGHT8) );
    light0.Type = D3DLIGHT_DIRECTIONAL;
    light0.Direction = D3DXVECTOR3( 0.0f, 0.0f, 1.0f );
    light0.Diffuse.r = 1.0f;
    light0.Diffuse.g = 1.0f;
    light0.Diffuse.b = 1.0f;
	light0.Diffuse.a = 1.0f;
    g_pd3dDevice->SetLight( 0, &light0 );

	// Enable some dim, grey ambient lighting so objects that are not lit 
    // by the other lights are not completely black.
    g_pd3dDevice->SetRenderState( D3DRS_AMBIENT, D3DCOLOR_COLORVALUE( 0.2f, 0.2f, 0.2f, 0.2f ) );

    g_pd3dDevice->LightEnable( 0, TRUE );

#endif
}

//-----------------------------------------------------------------------------
// Name: initShader()
// Desc: Load the vertex shader 
//-----------------------------------------------------------------------------
void initShader(void)
{
	// Create the context...
	g_CGcontext = cgCreateContext();
	
	//
	// Since we're using Cg's expanded interface, we'll need to pass the 
	// Direct3D device to Cg.
	//

	cgD3D8SetDevice( g_pd3dDevice );

	// Determine the best vertex profile to use...
	CGprofile vertexProfile = cgD3D8GetLatestVertexProfile();

	// Grab the optimal options for each profile...
	const char* vertexOptions[] = 
	{
		cgD3D8GetOptimalOptions( vertexProfile), 0 
	};

	//
	// Create the vertex shader...
	//

	g_CGprogram = cgCreateProgramFromFile( g_CGcontext,
		                                   CG_SOURCE,
		                                   "dx8_cg_lighting.cg",
	                                       vertexProfile,
										   "main",
										   vertexOptions );

	//
	// If your program uses explicit binding semantics (like this one), 
	// you can create a vertex declaration using those semantics.
	//

	DWORD declaration[] = 
	{
		D3DVSD_STREAM(0),
		D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT3),
		D3DVSD_REG(D3DVSDE_NORMAL, D3DVSDT_FLOAT3),
		D3DVSD_REG(D3DVSDE_DIFFUSE, D3DVSDT_D3DCOLOR),
		D3DVSD_END()
	};
	
	//
	// Ensure the resulting declaration is compatible with the shader...
	//

	assert(cgD3D8ValidateVertexDeclaration( g_CGprogram, declaration ));

	//
	// Load the program using Cg's expanded interface...
	//
	// When the second parameter is set to TRUE, shadowing is enabled. This
	// will allow parameters like our g_CGparam_constColor to be set once and
	// reused with out constantly setting it over and over again.
	//

	cgD3D8LoadProgram( g_CGprogram, TRUE, 0, 0, declaration );

	//
	// Bind some parameters by name so we can set them later...
	//

	g_CGparam_worldViewProj = cgGetNamedParameter( g_CGprogram, "worldViewProj" );
	g_CGparam_worldViewInv  = cgGetNamedParameter( g_CGprogram, "worldViewInv" );
	g_CGparam_eyePosition   = cgGetNamedParameter( g_CGprogram, "eyePosition" );
	g_CGparam_lightVector   = cgGetNamedParameter( g_CGprogram, "lightVector" );

	//
	// Make sure our parameters have the expected size...
	//

	assert(cgD3D8TypeToSize(cgGetParameterType(g_CGparam_worldViewProj)) == 16 );
	assert(cgD3D8TypeToSize(cgGetParameterType(g_CGparam_worldViewInv)) == 16 );
	assert(cgD3D8TypeToSize(cgGetParameterType(g_CGparam_eyePosition)) == 4 );
	assert(cgD3D8TypeToSize(cgGetParameterType(g_CGparam_lightVector)) == 4 );
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: Releases all previously initialized objects
//-----------------------------------------------------------------------------
void shutDown( void )
{
	cgD3D8SetDevice(NULL);
	cgDestroyProgram(g_CGprogram);
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

    D3DXMatrixTranslation( &matTrans, 0.0f, 0.0f, 4.0f );
	D3DXMatrixRotationYawPitchRoll( &matRot, 
		                            D3DXToRadian(g_fSpinX), 
		                            D3DXToRadian(g_fSpinY), 
		                            0.0f );
    g_matWorld = matRot * matTrans;

    D3DXMatrixIdentity( &g_matView ); // This sample is not really making use of a view matrix

    D3DXMATRIX worldViewProj = g_matWorld * g_matView * g_matProj;
	D3DXMatrixTranspose( &worldViewProj, &worldViewProj );

	D3DXMATRIX worldViewInv = g_matWorld * g_matView;
	D3DXMatrixInverse( &worldViewInv, NULL, &worldViewInv );

	D3DXVECTOR3 vEyePosition( 0.0f, 0.0f, 0.0f );
	D3DXVECTOR3 vLightVector( 0.0f, 0.0f, -1.0f );
	D3DXVec3Normalize( &vLightVector, &vLightVector );
	
	cgD3D8SetUniformMatrix( g_CGparam_worldViewProj, &worldViewProj );
	cgD3D8SetUniformMatrix( g_CGparam_worldViewInv, &worldViewInv );
	cgD3D8SetUniform( g_CGparam_eyePosition, &vEyePosition );
	cgD3D8SetUniform( g_CGparam_lightVector, &vLightVector );
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

#ifdef USE_FIXED_FUNCTION_DIRECT3D_LIGHTING

	D3DXMATRIX matTrans;
	D3DXMATRIX matRot;

    D3DXMatrixTranslation( &matTrans, 0.0f, 0.0f, 4.0f );

	D3DXMatrixRotationYawPitchRoll( &matRot, 
		                            D3DXToRadian(g_fSpinX), 
		                            D3DXToRadian(g_fSpinY), 
		                            0.0f );
    g_matWorld = matRot * matTrans;

	g_pd3dDevice->SetTransform( D3DTS_WORLD, &g_matWorld );
	g_pd3dDevice->SetVertexShader( Vertex::FVF_Flags );
    g_pd3dDevice->SetStreamSource( 0, g_pVertexBuffer, sizeof(Vertex) );

#else // Use the shader...

	setShaderConstants();

	cgD3D8BindProgram( g_CGprogram );
	
    g_pd3dDevice->SetStreamSource( 0, g_pVertexBuffer, sizeof(Vertex) );

#endif

    g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,  0, 2 );
	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,  4, 2 );
	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,  8, 2 );
	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 12, 2 );
	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 16, 2 );
	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 20, 2 );

    g_pd3dDevice->EndScene();
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}
