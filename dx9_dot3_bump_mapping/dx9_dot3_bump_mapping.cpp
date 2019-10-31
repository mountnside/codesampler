//-----------------------------------------------------------------------------
//           Name: dx9_dot3_bump_mapping.cpp
//         Author: Kevin Harris
//  Last Modified: 04/14/05
//    Description: This sample demonstrates how to perform Dot3 per-pixel bump 
//                 mapping using a normal map and the D3DTOP_DOTPRODUCT3 
//                 texture-blending operation. This technique is sometimes 
//                 referred to as per-pixel lighting or per-pixel attenuation, 
//                 but Dot3 per-pixel bump-mapping is what most programmers 
//                 know it as.
//
//                 This sample also demonstrates how to create tangent, 
//                 binormal, and normal vectors, for each vertex of our test
//                 geometry (a simple cube). These vectors are used during 
//                 rendering to define an inverse tangent matrix for each
//                 vertex. This has to be done because a normal-map stores its 
//                 normals in tangent-space. Therefore, we need an inverse 
//                 tangent matrix  so we can transform our scene's light vector 
//                 from model-space into tangent-space. Once transformed, we 
//                 then encode this new light vector as a DWORD color and pass 
//                 it into the texture blending stage as the vertex's Diffuse 
//                 color.
//
// Additional Notes:
//
//                 The phrase "Dot3" comes form the mathematical operation that 
//                 combines a light vector with a surface normal.
//
//                 The phrase "Per-pixel" comes from the fact that for every 
//                 pixel in the base texture map, we store a unique surface 
//                 normal to light it. These surface normals are passed into the
//                 texture blending stage via a normal-map. A normal-map is a 
//                 texture where normals (x,y,z) have been encoded and stored 
//                 as (r,g,b).
//
//   Control Keys: d/D - Toggle Dot3 bump mapping
//                 l/L - Toggle the usage of regular lighting in addition
//                       to the per-pixel lighting effect of bump mapping.
//                 m/M - Toggle motion of point light
//                 Up Arrow - Move the test cube closer
//                 Down Arrow - Move the test cube away
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "resource.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND                    g_hWnd          = NULL;
LPDIRECT3D9             g_pD3D          = NULL;
D3DDEVTYPE              g_deviceType    = D3DDEVTYPE_HAL;
LPDIRECT3DDEVICE9       g_pd3dDevice    = NULL;
LPDIRECT3DVERTEXBUFFER9 g_pVertexBuffer = NULL;
LPD3DXMESH              g_pSphereMesh   = NULL;

LPDIRECT3DTEXTURE9 g_pTexture = NULL;
LPDIRECT3DTEXTURE9 g_pNormalMapTexture = NULL;
bool g_bDoDot3BumpMapping = true;
bool g_bMoveLightAbout = true;
bool g_bToggleRegularLighting = false;
D3DLIGHT9 g_light0;

float g_fDistance = 5.0f;
float g_fSpinX    = 0.0f;
float g_fSpinY    = 0.0f;

float  g_fElpasedTime;
double g_dCurTime;
double g_dLastTime;

struct Vertex
{
    float x, y, z;
    float nx, ny, nz;
    DWORD diffuse;
    float tu1, tv1;
    float tu2, tv2;

    enum FVF
    {
        FVF_Flags = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX2
    };
};

const int NUM_VERTICES = 24;

Vertex g_cubeVertices[NUM_VERTICES] =
{
//     x     y     z      nx    ny    nz                         r    g    b    a        tu1 tv1    tu2 tv2
    // Front Face
    {-1.0f, 1.0f,-1.0f,  0.0f, 0.0f,-1.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   0.0f,0.0f, 0.0f,0.0f, },
    { 1.0f, 1.0f,-1.0f,  0.0f, 0.0f,-1.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   1.0f,0.0f, 1.0f,0.0f, },
    {-1.0f,-1.0f,-1.0f,  0.0f, 0.0f,-1.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   0.0f,1.0f, 0.0f,1.0f, },
    { 1.0f,-1.0f,-1.0f,  0.0f, 0.0f,-1.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   1.0f,1.0f, 1.0f,1.0f, },
    // Back Face
    {-1.0f, 1.0f, 1.0f,  0.0f, 0.0f, 1.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   1.0f,0.0f, 1.0f,0.0f, },
    {-1.0f,-1.0f, 1.0f,  0.0f, 0.0f, 1.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   1.0f,1.0f, 1.0f,1.0f, },
    { 1.0f, 1.0f, 1.0f,  0.0f, 0.0f, 1.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   0.0f,0.0f, 0.0f,0.0f, },
    { 1.0f,-1.0f, 1.0f,  0.0f, 0.0f, 1.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   0.0f,1.0f, 0.0f,1.0f, },
    // Top Face
    {-1.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   0.0f,0.0f, 0.0f,0.0f, },
    { 1.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   1.0f,0.0f, 1.0f,0.0f, },
    {-1.0f, 1.0f,-1.0f,  0.0f, 1.0f, 0.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   0.0f,1.0f, 0.0f,1.0f, },
    { 1.0f, 1.0f,-1.0f,  0.0f, 1.0f, 0.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   1.0f,1.0f, 1.0f,1.0f, },
    // Bottom Face
    {-1.0f,-1.0f, 1.0f,  0.0f,-1.0f, 0.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   0.0f,1.0f, 0.0f,1.0f, },
    {-1.0f,-1.0f,-1.0f,  0.0f,-1.0f, 0.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   0.0f,0.0f, 0.0f,0.0f, },
    { 1.0f,-1.0f, 1.0f,  0.0f,-1.0f, 0.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   1.0f,1.0f, 1.0f,1.0f, },
    { 1.0f,-1.0f,-1.0f,  0.0f,-1.0f, 0.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   1.0f,0.0f, 1.0f,0.0f, },
    // Right Face
    { 1.0f, 1.0f,-1.0f,  1.0f, 0.0f, 0.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   0.0f,0.0f, 0.0f,0.0f, },
    { 1.0f, 1.0f, 1.0f,  1.0f, 0.0f, 0.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   1.0f,0.0f, 1.0f,0.0f, },
    { 1.0f,-1.0f,-1.0f,  1.0f, 0.0f, 0.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   0.0f,1.0f, 0.0f,1.0f, },
    { 1.0f,-1.0f, 1.0f,  1.0f, 0.0f, 0.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   1.0f,1.0f, 1.0f,1.0f, },
    // Left Face
    {-1.0f, 1.0f,-1.0f, -1.0f, 0.0f, 0.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   1.0f,0.0f, 1.0f,0.0f, },
    {-1.0f,-1.0f,-1.0f, -1.0f, 0.0f, 0.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   1.0f,1.0f, 1.0f,1.0f, },
    {-1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   0.0f,0.0f, 0.0f,0.0f, },
    {-1.0f,-1.0f, 1.0f, -1.0f, 0.0f, 0.0f,  D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f),   0.0f,1.0f, 0.0f,1.0f, }
};

// For each vertex defined above, we'll need to create a Tangent, BiNormal, and
// Normal vector, which together define a tangent matrix for Dot3 bump mapping.
D3DXVECTOR3 g_vTangents[NUM_VERTICES];
D3DXVECTOR3 g_vBiNormals[NUM_VERTICES];
D3DXVECTOR3 g_vNormals[NUM_VERTICES];

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
DWORD encodeVectorAsDWORDColor(D3DXVECTOR3* vVector);
void computeTangentsMatricesForEachVertex(void);
void createTangentSpaceVectors( D3DXVECTOR3 *v1, D3DXVECTOR3 *v2, D3DXVECTOR3 *v3,
                                float v1u, float v1v, float v2u, float v2v, float v3u, float v3v, 
                                D3DXVECTOR3 *vTangent, D3DXVECTOR3 *vBiNormal, D3DXVECTOR3 *vNormal );

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
                             "Direct3D (DX9) - Dot3 Per-Pixel Bump-Mapping",
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
    static POINT ptLastMousePosit;
    static POINT ptCurrentMousePosit;
    static bool bMousing;

    switch( msg )
    {
        case WM_CHAR:
        {
            switch( wParam )
            {
                case 'd':
                case 'D':
                    g_bDoDot3BumpMapping = !g_bDoDot3BumpMapping;
                    break;

                case 'l':
                case 'L':
                    g_bToggleRegularLighting = !g_bToggleRegularLighting;
                    break;

                case 'm':
                case 'M':
                    g_bMoveLightAbout = !g_bMoveLightAbout;
                break;
            }
        }
        break;

        case WM_KEYDOWN:
        {
            switch( wParam )
            {
                case VK_ESCAPE:
                    PostQuitMessage(0);
                    break;

                case 38: // Up Arrow Key
                    g_fDistance -= 0.1f;
                    break;

                case 40: // Down Arrow Key
                    g_fDistance += 0.1f;
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
    //
    // Load the normal map...
    //

    D3DXCreateTextureFromFile( g_pd3dDevice, "stone_wall_normal_map.bmp", &g_pNormalMapTexture );
    //D3DXCreateTextureFromFile( g_pd3dDevice, "test_normal_map.bmp", &g_pNormalMapTexture );

    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );

    //
    // Load the base texture
    //

    D3DXCreateTextureFromFile( g_pd3dDevice, "stone_wall.bmp", &g_pTexture );
    //D3DXCreateTextureFromFile( g_pd3dDevice, "checker_with_numbers.bmp", &g_pTexture );

    g_pd3dDevice->SetSamplerState( 1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
    g_pd3dDevice->SetSamplerState( 1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
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
        // Respond to failure of GetDeviceCaps
        MessageBox(NULL, "Call to GetDeviceCaps failed!\n",
            "WARNING",MB_OK|MB_ICONEXCLAMATION);
        return;
    }

    // Does this device support the D3DTOP_DOTPRODUCT3 texture-blending operation?
    if( 0 == (d3dCaps.TextureOpCaps & D3DTOP_DOTPRODUCT3) )
    {
        MessageBox(NULL, "Current Direct3D driver does not support the "
            "D3DTOP_DOTPRODUCT3 texture-blending operation! \n\nSwitching to "
            "reference rasterizer!","WARNING",MB_OK|MB_ICONEXCLAMATION);

        g_deviceType = D3DDEVTYPE_REF;
    }

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

    g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, g_deviceType, g_hWnd,
                          D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                          &d3dpp, &g_pd3dDevice );

    loadTexture();

    g_pd3dDevice->CreateVertexBuffer( 24*sizeof(Vertex), D3DUSAGE_DYNAMIC, 
                                      Vertex::FVF_Flags, D3DPOOL_DEFAULT, 
                                      &g_pVertexBuffer, NULL );
    void *pVertices = NULL;

    g_pVertexBuffer->Lock( 0, sizeof(g_cubeVertices), (void**)&pVertices, 0 );
    memcpy( pVertices, g_cubeVertices, sizeof(g_cubeVertices) );
    g_pVertexBuffer->Unlock();

    D3DXMATRIX matProj;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DXToRadian( 45.0f ), 
                                640.0f / 480.0f, 0.1f, 100.0f );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

    g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE ); 
    //g_pd3dDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_FLAT ); // Direct3d doesn't like this at all when doing Dot3.
    g_pd3dDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );

    //
    // Set up lighting...
    //

    g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );

    // Set up a material
    D3DMATERIAL9 mtrl;
    ZeroMemory( &mtrl, sizeof(D3DMATERIAL9) );
    mtrl.Diffuse.r = 1.0f;
    mtrl.Diffuse.g = 1.0f;
    mtrl.Diffuse.b = 1.0f;
    mtrl.Diffuse.a = 1.0f;
    mtrl.Ambient.r = 1.0f;
    mtrl.Ambient.g = 1.0f;
    mtrl.Ambient.b = 1.0f;
    mtrl.Ambient.a = 1.0f;
    g_pd3dDevice->SetMaterial( &mtrl );

    // Set light 0 to be a pure white point light
    ZeroMemory( &g_light0, sizeof(D3DLIGHT9) );
    g_light0.Type = D3DLIGHT_POINT;
    g_light0.Position = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
    //g_light0.Range = 10.0f;
    //g_light0.Attenuation1 = 0.4f;
    g_light0.Diffuse.r = 1.0f;
    g_light0.Diffuse.g = 1.0f;
    g_light0.Diffuse.b = 1.0f;
    g_light0.Diffuse.a = 1.0f;
    g_pd3dDevice->SetLight( 0, &g_light0 );
    g_pd3dDevice->LightEnable( 0, TRUE );

    // Create a sphere mesh to represent a point light.
    D3DXCreateSphere(g_pd3dDevice, 0.05f, 8, 8, &g_pSphereMesh, NULL);

    //
    // For each vertex, create a tangent vector, binormal, and normal
    //

    // Initialize the inverse tangent matrix for each vertex to the identity 
    // matrix before we get started.
    for( int i = 0; i < NUM_VERTICES; ++i )
    {
        g_vTangents[i]  = D3DXVECTOR3( 1.0f, 0.0f, 0.0f );
        g_vBiNormals[i] = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
        g_vNormals[i]   = D3DXVECTOR3( 0.0f, 0.0f, 1.0f );
    }

    computeTangentsMatricesForEachVertex();
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )
{
    if( g_pSphereMesh != NULL )
        g_pSphereMesh->Release();

    if( g_pNormalMapTexture != NULL ) 
        g_pNormalMapTexture->Release();

    if( g_pVertexBuffer != NULL ) 
        g_pVertexBuffer->Release(); 

    if( g_pd3dDevice != NULL )
        g_pd3dDevice->Release();

    if( g_pD3D != NULL )
        g_pD3D->Release();
}

//-----------------------------------------------------------------------------
// Name: computeTangentsMatricesForEachVertex()
// Desc: 
//-----------------------------------------------------------------------------
void computeTangentsMatricesForEachVertex( void )
{
    D3DXVECTOR3 v1;
    D3DXVECTOR3 v2;
    D3DXVECTOR3 v3;
    D3DXVECTOR3 vTangent;
    D3DXVECTOR3 vBiNormal;
    D3DXVECTOR3 vNormal;

    //
    // For each cube face defined in the vertex array, compute a tangent matrix 
    // for each of the four vertices that define it.
    //

    for( int i = 0; i < NUM_VERTICES; i += 4 ) // Use += 4 to process 1 face at a time
    {
        //
        // Vertex 0 of current cube face...
        //
        //  v2
        //    3----2
        //    |    |
        //    |    |
        //    0----1
        //  v1      v3
        //

        v1 = D3DXVECTOR3(g_cubeVertices[i].x,g_cubeVertices[i].y,g_cubeVertices[i].z);
        v2 = D3DXVECTOR3(g_cubeVertices[i+3].x,g_cubeVertices[i+3].y,g_cubeVertices[i+3].z);
        v3 = D3DXVECTOR3(g_cubeVertices[i+1].x,g_cubeVertices[i+1].y,g_cubeVertices[i+1].z);

        createTangentSpaceVectors( &v1,&v2,&v3,
                                   g_cubeVertices[i].tu1, g_cubeVertices[i].tv1,
                                   g_cubeVertices[i+3].tu1, g_cubeVertices[i+3].tv1,
                                   g_cubeVertices[i+1].tu1, g_cubeVertices[i+1].tv1,
                                   &vTangent, &vBiNormal, &vNormal );
        g_vTangents[i]  = vTangent;
        g_vBiNormals[i] = vBiNormal;
        g_vNormals[i]   = vNormal;

        //
        // Vertex 1 of current cube face...
        //
        //          v3
        //    3----2
        //    |    |
        //    |    |
        //    0----1
        //  v2      v1
        //

        v1 = D3DXVECTOR3(g_cubeVertices[i+1].x,g_cubeVertices[i+1].y,g_cubeVertices[i+1].z);
        v2 = D3DXVECTOR3(g_cubeVertices[i].x,g_cubeVertices[i].y,g_cubeVertices[i].z);
        v3 = D3DXVECTOR3(g_cubeVertices[i+2].x,g_cubeVertices[i+2].y,g_cubeVertices[i+2].z);

        createTangentSpaceVectors( &v1,&v2,&v3,
                                   g_cubeVertices[i+1].tu1, g_cubeVertices[i+1].tv1,
                                   g_cubeVertices[i].tu1, g_cubeVertices[i].tv1,
                                   g_cubeVertices[i+2].tu1, g_cubeVertices[i+2].tv1,
                                   &vTangent, &vBiNormal, &vNormal );

        g_vTangents[i+1]  = vTangent;
        g_vBiNormals[i+1] = vBiNormal;
        g_vNormals[i+1]   = vNormal;

        //
        // Vertex 2 of current cube face...
        //
        //  v3      v1
        //    3----2
        //    |    |
        //    |    |
        //    0----1
        //          v2
        //

        v1 = D3DXVECTOR3(g_cubeVertices[i+2].x,g_cubeVertices[i+2].y,g_cubeVertices[i+2].z);
        v2 = D3DXVECTOR3(g_cubeVertices[i+1].x,g_cubeVertices[i+1].y,g_cubeVertices[i+1].z);
        v3 = D3DXVECTOR3(g_cubeVertices[i+3].x,g_cubeVertices[i+3].y,g_cubeVertices[i+3].z);

        createTangentSpaceVectors( &v1,&v2,&v3,
                                   g_cubeVertices[i+2].tu1, g_cubeVertices[i+2].tv1,
                                   g_cubeVertices[i+1].tu1, g_cubeVertices[i+1].tv1,
                                   g_cubeVertices[i+3].tu1, g_cubeVertices[i+3].tv1,
                                   &vTangent, &vBiNormal, &vNormal );

        g_vTangents[i+2]  = vTangent;
        g_vBiNormals[i+2] = vBiNormal;
        g_vNormals[i+2]   = vNormal;

        //
        // Vertex 3 of current cube face...
        //
        //  v1      v2
        //    3----2
        //    |    |
        //    |    |
        //    0----1
        //  v3
        //

        v1 = D3DXVECTOR3(g_cubeVertices[i+3].x,g_cubeVertices[i+3].y,g_cubeVertices[i+3].z);
        v2 = D3DXVECTOR3(g_cubeVertices[i+2].x,g_cubeVertices[i+2].y,g_cubeVertices[i+2].z);
        v3 = D3DXVECTOR3(g_cubeVertices[i].x,g_cubeVertices[i].y,g_cubeVertices[i].z);

        createTangentSpaceVectors( &v1,&v2,&v3,
                                   g_cubeVertices[i+3].tu1, g_cubeVertices[i+3].tv1,
                                   g_cubeVertices[i+2].tu1, g_cubeVertices[i+2].tv1,
                                   g_cubeVertices[i].tu1, g_cubeVertices[i].tv1,
                                   &vTangent, &vBiNormal, &vNormal );

        g_vTangents[i+3]  = vTangent;
        g_vBiNormals[i+3] = vBiNormal;
        g_vNormals[i+3]   = vNormal;
    }
}

//-----------------------------------------------------------------------------
// Name: createTangentSpaceVectors()
// Desc: Given a vertex (v1) and two other vertices (v2 & v3) which define a 
//       triangle, this function will return Tangent, BiNormal, and Normal, 
//       vectors which can be used to define the tangent matrix for the first 
//       vertex's position (v1).
//
// Args: v1        - vertex 1
//       v2        - vertex 2
//       v3        - vertex 3
//       v1u, v1v  - texture-coordinates of vertex 1
//       v2u, v2v  - texture-coordinates of vertex 2
//       v3u, v3v  - texture-coordinates of vertex 3
//       vTangent  - When the function returns, this will be set as the tangent vector
//       vBiNormal - When the function returns, this will be set as the binormal vector
//       vNormal   - When the function returns, this will be set as the normal vector
//
// Note: This function is based on an article by By Jakob Gath and Sbren Dreijer.
// http://www.blacksmith-studios.dk/projects/downloads/tangent_matrix_derivation.php
//------------------------------------------------------------------------------

void createTangentSpaceVectors( D3DXVECTOR3 *v1,
                                D3DXVECTOR3 *v2,
                                D3DXVECTOR3 *v3,
                                float v1u, float v1v,
                                float v2u, float v2v,
                                float v3u, float v3v,
                                D3DXVECTOR3 *vTangent,
                                D3DXVECTOR3 *vBiNormal,
                                D3DXVECTOR3 *vNormal )
{
    // Create edge vectors from vertex 1 to vectors 2 and 3.
    D3DXVECTOR3 vDirVec_v2_to_v1 = *v2 - *v1;
    D3DXVECTOR3 vDirVec_v3_to_v1 = *v3 - *v1;

    // Create edge vectors from the texture coordinates of vertex 1 to vector 2.
    float vDirVec_v2u_to_v1u = v2u - v1u;
    float vDirVec_v2v_to_v1v = v2v - v1v;

    // Create edge vectors from the texture coordinates of vertex 1 to vector 3.
    float vDirVec_v3u_to_v1u = v3u - v1u;
    float vDirVec_v3v_to_v1v = v3v - v1v;

    float fDenominator = vDirVec_v2u_to_v1u * vDirVec_v3v_to_v1v - 
                         vDirVec_v3u_to_v1u * vDirVec_v2v_to_v1v;

    if( fDenominator < 0.0001f && fDenominator > -0.0001f )
    {
        // We're too close to zero and we're at risk of a divide-by-zero! 
        // Set the tangent matrix to the identity matrix and do nothing.
        *vTangent  = D3DXVECTOR3( 1.0f, 0.0f, 0.0f );
        *vBiNormal = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
        *vNormal   = D3DXVECTOR3( 0.0f, 0.0f, 1.0f );
    }
    else
    {
        // Calculate and cache the reciprocal value
        float fScale1 = 1.0f / fDenominator;

        D3DXVECTOR3 T;
        D3DXVECTOR3 B;
        D3DXVECTOR3 N;

        T = D3DXVECTOR3((vDirVec_v3v_to_v1v * vDirVec_v2_to_v1.x - vDirVec_v2v_to_v1v * vDirVec_v3_to_v1.x) * fScale1,
                        (vDirVec_v3v_to_v1v * vDirVec_v2_to_v1.y - vDirVec_v2v_to_v1v * vDirVec_v3_to_v1.y) * fScale1,
                        (vDirVec_v3v_to_v1v * vDirVec_v2_to_v1.z - vDirVec_v2v_to_v1v * vDirVec_v3_to_v1.z) * fScale1);

        B = D3DXVECTOR3((-vDirVec_v3u_to_v1u * vDirVec_v2_to_v1.x + vDirVec_v2u_to_v1u * vDirVec_v3_to_v1.x) * fScale1,
                        (-vDirVec_v3u_to_v1u * vDirVec_v2_to_v1.y + vDirVec_v2u_to_v1u * vDirVec_v3_to_v1.y) * fScale1,
                        (-vDirVec_v3u_to_v1u * vDirVec_v2_to_v1.z + vDirVec_v2u_to_v1u * vDirVec_v3_to_v1.z) * fScale1);

        // The normal N is calculated as the cross product between T and B
        D3DXVec3Cross( &N, &T, &B );

        // Calculate and cache the reciprocal value
        float fScale2 = 1.0f / ((T.x * B.y * N.z - T.z * B.y * N.x) + 
                                (B.x * N.y * T.z - B.z * N.y * T.x) + 
                                (N.x * T.y * B.z - N.z * T.y * B.x));

        //
        // Use the temporary T (Tangent), (B) Binormal, and N (Normal) vectors 
        // to calculate the inverse of the tangent matrix that they represent.
        // The inverse of the tangent matrix is what we want since we need that
        // to transform the light's vector into tangent-space.
        //

        D3DXVECTOR3 vTemp;

        (*vTangent).x =   D3DXVec3Cross( &vTemp, &B, &N )->x * fScale2;
        (*vTangent).y = -(D3DXVec3Cross( &vTemp, &N, &T )->x * fScale2);
        (*vTangent).z =   D3DXVec3Cross( &vTemp, &T, &B )->x * fScale2;
        D3DXVec3Normalize( &(*vTangent), &(*vTangent) );

        (*vBiNormal).x = -(D3DXVec3Cross( &vTemp, &B, &N )->y * fScale2);
        (*vBiNormal).y =   D3DXVec3Cross( &vTemp, &N, &T )->y * fScale2;
        (*vBiNormal).z = -(D3DXVec3Cross( &vTemp, &T, &B )->y * fScale2);
        D3DXVec3Normalize( &(*vBiNormal), &(*vBiNormal) );

        (*vNormal).x =   D3DXVec3Cross( &vTemp, &B, &N )->z * fScale2;
        (*vNormal).y = -(D3DXVec3Cross( &vTemp, &N, &T )->z * fScale2);
        (*vNormal).z =   D3DXVec3Cross( &vTemp, &T, &B )->z * fScale2;
        D3DXVec3Normalize( &(*vNormal), &(*vNormal) );

        //
        // NOTE: Since the texture-space of Direct3D and OpenGL are laid-out 
        //       differently, a single normal map can't look right in both 
        //       unless you make some adjustments somewhere.
        //
        //       You can adjust or fix this problem in three ways:
        //
        //       1. Create two normal maps: one for OpenGL and one for Direct3D.
        //       2. Flip the normal map image over as you load it into a texture 
        //          object.
        //       3. Flip the binormal over when computing the tangent-space
        //          matrix.
        //
        // Since the normal map used by this sample was created for Direct3D,
        // no adjustment is necessary.
        //
        //*vBiNormal = *vBiNormal * -1.0f;
    }
}

//-----------------------------------------------------------------------------
// Name: encodeVectorAsDWORDColor()
// Desc: 
//-----------------------------------------------------------------------------
DWORD encodeVectorAsDWORDColor( D3DXVECTOR3* vVector )
{
    DWORD dwRed   = (DWORD)(127 * vVector->x + 128);
    DWORD dwGreen = (DWORD)(127 * vVector->y + 128);
    DWORD dwBlue  = (DWORD)(127 * vVector->z + 128);

    return (DWORD)(0xff000000 + (dwRed << 16) + (dwGreen << 8) + dwBlue);
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: 
//-----------------------------------------------------------------------------
void render( void )
{
    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                         D3DCOLOR_COLORVALUE(0.35f, 0.53f, 0.7, 1.0f), 1.0f, 0 );

    if( g_bMoveLightAbout == true )
    {
        //
        // Spin our point light around the cube...
        //

        static float fAngle = 0;
        fAngle += 60 * g_fElpasedTime;
        // Wrap it around, if it gets too big
        while(fAngle > 360.0f) fAngle -= 360.0f;
        while(fAngle < 0.0f)   fAngle += 360.0f;
        float x = sinf( D3DXToRadian(fAngle) );
        float y = cosf( D3DXToRadian(fAngle) );

        // This will place the light directly into world-space
        g_light0.Position = D3DXVECTOR3( 1.2f * x,
                                         1.2f * y,
                                         g_fDistance - 2.0f );

        g_pd3dDevice->SetLight( 0, &g_light0 );
    }

    D3DXMATRIX matWorld;
    D3DXMATRIX matView;
    D3DXMATRIX matProj;
    D3DXMATRIX matTrans;
    D3DXMATRIX matRot;

    D3DXMatrixTranslation( &matTrans, 0.0f, 0.0f, g_fDistance );

    D3DXMatrixRotationYawPitchRoll( &matRot, 
                                    D3DXToRadian(g_fSpinX), 
                                    D3DXToRadian(g_fSpinY), 
                                    0.0f );
    
    // This sample is not really making use of a view matrix
    D3DXMatrixIdentity( &matView );

    matWorld = matRot * matTrans;
    g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

    g_pd3dDevice->BeginScene();

    if( g_bToggleRegularLighting == true )
        g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
    else
        g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );

    if( g_bDoDot3BumpMapping == true )
    {
        //
        // Render a Dot3 bump mapped cube...
        //

        //
        // Transform the light's position from world-space to model-space
        //

        D3DXVECTOR3 vLightPosWS;    // Light position (in world-space)
        D3DXVECTOR3 vLightPosMS;    // Light position (in model-space)
        D3DXVECTOR3 vVertToLightMS; // L vector of N.L (in model-space)
        D3DXVECTOR3 vVertToLightTS; // L vector of N.L (in tangent-space)

        // Get the light's current position, which is in world-space.
        vLightPosWS = g_light0.Position;

        // Transform the light's position into model-space
        D3DXMATRIX worldInverse;
        D3DXMatrixInverse( &worldInverse, NULL, &matWorld );
        D3DXVec3TransformCoord( &vLightPosMS, &vLightPosWS, &worldInverse );

        //
        // Since our cube's vertex data is stored in a Vertex Buffer, we will
        // need to lock it briefly so we can encode the new tangent-space
        // L vectors that we're going to create into the diffuse color of each 
        // vertex.
        //

        Vertex *pVertices = NULL;
        g_pVertexBuffer->Lock( 0, 0, (void**)&pVertices, 0 );

        for( int i = 0; i < NUM_VERTICES; ++i )
        {
            //
            // For each vertex, rotate L (of N.L) into tangent-space and 
            // pass it into Direct3D's texture blending system by packing it 
            // into the vertex's diffuse color.
            //

            D3DXVECTOR3 vCurrentVertex = D3DXVECTOR3( g_cubeVertices[i].x, 
                                                      g_cubeVertices[i].y, 
                                                      g_cubeVertices[i].z );

            vVertToLightMS = vLightPosMS - vCurrentVertex;
            D3DXVec3Normalize( &vVertToLightMS, &vVertToLightMS );

            //
            // Build up an inverse tangent-space matrix using the Tangent, 
            // Binormal, and Normal calculated for the current vertex and 
            // then use it to transform our L vector (of N.L), which is in 
            // model-space, into tangent-space.
            //
            // A tangent matrix is of the form:
            //
            // |Tx Bx Nx 0|
            // |Ty By Ny 0|
            // |Tz Bz Nz 0|
            // |0  0  0  1|
            //
            // Note: Our vectors have already been inverted, so there is no 
            // need to invert our tangent matrix once we build it up.
            //

            //                           Tangent           Binormal           Normal
            D3DXMATRIX invTangentMatrix( g_vTangents[i].x, g_vBiNormals[i].x, g_vNormals[i].x, 0.0f,
                                         g_vTangents[i].y, g_vBiNormals[i].y, g_vNormals[i].y, 0.0f,
                                         g_vTangents[i].z, g_vBiNormals[i].z, g_vNormals[i].z, 0.0f,
                                         0.0f,             0.0f,              0.0f,            1.0f );

            D3DXVec3TransformNormal( &vVertToLightTS, &vVertToLightMS, &invTangentMatrix );

            //
            // Last but not least, we must encode our new L vector as a DWORD 
            // value so we can set it as the new vertex color. Of course, the 
            // hardware assumes that we are  going to do this, so it will 
            // simply decode the original vector back out by reversing the 
            // DOWRD encdoing we've performed here.
            //

            pVertices[i].diffuse = encodeVectorAsDWORDColor( &vVertToLightTS );
        }

        g_pVertexBuffer->Unlock();

        //
        // STAGE 0
        //
        // Use D3DTOP_DOTPRODUCT3 to find the dot-product of (N.L), where N is 
        // stored in the normal map, and L is passed in as the vertex color - 
        // D3DTA_DIFFUSE.
        //
        
        g_pd3dDevice->SetTexture( 0, g_pNormalMapTexture );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );

        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_DOTPRODUCT3 ); // Perform a Dot3 operation...
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );    // between the N (of N.L) which is stored in a normal map texture...
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );    // with the L (of N.L) which is stored in the vertex's diffuse color.

        //
        // STAGE 1
        //
        // Modulate the base texture by N.L calculated in STAGE 0.
        //

        g_pd3dDevice->SetTexture( 1, g_pTexture );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 1 );

        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_MODULATE ); // Modulate...
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE ); // the texture for this stage with...
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT ); // the current argument passed down from stage 0
    }
    else
    {
        //
        // Render a regular textured cube with no Dot3 bump mapping...
        //

        // Reset the vertex colors back to pure white...
        Vertex *pVertices = NULL;
        g_pVertexBuffer->Lock( 0, 0, (void**)&pVertices, 0 );
        for( int i = 0; i < NUM_VERTICES; ++i )
            pVertices[i].diffuse = D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f);
        g_pVertexBuffer->Unlock();

        //
        // STAGE 0
        //

        g_pd3dDevice->SetTexture( 0, g_pTexture );

        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

        //
        // STAGE 1
        //

        g_pd3dDevice->SetTexture( 1, NULL );

        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_MODULATE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
    }

    //
    // Now, render our textured test cube, which consists of 6 cube faces...
    //

    g_pd3dDevice->SetStreamSource( 0, g_pVertexBuffer, 0, sizeof(Vertex) );
    g_pd3dDevice->SetFVF( Vertex::FVF_Flags );

    g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,  0, 2 );
    g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,  4, 2 );
    g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,  8, 2 );
    g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 12, 2 );
    g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 16, 2 );
    g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 20, 2 );

    //
    // Render a small white sphere to mark the point light's position...
    //

    g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );

    g_pd3dDevice->SetTexture( 0, NULL );
    g_pd3dDevice->SetTexture( 1, NULL );

    D3DXMatrixIdentity( &matWorld );
    D3DXMatrixTranslation( &matWorld, g_light0.Position.x,
                                      g_light0.Position.y,
                                      g_light0.Position.z );

    g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );
    g_pSphereMesh->DrawSubset(0);

    g_pd3dDevice->EndScene();
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}
