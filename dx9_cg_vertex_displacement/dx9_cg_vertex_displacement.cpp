//-----------------------------------------------------------------------------
//           Name: dx9_cg_vertex_displacement.cpp
//         Author: Kevin Harris
//  Last Modified: 10/13/06
//    Description: This sample demonstrates how to perform mesh deformation or 
//                 vertex displacement with Direct3D using a Cg shader.
//
//   Control Keys: F1 - Increase flag motion
//                 F2 - Decrease flag motion
//                 F3 - Toggle wire-frame mode
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <mmsystem.h>
#include <assert.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <Cg/Cg.h>
#include <Cg/CgD3D9.h>
#include "resource.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND                    g_hWnd              = NULL;
LPDIRECT3D9             g_pD3D              = NULL;
LPDIRECT3DDEVICE9       g_pd3dDevice        = NULL;
LPDIRECT3DTEXTURE9      g_pTexture          = NULL;
LPDIRECT3DVERTEXBUFFER9 g_pMeshVertexBuffer = NULL;

LPDIRECT3DVERTEXDECLARATION9 g_pVertexDeclaration = NULL;

CGprofile   g_CGprofile;
CGcontext   g_CGcontext;
CGprogram   g_CGprogram;
CGparameter g_CGparam_worldViewProj;
CGparameter g_CGparam_currentAngle;

float  g_fElpasedTime;
double g_dCurTime;
double g_dLastTime;
float  g_fSpinX = 0.0f;
float  g_fSpinY = 0.0f;

float g_fCurrentAngle    = 0.0f;
float g_fSpeedOfRotation = 10.0f;
bool  g_bWireFrameMode   = false;

D3DXMATRIX g_matWorld;
D3DXMATRIX g_matView;
D3DXMATRIX g_matProj;

const int g_nNumVertsX = 16; // Number of flag vertices along x axis
const int g_nNumVertsZ = 16; // Number of flag vertices along z axis

// Number of triangles needed for the flag
const int g_nTriCount  = (g_nNumVertsX-1)*(g_nNumVertsZ-1)*2;
const int g_nVertCount = g_nTriCount*3;

struct Vertex
{
	D3DXVECTOR3 position;
    D3DXVECTOR3 normal;
	D3DXVECTOR2 texcoords;

	enum FVF
	{
		FVF_Flags = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1
	};
};

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void loadTexture(void);
void createMesh(void);
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
		                     "Direct3D (DX9) - Vertex Displacement Shader Using CG",
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
	static POINT ptLastMousePosit;
	static POINT ptCurrentMousePosit;
	static bool  bMousing;
	
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
					g_fSpeedOfRotation += 0.5f;
					break;

                case VK_F2:
					g_fSpeedOfRotation -= 0.5f;
					break;

                case VK_F3:
					g_bWireFrameMode = !g_bWireFrameMode;
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
void loadTexture( void )
{
    D3DXCreateTextureFromFile( g_pd3dDevice, "us_flag.bmp", &g_pTexture );

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

	g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	
	D3DXMatrixPerspectiveFovLH( &g_matProj, D3DXToRadian( 45.0f ), 
                                640.0f / 480.0f, 0.1f, 100.0f );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &g_matProj );

	loadTexture();

    createMesh();
}

//-----------------------------------------------------------------------------
// Name: createMesh()
// Desc: 
//-----------------------------------------------------------------------------
void createMesh( void )
{
    //
    // Create a square tesselated mesh, which measures g_nNumVertsX * g_nNumVertsZ.
    //

	float dX =  (1.0f/(g_nNumVertsX-1));
	float dZ = -(1.0f/(g_nNumVertsZ-1));

    float dTU = 1.0f/(g_nNumVertsX-1);
	float dTV = 1.0f/(g_nNumVertsZ-1);

    int i = 0;
    int x = 0;
    int z = 0;
    float fSizeFactorX = 8.0f;
    float fSizeFactorZ = 5.0f;

	Vertex *v = NULL;

    g_pd3dDevice->CreateVertexBuffer( g_nTriCount*3*sizeof(Vertex),
		                              D3DUSAGE_WRITEONLY, Vertex::FVF_Flags,
                                      D3DPOOL_MANAGED, &g_pMeshVertexBuffer, NULL );

    g_pMeshVertexBuffer->Lock( 0, 0, (void**)&v, 0 );
	
    // The normal is the same for every vertex.
    for( i = 0; i < g_nVertCount; ++i )
		v[i].normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);

    for( z = 0, i = 0; z < (g_nNumVertsZ-1); ++z )
    {
        for( x = 0; x < (g_nNumVertsX-1); ++x )
        {
			// First Triangle...

            v[i].position = D3DXVECTOR3(fSizeFactorX*x*dX, 0.0f, fSizeFactorZ*z*dZ );
			v[i].texcoords = D3DXVECTOR2(x * dTU, z * dTV);
            ++i;

            v[i].position = D3DXVECTOR3(fSizeFactorX*x*dX, 0.0f, fSizeFactorZ*(z+1)*dZ );
			v[i].texcoords = D3DXVECTOR2(x * dTU, (z+1.0f) * dTV);
            ++i;

            v[i].position = D3DXVECTOR3(fSizeFactorX*(x+1)*dX, 0.0f, fSizeFactorZ*(z+1)*dZ );
			v[i].texcoords = D3DXVECTOR2((x+1.0f) * dTU, (z+1.0f) * dTV);
            ++i;

			// Second Triangle...

            v[i].position = D3DXVECTOR3(fSizeFactorX*x*dX, 0.0f, fSizeFactorZ*z*dZ );
			v[i].texcoords = D3DXVECTOR2(x * dTU, z * dTU);
            ++i;

            v[i].position = D3DXVECTOR3(fSizeFactorX*(x+1)*dX, 0.0f, fSizeFactorZ*(z+1)*dZ );
			v[i].texcoords = D3DXVECTOR2((x+1.0f) * dTU, (z+1.0f) * dTV);
            ++i;

            v[i].position = D3DXVECTOR3(fSizeFactorX*(x+1)*dX, 0.0f, fSizeFactorZ*z*dZ );
			v[i].texcoords = D3DXVECTOR2((x+1.0f) * dTU, z * dTV);
            ++i;
        }
    }
	
    g_pMeshVertexBuffer->Unlock();
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

	cgD3D9SetDevice( g_pd3dDevice );

	// Determine the best vertex profile to use...
	CGprofile vertexProfile = cgD3D9GetLatestVertexProfile();

	// Grab the optimal options for each profile...
	const char **vertexOptions[] =
	{
		cgD3D9GetOptimalOptions( vertexProfile ),
		NULL,
	};

	//
	// Create the vertex shader...
	//

	g_CGprogram = cgCreateProgramFromFile( g_CGcontext,
		                                   CG_SOURCE,
		                                   "dx9_cg_vertex_displacement.cg",
	                                       vertexProfile,
										   "main",
										   *vertexOptions );

	//
	// If your program uses explicit binding semantics (like this one), 
	// you can create a vertex declaration using those semantics.
	//

    const D3DVERTEXELEMENT9 declaration[] =
    {
        { 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },
        { 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
        D3DDECL_END()
    };

	//
	// Ensure the resulting declaration is compatible with the shader...
	//

	cgD3D9ValidateVertexDeclaration( g_CGprogram, declaration );

    g_pd3dDevice->CreateVertexDeclaration( declaration, &g_pVertexDeclaration );

	//
	// Load the program using Cg's expanded interface...
	//
	// When the second parameter is set to TRUE, shadowing is enabled. This
	// will allow parameters like our g_CGparam_constColor to be set once and
	// reused with out constantly setting it over and over again.
	//

	cgD3D9LoadProgram( g_CGprogram, TRUE, 0 );

	//
	// Bind some parameters by name so we can set them later...
	//

	g_CGparam_worldViewProj = cgGetNamedParameter( g_CGprogram, "worldViewProj" );
	g_CGparam_currentAngle  = cgGetNamedParameter( g_CGprogram, "currentAngle" );

	//
	// Make sure our parameters have the expected size...
	//

	assert(cgD3D9TypeToSize(cgGetParameterType(g_CGparam_worldViewProj)) == 16 );
	assert(cgD3D9TypeToSize(cgGetParameterType(g_CGparam_currentAngle)) == 4 );
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: Releases all previously initialized objects
//-----------------------------------------------------------------------------
void shutDown( void )
{
	cgD3D9SetDevice(NULL);
	cgDestroyProgram(g_CGprogram);
	cgDestroyContext(g_CGcontext);
	
	if( g_pTexture != NULL ) 
        g_pTexture->Release();

    if( g_pMeshVertexBuffer != NULL )
        g_pMeshVertexBuffer->Release();

    if( g_pd3dDevice != NULL )
        g_pd3dDevice->Release();

    if( g_pD3D != NULL )
        g_pD3D->Release();
}

//-----------------------------------------------------------------------------
// Name: setShaderConstants()
// Desc: 
//-----------------------------------------------------------------------------
void setShaderConstants( void )
{
	g_fCurrentAngle -= g_fSpeedOfRotation * g_fElpasedTime;

    while( g_fCurrentAngle > 360.0f ) g_fCurrentAngle -= 360.0f;
    while( g_fCurrentAngle < 0.0f   ) g_fCurrentAngle += 360.0f;

    D3DXMATRIX matTrans;
	D3DXMATRIX matRot;

    D3DXMatrixTranslation( &matTrans, -4.0f, 2.5f, 10.0f );
	D3DXMatrixRotationYawPitchRoll( &matRot,
		                            D3DXToRadian(g_fSpinX), 
		                            D3DXToRadian(g_fSpinY - 90.0f), 
		                            0.0f );
    g_matWorld = matRot * matTrans;

    D3DXMatrixIdentity( &g_matView ); // This sample is not really making use of a view matrix

    D3DXMATRIX worldViewProj = g_matWorld * g_matView * g_matProj;
	D3DXMatrixTranspose( &worldViewProj, &worldViewProj );

	D3DXVECTOR3 vfCurrentAngle( g_fCurrentAngle, 0.0f, 0.0f );
	
	cgD3D9SetUniformMatrix( g_CGparam_worldViewProj, &worldViewProj );
	cgD3D9SetUniform( g_CGparam_currentAngle, &vfCurrentAngle );
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

    if( g_bWireFrameMode )
        g_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
    else
        g_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );

    g_pd3dDevice->SetTexture( 0, g_pTexture );
	g_pd3dDevice->SetStreamSource(0, g_pMeshVertexBuffer, 0, sizeof( Vertex ) );

	setShaderConstants();

	g_pd3dDevice->SetVertexDeclaration( g_pVertexDeclaration );
    cgD3D9BindProgram( g_CGprogram );

    g_pd3dDevice->SetStreamSource( 0, g_pMeshVertexBuffer, 0, sizeof(Vertex) );
	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, g_nTriCount );

    g_pd3dDevice->EndScene();
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}
