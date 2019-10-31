//-----------------------------------------------------------------------------
//           Name: dx9_cg_anisotropic_lighting.cpp
//         Author: Kevin Harris
//  Last Modified: 10/13/06
//    Description: Demonstrates how to write a vertex shader using Cg, which 
//                 calculates Anisotropic Lighting for a single light source. 
//                 The shader uses a texture as a look-up table for the correct 
//                 Anisotropic Lighting values by storing the pre-calculated 
//                 diffuse values in the texture's RGB components and specular 
//                 values in its alpha component. This style of lighting is 
//                 very useful for rendering surfaces like brushed steel where 
//                 the surface is composed of micro facets or microscopic 
//                 scratches that tend to lay parallel or run in the same 
//                 direction.
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
HWND                    g_hWnd                = NULL;
LPDIRECT3D9             g_pD3D                = NULL;
LPDIRECT3DDEVICE9       g_pd3dDevice          = NULL;
LPDIRECT3DTEXTURE9      g_pAnisoLookUpTexture = NULL;
LPDIRECT3DVERTEXBUFFER9 g_pTeapotVertexBuffer = NULL;
LPDIRECT3DINDEXBUFFER9  g_pTeapotIndexBuffer  = NULL;
D3DXATTRIBUTERANGE     *g_pAttributes         = NULL;
unsigned long           g_dwNumSections       = NULL;

LPDIRECT3DVERTEXDECLARATION9 g_pVertexDeclaration = NULL;

CGprofile   g_CGprofile;
CGcontext   g_CGcontext;
CGprogram   g_CGprogram;
CGparameter g_CGparam_worldViewProj;
CGparameter g_CGparam_worldViewInv;
CGparameter g_CGparam_world;
CGparameter g_CGparam_lightVector;
CGparameter g_CGparam_eyePosition;

D3DXMATRIX g_matWorld;
D3DXMATRIX g_matView;
D3DXMATRIX g_matProj;
float      g_fSpinX = 0.0f;
float      g_fSpinY = 0.0f;

struct Vertex
{
	float x, y, z;
	float nx, ny, nz;

	enum FVF
	{
		FVF_Flags = D3DFVF_XYZ | D3DFVF_NORMAL
	};
};

//-----------------------------------------------------------------------------
// MACROS
//-----------------------------------------------------------------------------
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(a) if ((a)) {(a)->Release(); (a) = NULL;}
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(a) if ((a)) {delete(a); a = NULL;}
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(a) if ((a)) {delete [] (a); a = NULL;}
#endif

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void loadXFile(const char* fileName, const DWORD dwFVF);
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
                             "Direct3D (DX9) - Anisotropic Lighting Shader Using CG",
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
			ptLastMousePosit.x = ptCurrentMousePosit.x = LOWORD(lParam);
            ptLastMousePosit.y = ptCurrentMousePosit.y = HIWORD(lParam);
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
			ptCurrentMousePosit.x = LOWORD(lParam);
			ptCurrentMousePosit.y = HIWORD(lParam);

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
// Name: loadXFile()
// Desc: Loads an X file and then converts it over to our custom FVF format
//-----------------------------------------------------------------------------
void loadXFile( const char* fileName, const DWORD dwFVF )
{
	ID3DXMesh *tempMeshRaw = NULL; // Raw mesh data, straight from the file
    ID3DXMesh *tempMeshOpt = NULL; // Optimized mesh data
    ID3DXMesh *tempMeshFVF = NULL; // Mesh converted to our custom FVF format
		
	D3DXLoadMeshFromX( const_cast<char*>(fileName), D3DXMESH_SYSTEMMEM, 
                       g_pd3dDevice, NULL, NULL, NULL, &g_dwNumSections, &tempMeshRaw );

	tempMeshRaw->Optimize( D3DXMESHOPT_ATTRSORT, NULL, NULL, NULL, NULL, &tempMeshOpt );

	tempMeshOpt->GetAttributeTable( NULL, &g_dwNumSections );

    SAFE_DELETE_ARRAY( g_pAttributes );

	g_pAttributes = new D3DXATTRIBUTERANGE[g_dwNumSections];
	
	tempMeshOpt->GetAttributeTable( g_pAttributes, &g_dwNumSections );

    SAFE_RELEASE( g_pTeapotVertexBuffer );
    SAFE_RELEASE( g_pTeapotIndexBuffer );

	// Convert to our custom format...
	tempMeshOpt->CloneMeshFVF( D3DXMESH_WRITEONLY, dwFVF, g_pd3dDevice, &tempMeshFVF );
	
	tempMeshFVF->GetVertexBuffer( &g_pTeapotVertexBuffer );
	tempMeshFVF->GetIndexBuffer( &g_pTeapotIndexBuffer );

    SAFE_RELEASE( tempMeshRaw );
    SAFE_RELEASE( tempMeshFVF );
    SAFE_RELEASE( tempMeshOpt );
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
    
	D3DXMatrixPerspectiveFovLH( &g_matProj, D3DXToRadian( 45.0f ), 
                                640.0f / 480.0f, 0.1f, 100.0f );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &g_matProj );

    //
    // Set up some general rendering states...
    //

    g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );
    g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );

    //
    // This is the texture used by the shader for looking up the anisotropic 
    // lighting values...
    //

    D3DXCreateTextureFromFile( g_pd3dDevice, "dx_aniso.tga", &g_pAnisoLookUpTexture );

    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU,  D3DTADDRESS_MIRROR );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );

    //
    // It's often needed to modulate or increase the diffuse and specular 
    // terms to a small power (between 2 or 4) to account for the fact that  
    // the most-significant normal does not account for the entire lighting 
    // of the anisotropic surface.
    //

    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE4X ); // Modulate 4 times for brightness...
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );     // the texture for this stage with...
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE | D3DTA_ALPHAREPLICATE ); // the same texture, and don't forget the alpha
    
    //
    // This will load up our teapot mesh and convert the FVF format to our 
    // custom FVF format which the shader is expecting...
    //

    loadXFile( "teapot.x", Vertex::FVF_Flags );
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
		                                   "dx9_cg_anisotropic_lighting.cg",
	                                       vertexProfile,
										   "main",
										   *vertexOptions );

	//
	// If your program uses explicit binding semantics (like this one), 
	// you can create a vertex declaration using those semantics.
	//

	const D3DVERTEXELEMENT9 declaration[] = 
	{
		{ 0, 0 * sizeof(float),D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,D3DDECLUSAGE_POSITION, 0 },
		{ 0, 3 * sizeof(float),D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,D3DDECLUSAGE_NORMAL,   0 },
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
	g_CGparam_worldViewInv  = cgGetNamedParameter( g_CGprogram, "worldViewInv" );
	g_CGparam_world         = cgGetNamedParameter( g_CGprogram, "world" );
	g_CGparam_lightVector   = cgGetNamedParameter( g_CGprogram, "lightVector" );
	g_CGparam_eyePosition   = cgGetNamedParameter( g_CGprogram, "eyePosition" );

	//
	// Make sure our parameters have the expected size...
	//

	assert(cgD3D9TypeToSize(cgGetParameterType(g_CGparam_worldViewProj)) == 16 );
	assert(cgD3D9TypeToSize(cgGetParameterType(g_CGparam_worldViewInv)) == 9 );
	assert(cgD3D9TypeToSize(cgGetParameterType(g_CGparam_world)) == 12 );
	assert(cgD3D9TypeToSize(cgGetParameterType(g_CGparam_lightVector)) == 3 );
	assert(cgD3D9TypeToSize(cgGetParameterType(g_CGparam_eyePosition)) == 3 );
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
	
    SAFE_RELEASE( g_pAnisoLookUpTexture );
    SAFE_RELEASE( g_pTeapotVertexBuffer );
    SAFE_RELEASE( g_pTeapotIndexBuffer );
    SAFE_DELETE_ARRAY( g_pAttributes );
    SAFE_RELEASE( g_pd3dDevice );
    SAFE_RELEASE( g_pD3D );
}

//-----------------------------------------------------------------------------
// Name: setShaderConstants()
// Desc: 
//-----------------------------------------------------------------------------
void setShaderConstants( void )
{
    //
    // We'll start off by bulding a world matrix for our teapot, which will
    // let us spin it and translate it away...
    //

    D3DXMATRIX matTrans;
	D3DXMATRIX matRot;

    D3DXMatrixTranslation( &matTrans, 0.0f, 0.0f, 4.0f );
	D3DXMatrixRotationYawPitchRoll( &matRot, 
		                            D3DXToRadian(g_fSpinX), 
		                            D3DXToRadian(g_fSpinY), 
		                            0.0f );
    g_matWorld = matRot * matTrans;

    D3DXMatrixIdentity( &g_matView ); // This sample is not really making use of a view matrix

	// This matrix will be used to transform the vertices from world-space to clip-space
    D3DXMATRIX worldViewProjection = g_matWorld * g_matView * g_matProj;
	D3DXMatrixTranspose( &worldViewProjection, &worldViewProjection );

	// This matrix will be used to transform the normals from model-space to view-space
	D3DXMATRIX worldViewInverse = g_matWorld * g_matView;
	D3DXMatrixInverse( &worldViewInverse, NULL, &worldViewInverse );
    
    // This matrix will be used to compute simple world-space positions
	D3DXMATRIX world = g_matWorld;
    D3DXMatrixTranspose( &world, &world );

    D3DXVECTOR4 vLightVector( 1.0f, 0.0f, -1.0f, 0.0f );
	D3DXVec4Normalize( &vLightVector, &vLightVector );

    D3DXVECTOR3 vEyePosition( 0.0f, 0.0f, 0.0f );

	cgD3D9SetUniformMatrix( g_CGparam_worldViewProj, &worldViewProjection );
	cgD3D9SetUniformMatrix( g_CGparam_worldViewInv, &worldViewInverse );
	cgD3D9SetUniformMatrix( g_CGparam_world, &world );
	cgD3D9SetUniform( g_CGparam_lightVector, &vLightVector );
	cgD3D9SetUniform( g_CGparam_eyePosition, &vEyePosition );
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: 
//-----------------------------------------------------------------------------
void render( void )
{
    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                         D3DCOLOR_COLORVALUE(0.0f,0.0f,0.0f,1.0f), 1.0f, 0 );

    g_pd3dDevice->BeginScene();

    g_pd3dDevice->SetTexture( 0, g_pAnisoLookUpTexture );
	g_pd3dDevice->SetIndices( g_pTeapotIndexBuffer );
	g_pd3dDevice->SetStreamSource(0, g_pTeapotVertexBuffer, 0, sizeof( Vertex ) );

	setShaderConstants();

	g_pd3dDevice->SetVertexDeclaration( g_pVertexDeclaration );
    cgD3D9BindProgram( g_CGprogram );

    //
    // Render teapot mesh...
    //

	for( unsigned long i = 0; i < g_dwNumSections; ++i )
	{
		g_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0,
										    g_pAttributes[i].VertexStart,
										    g_pAttributes[i].VertexCount,
							  			    g_pAttributes[i].FaceStart * 3,
										    g_pAttributes[i].FaceCount );
	}

    g_pd3dDevice->EndScene();
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}

