//-----------------------------------------------------------------------------
//           Name: dx9_cg_simple_vs2ps.cpp
//         Author: Kevin Harris
//  Last Modified: 10/13/06
//    Description: This sample demonstrates how to write both a simple Vertex 
//                 and Pixel Shader with Cg under Direct3D. The two Shaders 
//                 also have matching connectors and can be used simultaneously 
//                 on the same piece of geometry. The sample should be 
//                 considered a starting point or framework for more advanced 
//                 samples.
//
//   Control Keys: F1 - Toggle usage of vertex and pixel shaders.
//
// Note: The pixel shader has been changed slightly from what the 
//       fixed-function pipeline does by default so you can see a noticeable 
//       change when toggling the shaders on and off. Instead of modulating 
//       the vertex color with the texture's texel, the pixel shader adds the 
//       two together, which causes the pixel shader to produce a brighter, 
//       washed-out image. This modification can be switched back in the pixel 
//       shader file.
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <assert.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <Cg/Cg.h>
#include <Cg/CgD3D9.h>
#include "resource.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND                    g_hWnd          = NULL;
LPDIRECT3D9             g_pD3D          = NULL;
LPDIRECT3DDEVICE9       g_pd3dDevice    = NULL;
LPDIRECT3DVERTEXBUFFER9 g_pVertexBuffer = NULL;
LPDIRECT3DTEXTURE9      g_pTexture      = NULL;

LPDIRECT3DVERTEXDECLARATION9 g_pVertexDeclaration = NULL;

CGcontext   g_CGcontext;
CGprogram   g_CGprogram_vertex;
CGprogram   g_CGprogram_pixel;
CGparameter g_CGparam_worldViewProj;
CGparameter g_CGparam_testTexture;

bool g_bUseShaders = true;

D3DXMATRIX g_matWorld;
D3DXMATRIX g_matView;
D3DXMATRIX g_matProj;
float      g_fSpinX = 0.0f;
float      g_fSpinY = 0.0f;

struct Vertex
{
    float x, y, z;
    DWORD color;
    float tu, tv;

    enum FVF
    {
        FVF_Flags = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1
    };
};

Vertex g_quadVertices[] =
{
    {-1.0f, 1.0f, 0.0f,  0xffffff00,  0.0f,0.0f },
    { 1.0f, 1.0f, 0.0f,  0xff00ff00,  1.0f,0.0f },
    {-1.0f,-1.0f, 0.0f,  0xffff0000,  0.0f,1.0f },
    { 1.0f,-1.0f, 0.0f,  0xff0000ff,  1.0f,1.0f }
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
int WINAPI WinMain( HINSTANCE hInstance,
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
    winClass.hIcon         = LoadIcon(hInstance, (LPCTSTR)IDI_DIRECTX_ICON);
    winClass.hIconSm       = LoadIcon(hInstance, (LPCTSTR)IDI_DIRECTX_ICON);
    winClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    winClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    winClass.lpszMenuName  = NULL;
    winClass.cbClsExtra    = 0;
    winClass.cbWndExtra    = 0;

    if( !RegisterClassEx(&winClass) )
        return E_FAIL;

    g_hWnd = CreateWindowEx( NULL, "MY_WINDOWS_CLASS", 
                             "Direct3D (DX9) - Simple Vertex & Pixel Shader Using CG",
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

                case VK_F1:
                    g_bUseShaders = !g_bUseShaders;
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
    
    g_pd3dDevice->CreateVertexBuffer( 4*sizeof(Vertex), D3DUSAGE_WRITEONLY, 
                                      Vertex::FVF_Flags, D3DPOOL_DEFAULT, 
                                      &g_pVertexBuffer, NULL );
    void *pVertices = NULL;

    g_pVertexBuffer->Lock( 0, sizeof(g_quadVertices), (void**)&pVertices, 0 );
    memcpy( pVertices, g_quadVertices, sizeof(g_quadVertices) );
    g_pVertexBuffer->Unlock();

    g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

    D3DXMatrixPerspectiveFovLH( &g_matProj, D3DXToRadian( 45.0f ), 
                                640.0f / 480.0f, 0.1f, 100.0f );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &g_matProj );

    //
    // Create a texture to test out our pixel shader...
    //

    D3DXCreateTextureFromFile( g_pd3dDevice, "test.bmp", &g_pTexture );
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

    //
    // Determine the best vertex and pixel profiles to use...
    //
    
    CGprofile vertexProfile = cgD3D9GetLatestVertexProfile();
    CGprofile pixelProfile  = cgD3D9GetLatestPixelProfile();

    //
    // Grab the optimal options for each profile...
    //

	// Grab the optimal options for each profile...
	const char **vertexOptions[] =
	{
		cgD3D9GetOptimalOptions( vertexProfile ),
		NULL,
	};
	const char **pixelOptions[] =
	{
		cgD3D9GetOptimalOptions( pixelProfile ),
		NULL,
	};

    //
    // Create the vertex and pixel shaders...
    //

    g_CGprogram_vertex = cgCreateProgramFromFile( g_CGcontext,
                                                  CG_SOURCE,
                                                  "vertex_shader.cg",
                                                  vertexProfile,
                                                  "main",
                                                  *vertexOptions );

    g_CGprogram_pixel = cgCreateProgramFromFile( g_CGcontext,
                                                 CG_SOURCE,
                                                 "pixel_shader.cg",
                                                 pixelProfile,
                                                 "main",
                                                 *pixelOptions );

    //
    // If your program uses explicit binding semantics (like this one), 
    // you can create a vertex declaration using those semantics.
    //

    D3DVERTEXELEMENT9 declaration[] =
    {
        { 0, 0,  D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,    0 },
        { 0, 16, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
        D3DDECL_END()
    };

    //
    // Ensure the resulting declaration is compatible with the shader...
    //

    cgD3D9ValidateVertexDeclaration( g_CGprogram_vertex, declaration );

    g_pd3dDevice->CreateVertexDeclaration( declaration, &g_pVertexDeclaration );
    
    //
    // Load the programs using Cg's expanded interface...
    //
    // When the second parameter is set to TRUE, shadowing is enabled. This
    // will allow parameters like our g_CGparam_constColor to be set once and
    // reused with out constantly setting it over and over again.
    //

    cgD3D9LoadProgram( g_CGprogram_vertex, TRUE, 0 );
    cgD3D9LoadProgram( g_CGprogram_pixel, TRUE, 0 );

    //
    // Bind some parameters by name so we can set them later...
    //

    g_CGparam_worldViewProj = cgGetNamedParameter( g_CGprogram_vertex, "worldViewProj" );
    g_CGparam_testTexture   = cgGetNamedParameter( g_CGprogram_pixel, "testTexture" );

    //
    // Make sure our parameters have the expected size...
    //

    int s = cgD3D9TypeToSize(cgGetParameterType(g_CGparam_worldViewProj));

    assert(cgD3D9TypeToSize(cgGetParameterType(g_CGparam_worldViewProj)) == 16 );
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: Releases all previously initialized objects
//-----------------------------------------------------------------------------
void shutDown( void )
{
    cgD3D9SetDevice(NULL);
    cgDestroyProgram(g_CGprogram_vertex);
    cgDestroyProgram(g_CGprogram_pixel);
    cgDestroyContext(g_CGcontext);

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
// Name: setShaderConstants()
// Desc: 
//-----------------------------------------------------------------------------
void setShaderConstants( void )
{
    //
    // Set constants for the vertex shader...
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

    D3DXMATRIX worldViewProj = g_matWorld * g_matView * g_matProj;
    D3DXMatrixTranspose( &worldViewProj, &worldViewProj );

    // Load a combined world-view-projection matrix
    cgD3D9SetUniformMatrix( g_CGparam_worldViewProj, &worldViewProj );

    //
    // Set constants for the pixel shader...
    //

    cgD3D9SetSamplerState(g_CGparam_testTexture, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    cgD3D9SetSamplerState(g_CGparam_testTexture, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    cgD3D9SetSamplerState(g_CGparam_testTexture, D3DSAMP_MINFILTER, D3DTEXF_POINT);
    cgD3D9SetSamplerState(g_CGparam_testTexture, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
    cgD3D9SetSamplerState(g_CGparam_testTexture, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
    cgD3D9SetTextureWrapMode(g_CGparam_testTexture, 0);

    cgD3D9SetTexture( g_CGparam_testTexture, g_pTexture );
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

    if( g_bUseShaders == true )
    {
        //
        // Use vertex and pixel shaders...
        //

        setShaderConstants();

        cgD3D9BindProgram( g_CGprogram_vertex );
        cgD3D9BindProgram( g_CGprogram_pixel );

        g_pd3dDevice->SetFVF(NULL);
        g_pd3dDevice->SetVertexDeclaration( g_pVertexDeclaration );

        g_pd3dDevice->SetStreamSource( 0, g_pVertexBuffer, 0,sizeof(Vertex) );
        g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
    }
    else
    {
        //
        // Render the normal way...
        //
    
        D3DXMATRIX matTrans;
        D3DXMATRIX matRot;

        D3DXMatrixTranslation( &matTrans, 0.0f, 0.0f, 4.0f );
        D3DXMatrixRotationYawPitchRoll( &matRot, 
                                        D3DXToRadian(g_fSpinX), 
                                        D3DXToRadian(g_fSpinY), 
                                        0.0f );
        g_matWorld = matRot * matTrans;

        g_pd3dDevice->SetTransform( D3DTS_WORLD, &g_matWorld );
        
        g_pd3dDevice->SetFVF( Vertex::FVF_Flags );
        g_pd3dDevice->SetVertexShader( NULL );
        g_pd3dDevice->SetPixelShader( NULL );

        g_pd3dDevice->SetTexture( 0, g_pTexture );
        
        g_pd3dDevice->SetStreamSource( 0, g_pVertexBuffer, 0,sizeof(Vertex) );
        g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
    }

    g_pd3dDevice->EndScene();
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}

