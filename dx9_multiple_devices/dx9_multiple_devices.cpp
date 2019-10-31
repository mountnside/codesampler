//-----------------------------------------------------------------------------
//           Name: dx9_multiple_devices.cpp
//         Author: Kevin Harris
//  Last Modified: 02/01/05
//    Description: This sample demonstrates how to render to multiple windows
//                 using multiple devices under DirectX 9.0. You should note
//                 that this is not considered the most optimal way to manage
//                 two windows from one application. The preferred method is to
//                 create one Direct3D device and use CreateAdditionalSwapChain
//                 to create additional front and back buffer pairs for
//                 rendering in multiple windows. This allows two windows to 
//                 share resources like meshes and textures and keeps the 
//                 driver from performing a costly context switch between 
//                 multiple devices.
//
//   Control Keys: Left Mouse Button - Spin the teapot
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
LPDIRECT3D9 g_pD3D   = NULL;
HWND        g_hWnd_0 = NULL; // Handle to the first window
HWND        g_hWnd_1 = NULL; // Handle to the second window

LPDIRECT3DDEVICE9 g_pd3dDevice_0  = NULL; // Direct3d device for the first window
LPDIRECT3DDEVICE9 g_pd3dDevice_1  = NULL; // Direct3d device for the second window

LPD3DXMESH g_pTeapotMesh_0 = NULL; // The two devices can't share resources, so we'll
LPD3DXMESH g_pTeapotMesh_1 = NULL; // have to create two meshes... one for each device

D3DMATERIAL9 g_teapotMtrl;
D3DLIGHT9    g_pLight0;

LPD3DXFONT g_pd3dxFont = NULL;

DWORD g_dwBackBufferWidth  = 0;
DWORD g_dwBackBufferHeight = 0;

float g_fSpinX = 0.0f;
float g_fSpinY = 0.0f;

double g_dElpasedFrameTime = 0.0f;
double g_dElpasedAppTime   = 0.0f;
double g_dCurrentTime      = 0.0f;
double g_dLastTime         = 0.0f;

struct Vertex
{
    float x, y, z;    // Position of vertex in 3D space
    float nx, ny, nz; // Normal for lighting calculations
    DWORD diffuse;    // Diffuse color of vertex

    enum FVF
    {
        FVF_Flags = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE
    };
};

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void createFont(void);
void init_0(void);
void init_1(void);
void shutDown_0(void);
void shutDown_1(void);
void render_0(void);
void render_1(void);

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

    if(!RegisterClassEx( &winClass) )
        return E_FAIL;

    //
    // Create window #0...
    //

    g_hWnd_0 = CreateWindowEx( NULL, "MY_WINDOWS_CLASS", 
                               "Direct3D (DX9) - Multiple Devices (Window #0)",
                               WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                               0, 0, 640, 480, NULL, NULL, hInstance, NULL );

    if( g_hWnd_0 == NULL )
        return E_FAIL;

    ShowWindow( g_hWnd_0, nCmdShow );
    UpdateWindow( g_hWnd_0 );

    //
    // Create window #1...
    //

    g_hWnd_1 = CreateWindowEx( NULL, "MY_WINDOWS_CLASS", 
                               "Direct3D (DX9) - Multiple Devices (Window #1)",
                               WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                               200, 200, 640, 480, NULL, NULL, hInstance, NULL );

    if( g_hWnd_1 == NULL )
        return E_FAIL;

    ShowWindow( g_hWnd_1, nCmdShow );
    UpdateWindow( g_hWnd_1 );

    //
    // Init Direct3D usage on both windows...
    //

    init_0();
    init_1();

    while( uMsg.message != WM_QUIT )
    {
        if( PeekMessage( &uMsg, NULL, 0, 0, PM_REMOVE ) )
        { 
            TranslateMessage( &uMsg );
            DispatchMessage( &uMsg );
        }
        else
        {
            g_dCurrentTime = timeGetTime();
            g_dElpasedFrameTime = g_dCurrentTime - g_dLastTime; // How much time has passed since the last frame?
            g_dElpasedAppTime += g_dElpasedFrameTime; // How much time has passed overall for the application?
            g_dLastTime = g_dCurrentTime;

            //
            // Rendee something to both windows...
            //

            render_0();
            render_1();
        }
    }

    //
    // Cleanup Direct3D usage on both windows...
    //

    shutDown_1();
    shutDown_0();

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
    hr = D3DXCreateFont( g_pd3dDevice_0, nHeight, 0, FW_BOLD, 0, FALSE, 
                         DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, 
                         DEFAULT_PITCH | FF_DONTCARE, TEXT("Arial"), 
                         &g_pd3dxFont );

    if( FAILED( hr ) )
        MessageBox(NULL,"Call to D3DXCreateFont failed!", "ERROR",MB_OK|MB_ICONEXCLAMATION);
}

//-----------------------------------------------------------------------------
// Name: init_0()
// Desc: 
//-----------------------------------------------------------------------------
void init_0( void )
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

    g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, 
                          D3DDEVTYPE_HAL,
                          g_hWnd_0, D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                          &d3dpp, &g_pd3dDevice_0 );

    createFont();

    D3DXMATRIX matProj;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DXToRadian( 45.0f ), 
                                640.0f / 480.0f, 0.1f, 100.0f );
    g_pd3dDevice_0->SetTransform( D3DTS_PROJECTION, &matProj );

    g_pd3dDevice_0->SetRenderState( D3DRS_ZENABLE, TRUE );
    g_pd3dDevice_0->SetRenderState( D3DRS_LIGHTING, TRUE );
    g_pd3dDevice_0->SetRenderState( D3DRS_SPECULARENABLE, TRUE );

    // Setup a material for the teapot
    ZeroMemory( &g_teapotMtrl, sizeof(D3DMATERIAL9) );

    g_teapotMtrl.Diffuse.r = 1.0f;
    g_teapotMtrl.Diffuse.g = 1.0f;
    g_teapotMtrl.Diffuse.b = 1.0f;
    g_teapotMtrl.Diffuse.a = 1.0f;

    // Setup a simple directional light and some ambient...
    g_pLight0.Type = D3DLIGHT_DIRECTIONAL;
    g_pLight0.Direction = D3DXVECTOR3( 1.0f, 0.0f, 1.0f );

    g_pLight0.Diffuse.r = 1.0f;
    g_pLight0.Diffuse.g = 1.0f;
    g_pLight0.Diffuse.b = 1.0f;
    g_pLight0.Diffuse.a = 1.0f;

    g_pLight0.Specular.r = 1.0f;
    g_pLight0.Specular.g = 1.0f;
    g_pLight0.Specular.b = 1.0f;
    g_pLight0.Specular.a = 1.0f;

    g_pd3dDevice_0->SetLight( 0, &g_pLight0 );
    g_pd3dDevice_0->LightEnable( 0, TRUE );

    g_pd3dDevice_0->SetRenderState( D3DRS_AMBIENT, D3DCOLOR_COLORVALUE( 0.2f, 0.2f, 0.2f, 1.0f ) );

    // Load up the teapot mesh...
    D3DXLoadMeshFromX( "teapot.x", D3DXMESH_SYSTEMMEM, g_pd3dDevice_0, 
                       NULL, NULL, NULL, NULL, &g_pTeapotMesh_0 );


    //
    // Get the width & height of the back-buffer...
    //

    LPDIRECT3DSURFACE9 pBackBuffer = NULL;
    D3DSURFACE_DESC d3dsd;
    g_pd3dDevice_0->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
    pBackBuffer->GetDesc( &d3dsd );
    pBackBuffer->Release();
    g_dwBackBufferWidth  = d3dsd.Width;
    g_dwBackBufferHeight = d3dsd.Height;
}

//-----------------------------------------------------------------------------
// Name: init_1()
// Desc: 
//-----------------------------------------------------------------------------
void init_1( void )
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

    g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, 
                          D3DDEVTYPE_HAL,
                          g_hWnd_1, D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                          &d3dpp, &g_pd3dDevice_1 );

    D3DXMATRIX matProj;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DXToRadian( 45.0f ), 
                                640.0f / 480.0f, 0.1f, 100.0f );
    g_pd3dDevice_1->SetTransform( D3DTS_PROJECTION, &matProj );

    g_pd3dDevice_1->SetRenderState( D3DRS_ZENABLE, TRUE );
    g_pd3dDevice_1->SetRenderState( D3DRS_LIGHTING, TRUE );
    g_pd3dDevice_1->SetRenderState( D3DRS_SPECULARENABLE, TRUE );

    // Setup a material for the teapot
    ZeroMemory( &g_teapotMtrl, sizeof(D3DMATERIAL9) );

    g_teapotMtrl.Diffuse.r = 1.0f;
    g_teapotMtrl.Diffuse.g = 1.0f;
    g_teapotMtrl.Diffuse.b = 1.0f;
    g_teapotMtrl.Diffuse.a = 1.0f;

    // Setup a simple directional light and some ambient...
    g_pLight0.Type = D3DLIGHT_DIRECTIONAL;
    g_pLight0.Direction = D3DXVECTOR3( 1.0f, 0.0f, 1.0f );

    g_pLight0.Diffuse.r = 1.0f;
    g_pLight0.Diffuse.g = 1.0f;
    g_pLight0.Diffuse.b = 1.0f;
    g_pLight0.Diffuse.a = 1.0f;

    g_pLight0.Specular.r = 1.0f;
    g_pLight0.Specular.g = 1.0f;
    g_pLight0.Specular.b = 1.0f;
    g_pLight0.Specular.a = 1.0f;

    g_pd3dDevice_1->SetLight( 0, &g_pLight0 );
    g_pd3dDevice_1->LightEnable( 0, TRUE );

    g_pd3dDevice_1->SetRenderState( D3DRS_AMBIENT, D3DCOLOR_COLORVALUE( 0.2f, 0.2f, 0.2f, 1.0f ) );

    // Load up the teapot mesh...
    D3DXLoadMeshFromX( "teapot.x", D3DXMESH_SYSTEMMEM, g_pd3dDevice_1, 
                       NULL, NULL, NULL, NULL, &g_pTeapotMesh_1 );


    //
    // Get the width & height of the back-buffer...
    //

    LPDIRECT3DSURFACE9 pBackBuffer = NULL;
    D3DSURFACE_DESC d3dsd;
    g_pd3dDevice_1->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
    pBackBuffer->GetDesc( &d3dsd );
    pBackBuffer->Release();
    g_dwBackBufferWidth  = d3dsd.Width;
    g_dwBackBufferHeight = d3dsd.Height;
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown_0( void )
{
    if( g_pTeapotMesh_0 != NULL )
        g_pTeapotMesh_0->Release();

    if( g_pd3dDevice_0 != NULL )
        g_pd3dDevice_0->Release();

    if( g_pD3D != NULL )
        g_pD3D->Release();
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown_1( void )
{
    if( g_pTeapotMesh_1 != NULL )
        g_pTeapotMesh_1->Release();

    if( g_pd3dDevice_1 != NULL )
        g_pd3dDevice_1->Release();
}

//-----------------------------------------------------------------------------
// Name: render_0()
// Desc: 
//-----------------------------------------------------------------------------
void render_0( void )
{
    D3DXMATRIX matView;
    D3DXMATRIX matWorld;
    D3DXMATRIX matRotation;
    D3DXMATRIX matTranslation;

    // Now we can clear just view-port's portion of the buffer to red...
    g_pd3dDevice_0->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                         D3DCOLOR_COLORVALUE( 1.0f, 0.0f, 0.0f, 1.0f ), 1.0f, 0 );

    g_pd3dDevice_0->BeginScene();
    {
        // For the left view-port, leave the view at the origin...
        D3DXMatrixIdentity( &matView );
        g_pd3dDevice_0->SetTransform( D3DTS_VIEW, &matView );

        // ... and use the world matrix to spin and translate the teapot  
        // out where we can see it...
        D3DXMatrixRotationYawPitchRoll( &matRotation, D3DXToRadian(g_fSpinX), D3DXToRadian(g_fSpinY), 0.0f );
        D3DXMatrixTranslation( &matTranslation, 0.0f, 0.0f, 5.0f );
        matWorld = matRotation * matTranslation;
        g_pd3dDevice_0->SetTransform( D3DTS_WORLD, &matWorld );

        g_pd3dDevice_0->SetMaterial( &g_teapotMtrl );
        g_pTeapotMesh_0->DrawSubset(0);
    }
    g_pd3dDevice_0->EndScene();
 
    //
    // Report frames per second and the number of objects culled...
    //

    static int nFrameCount = 0;
    ++nFrameCount;

    static double nTimeOfLastFPSUpdate = 0.0;
    static char fpsString[50] = "Frames Per Second = ";

    if( g_dElpasedAppTime - nTimeOfLastFPSUpdate > 1000 ) // Update once a second
    {
        sprintf( fpsString, "Frames Per Second = %4.2f",
            nFrameCount*1000.0/(g_dElpasedAppTime - nTimeOfLastFPSUpdate) );

        nTimeOfLastFPSUpdate = g_dElpasedAppTime;
        nFrameCount = 0;
    }

    RECT destRect;
    SetRect( &destRect, 5, 5, 0, 0 );

    g_pd3dxFont->DrawText( NULL, fpsString, -1, &destRect, DT_NOCLIP, 
                           D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );

    g_pd3dDevice_0->Present( NULL, NULL, NULL, NULL );
}

//-----------------------------------------------------------------------------
// Name: render_1()
// Desc: 
//-----------------------------------------------------------------------------
void render_1( void )
{
    D3DXMATRIX matView;
    D3DXMATRIX matWorld;
    D3DXMATRIX matRotation;
    D3DXMATRIX matTranslation;

    // Now we can clear just view-port's portion of the buffer to green...
    g_pd3dDevice_1->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                           D3DCOLOR_COLORVALUE( 0.0f, 1.0f, 0.0f, 1.0f ), 1.0f, 0 );

    g_pd3dDevice_1->BeginScene();
    {
        // For the right view-port, translate and rotate the view around 
        // the teapot so we can see it...
        D3DXMatrixRotationYawPitchRoll( &matRotation, D3DXToRadian(g_fSpinX), D3DXToRadian(g_fSpinY), 0.0f );
        D3DXMatrixTranslation( &matTranslation, 0.0f, 0.0f, 5.0f );
        matView = matRotation * matTranslation;
        g_pd3dDevice_1->SetTransform( D3DTS_VIEW, &matView );

        // ... and don't bother with the world matrix at all.
        D3DXMatrixIdentity( &matWorld );
        g_pd3dDevice_1->SetTransform( D3DTS_WORLD, &matWorld );

        g_pd3dDevice_1->SetMaterial( &g_teapotMtrl );
        g_pTeapotMesh_1->DrawSubset(0);
    }
    g_pd3dDevice_1->EndScene();

    //
    // We're done! Now, we just call Present()
    //

    g_pd3dDevice_1->Present( NULL, NULL, NULL, NULL );
}

