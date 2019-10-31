//-----------------------------------------------------------------------------
//           Name: dx9_resize_window.cpp
//         Author: Kevin Harris
//  Last Modified: 06/16/05
//    Description: This sample demonstrates how to respond to the app's window  
//                 getting resized by resizing the front and back buffers of  
//                 the Direct3D device to match it. If you don't do this,   
//                 Direct3D will be forced to perform a stretch blit when the 
//                 window is enlarged and everything rendered will appear
//                 course and blocky. For example, if the initial window size 
//                 and Direct3D device are set to 640x480 and you enlarge the 
//                 window to be 1024x768, the Direct3D device will continue to 
//                 render at 640x480 unless its front and back buffers are  
//                 resized accordingly.
//
//                 To see what happens when you don't handle resizing properly, 
//                 run the sample and maximize the window. Once maximized, 
//                 note how smooth the teapot is and how clean the texturing 
//                 on the quad is. Next, grab the window's corner and 
//                 considerably reduce the window's size and hit F1 to toggle 
//                 off the handling code and maximize it again. When the 
//                 window is maximized the teapot and textured quad should 
//                 appear highly pixilated.
//
//   Control Keys: F1 - Toggle handling of WM_SIZE window message
//                 Left Mouse Button - Spin the teapot and textured quad
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "resource.h"

//-----------------------------------------------------------------------------
// MACROS
//-----------------------------------------------------------------------------
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND                    g_hWnd          = NULL;
LPDIRECT3D9             g_pD3D          = NULL;
LPDIRECT3DDEVICE9       g_pd3dDevice    = NULL;
LPD3DXMESH              g_pTeapotMesh   = NULL;
D3DMATERIAL9            g_teapotMtrl;
LPDIRECT3DVERTEXBUFFER9 g_pVertexBuffer = NULL;
LPDIRECT3DTEXTURE9      g_pTexture      = NULL;
D3DMATERIAL9            g_quadMtrl;
D3DLIGHT9               g_pLight0;
D3DPRESENT_PARAMETERS   g_d3dpp;

bool g_bHandleWindowResize = true;
float g_fSpinX = 0.0f;
float g_fSpinY = 0.0f;

struct TeapotVertex
{
    float x, y, z;
    float nx, ny, nz;
    DWORD diffuse;

    enum FVF
    {
        FVF_Flags = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE
    };
};

struct QuadVertex
{
    float x , y, z;
    float nx, ny, nz;
    float tu, tv;

    enum FVF
    {
        FVF_Flags = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1
    };
};

QuadVertex g_quadVertices[] =
{
//     x      y     z     nx    ny     nz     tu   tv
    {-1.0f, 1.0f, 0.0f,  0.0f, 0.0f, -1.0f,  0.0f,0.0f },
    { 1.0f, 1.0f, 0.0f,  0.0f, 0.0f, -1.0f,  1.0f,0.0f },
    {-1.0f,-1.0f, 0.0f,  0.0f, 0.0f, -1.0f,  0.0f,1.0f },
    { 1.0f,-1.0f, 0.0f,  0.0f, 0.0f, -1.0f,  1.0f,1.0f }
};

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void oneTimeSystemInit(void);
HRESULT restoreDeviceObjects(void);
HRESULT invalidateDeviceObjects(void);
void render(void);
void shutDown(void);

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
                             "Direct3D (DX9) - Resize Window",
                             WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                             0, 0, 640, 480, NULL, NULL, hInstance, NULL );

    if( g_hWnd == NULL )
        return E_FAIL;

    ShowWindow( g_hWnd, nCmdShow );
    UpdateWindow( g_hWnd );

    oneTimeSystemInit();

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
					g_bHandleWindowResize = !g_bHandleWindowResize;
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

        case WM_SIZE:
        {
			if( g_bHandleWindowResize == true )
			{
				// If the device is not NULL and the WM_SIZE message is not a
				// SIZE_MINIMIZED event, resize the device's swap buffers to match
				// the new window size.
				if( g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED )
				{
					invalidateDeviceObjects();

					g_d3dpp.BackBufferWidth  = LOWORD(lParam);
					g_d3dpp.BackBufferHeight = HIWORD(lParam);

					HRESULT hr = g_pd3dDevice->Reset( &g_d3dpp );

                    if( hr == D3DERR_INVALIDCALL )
                    {
                        MessageBox( NULL, "Call to Reset() failed with D3DERR_INVALIDCALL! ",
                            "ERROR", MB_OK | MB_ICONEXCLAMATION );
                    }

					restoreDeviceObjects();
				}
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
// Name: oneTimeSystemInit()
// Desc: This function will only be called once during the application's 
//       initialization phase. Therefore, it can't contain any resources that 
//       need to be restored every time the Direct3D device is lost or the 
//       window is resized.
//-----------------------------------------------------------------------------
void oneTimeSystemInit( void )
{
    g_pD3D = Direct3DCreate9( D3D_SDK_VERSION );

    D3DDISPLAYMODE d3ddm;

    g_pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm );

    ZeroMemory( &g_d3dpp, sizeof(g_d3dpp) );

    g_d3dpp.Windowed               = TRUE;
    g_d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat       = d3ddm.Format;
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_IMMEDIATE;

    g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
                          g_hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                          &g_d3dpp, &g_pd3dDevice );

    // Setup a material for the teapot
    ZeroMemory( &g_teapotMtrl, sizeof(D3DMATERIAL9) );
	g_teapotMtrl.Diffuse.r = 0.0f;
	g_teapotMtrl.Diffuse.g = 1.0f;
	g_teapotMtrl.Diffuse.b = 1.0f;
	g_teapotMtrl.Diffuse.a = 1.0f;
    g_teapotMtrl.Specular.r = 1.0f;
    g_teapotMtrl.Specular.g = 1.0f;
    g_teapotMtrl.Specular.b = 1.0f;
    g_teapotMtrl.Specular.a = 1.0f;
    g_teapotMtrl.Power = 40.0f;

    // Setup a material for the textured quad
    ZeroMemory( &g_quadMtrl, sizeof(D3DMATERIAL9) );
    g_quadMtrl.Diffuse.r = 1.0f;
    g_quadMtrl.Diffuse.g = 1.0f;
    g_quadMtrl.Diffuse.b = 1.0f;
    g_quadMtrl.Diffuse.a = 1.0f;
    g_quadMtrl.Specular.r = 1.0f;
    g_quadMtrl.Specular.g = 1.0f;
    g_quadMtrl.Specular.b = 1.0f;
    g_quadMtrl.Specular.a = 1.0f;
    g_quadMtrl.Power = 40.0f;

    // Setup a simple directional light and some ambient
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

    // Any resources or settings that need to be restored after losing the 
    // DirectD device should probably be grouped together into one function so 
    // they can be re-created or reset in one call.
    restoreDeviceObjects();
}

//-----------------------------------------------------------------------------
// Name: restoreDeviceObjects()
// Desc: You are encouraged to develop applications with a single code path to 
//       respond to device loss. This code path is likely to be similar, if not 
//       identical, to the code path taken to initialize the device at startup.
//-----------------------------------------------------------------------------
HRESULT restoreDeviceObjects( void )
{
    //
    // Set some important state settings...
    //

    D3DXMATRIX matProj;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DXToRadian( 45.0f ),
                                640.0f / 480.0f,
                                //(float)(g_d3dpp.BackBufferWidth / g_d3dpp.BackBufferHeight), 
                                0.1f, 100.0f );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

    g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
    g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
    g_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE, TRUE );

    g_pd3dDevice->SetLight( 0, &g_pLight0 );
    g_pd3dDevice->LightEnable( 0, TRUE );

    g_pd3dDevice->SetRenderState( D3DRS_AMBIENT, D3DCOLOR_COLORVALUE( 0.2f, 0.2f, 0.2f, 1.0f ) );

    //
    // Create a texture object...
    //

    D3DXCreateTextureFromFile( g_pd3dDevice, "test.bmp", &g_pTexture );

    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );

    //
    // Create a vertex buffer...
    //
    // NOTE: When a device is lost, vertex buffers created using  
    // D3DPOOL_DEFAULT must be released properly before calling 
    // IDirect3DDevice9::Reset.
    //

    g_pd3dDevice->CreateVertexBuffer( 4*sizeof(QuadVertex),
                                      D3DUSAGE_WRITEONLY,
                                      QuadVertex::FVF_Flags,
                                      //D3DPOOL_MANAGED, // Does not have to be properly Released before calling IDirect3DDevice9::Reset
                                      D3DPOOL_DEFAULT,   // Must be Released properly before calling IDirect3DDevice9::Reset
                                      &g_pVertexBuffer, NULL );
    void *pVertices = NULL;

    g_pVertexBuffer->Lock( 0, sizeof(g_quadVertices), (void**)&pVertices, 0 );
    memcpy( pVertices, g_quadVertices, sizeof(g_quadVertices) );
    g_pVertexBuffer->Unlock();

    //
    // Create a mesh object...
    //
    // NOTE: When a device is lost, meshes created using D3DXMESH_DYNAMIC 
    // must be released properly before calling IDirect3DDevice9::Reset.
    //

    D3DXLoadMeshFromX( "teapot.x",
                       //D3DXMESH_SYSTEMMEM, // Does not have to be properly Released before calling IDirect3DDevice9::Reset
                       //D3DXMESH_MANAGED,   // Does not have to be properly Released before calling IDirect3DDevice9::Reset
                       //D3DXMESH_WRITEONLY, // Does not have to be properly Released before calling IDirect3DDevice9::Reset
                       D3DXMESH_DYNAMIC,     // Must be Released properly before calling IDirect3DDevice9::Reset
                       g_pd3dDevice,
                       NULL, NULL, NULL, NULL, &g_pTeapotMesh );
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: invalidateDeviceObjects()
// Desc: If the lost device can be restored, the application prepares the 
//       device by destroying all video-memory resources and any 
//       swap chains. This is typically accomplished by using the SAFE_RELEASE 
//       macro.
//-----------------------------------------------------------------------------
HRESULT invalidateDeviceObjects( void )
{
    //
    // To see how mismanagement of an object's reference counter can cause us
    // problems when calling Reset on the device, uncomment the line below.
    // The line below will call AddRef() on the vertex buffer object, which
    // will add one to the vertex buffer's reference count. This will cause it
    // to hang around after we call Release() on it, which is not what we 
    // wanted to happen here.
    //
    //g_pVertexBuffer->AddRef();

    //
    // NOTE: You could use the SAFE_RELEASE macro to invalidate your device 
    //       objects like so:
    //
    // SAFE_RELEASE( g_pTexture )
    // SAFE_RELEASE( g_pVertexBuffer )
    // SAFE_RELEASE( g_pTeapotMesh )
    //
    // But I've chosen a more verbose method (below) which I feel is better  
    // for understanding how the reference counting system can be used to
    // identify potential problems.
    //

    //
    // Invalidate the texture object...
    //

    if( g_pTexture != NULL )
    {
        int nNewRefCount = g_pTexture->Release();

        if( nNewRefCount > 0 )
        {
            static char strError[255];
            sprintf( strError, "The texture object failed to cleanup properly.\n"
                "Release() returned a reference count of %d", nNewRefCount );
            MessageBox( NULL, strError, "ERROR", MB_OK | MB_ICONEXCLAMATION );
        }

        g_pTexture = NULL;
    }

    //
    // Invalidate the vertex buffer object...
    //

    if( g_pVertexBuffer != NULL )
    {
        int nNewRefCount = g_pVertexBuffer->Release();

        if( nNewRefCount > 0 )
        {
            static char strError[255];
            sprintf( strError, "The vertex buffer object failed to cleanup properly.\n"
                "Release() returned a reference count of %d", nNewRefCount );
            MessageBox( NULL, strError, "ERROR", MB_OK | MB_ICONEXCLAMATION );
        }

        g_pVertexBuffer = NULL;
    }

    //
    // Invalidate the mesh object...
    //

    if( g_pTeapotMesh != NULL )
    {
        int nNewRefCount = g_pTeapotMesh->Release();

        if( nNewRefCount > 0 )
        {
            static char strError[255];
            sprintf( strError, "The mesh object failed to cleanup properly.\n"
                "Release() returned a reference count of %d", nNewRefCount );
            MessageBox( NULL, strError, "ERROR", MB_OK | MB_ICONEXCLAMATION );
        }

        g_pTeapotMesh = NULL;
    }

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )
{
    invalidateDeviceObjects();

    if( g_pd3dDevice != NULL )
    {
        int nNewRefCount = g_pd3dDevice->Release();

        if( nNewRefCount > 0 )
        {
            static char strError[255];
            sprintf( strError, "The device object failed to cleanup properly. \n"
                "Release() returned a reference count of %d", nNewRefCount );
            MessageBox( NULL, strError, "ERROR", MB_OK | MB_ICONEXCLAMATION );
        }

        g_pd3dDevice = NULL;
    }

    SAFE_RELEASE( g_pD3D )
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: 
//-----------------------------------------------------------------------------
void render( void )
{
    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                         D3DCOLOR_COLORVALUE(0.35f,0.53f,0.7f,1.0f), 1.0f, 0 );

    //
    // Render a teapot and a textured quad...
    //

    g_pd3dDevice->BeginScene();
    {
        D3DXMATRIX matView;
        D3DXMATRIX matWorld;
        D3DXMATRIX matRotation;
        D3DXMATRIX matTranslation;

        D3DXMatrixIdentity( &matView );
        g_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

        // Place and render first teapot...
        D3DXMatrixRotationYawPitchRoll( &matRotation, D3DXToRadian(g_fSpinX), D3DXToRadian(g_fSpinY), 0.0f );
        D3DXMatrixTranslation( &matTranslation, 1.4f, 0.0f, 6.0f );
        matWorld = matRotation * matTranslation;
        g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

        g_pd3dDevice->SetMaterial( &g_teapotMtrl );
        g_pTeapotMesh->DrawSubset(0);

        // Place and render textured quad...
        D3DXMatrixTranslation( &matTranslation, -1.6f, 0.0f, 6.0f );
        matWorld = matRotation * matTranslation;
        g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

        g_pd3dDevice->SetMaterial( &g_quadMtrl );
        g_pd3dDevice->SetTexture( 0, g_pTexture );
        g_pd3dDevice->SetStreamSource( 0, g_pVertexBuffer, 0, sizeof(QuadVertex) );
        g_pd3dDevice->SetFVF( QuadVertex::FVF_Flags );
        g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
    }
    g_pd3dDevice->EndScene();

    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}
