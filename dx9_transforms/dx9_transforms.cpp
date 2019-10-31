//-----------------------------------------------------------------------------
//           Name: dx9_transforms.cpp
//         Author: Kevin Harris
//  Last Modified: 06/28/05
//    Description: Demonstrates how to use translation, rotation, and scaling 
//                 matrices to create a simulated solar system.
//
//   Control Keys: F1    - Speed up rotations
//                 F2    - Slow down rotations
//                 Space - Toggle orbiting on/off
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <mmsystem.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "resource.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND              g_hWnd        = NULL;
LPDIRECT3D9       g_pD3D        = NULL;
LPDIRECT3DDEVICE9 g_pd3dDevice  = NULL;
LPD3DXMESH        g_pSunMesh    = NULL;
LPD3DXMESH        g_pEarthMesh  = NULL;
LPD3DXMESH        g_pMoonMesh   = NULL;
LPD3DXMATRIXSTACK g_matrixStack = NULL;

float  g_fElpasedTime;
double g_dCurrentTime;
double g_dLastTime;

float g_fSpeedmodifier = 1.0f;
bool  g_bOrbitOn = true;

#define D3DFVF_MY_VERTEX ( D3DFVF_XYZ | D3DFVF_DIFFUSE )

struct Vertex
{
    float x, y, z; // Position of vertex in 3D space
    DWORD diffuse; // Diffuse color of vertex
};

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void init(void);
void shutDown(void);
void render(void);

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
                             "Direct3D (DX9) - Transforms",
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
            g_dCurrentTime = timeGetTime();
            g_fElpasedTime = (float)((g_dCurrentTime - g_dLastTime) * 0.001);
            g_dLastTime    = g_dCurrentTime;

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
    switch( msg )
    {   
        case WM_KEYDOWN:
        {
            switch( wParam )
            {
                case VK_ESCAPE:
                    PostQuitMessage(0);
                    break;

                case VK_SPACE:
                    g_bOrbitOn = !g_bOrbitOn;
                    break;

                case VK_F1:
                    ++g_fSpeedmodifier;
                    break;

                case VK_F2:
                    --g_fSpeedmodifier;
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
// Name: init()
// Desc: Initializes Direct3D under DirectX 9.0
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

    g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
    g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    g_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);

    D3DXMATRIX mProjection;
    D3DXMatrixPerspectiveFovLH( &mProjection, D3DXToRadian( 45.0f ), 
                                640.0f / 480.0f, 1.0f, 100.0f );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &mProjection );

    //
    // We'll use the D3DXCreateSphere utility function to create three simple 
    // sphere meshes to experiment with.
    //

    LPD3DXMESH pTempEarthMesh = NULL;
    LPD3DXMESH pTempSunMesh   = NULL;

    D3DXCreateSphere(g_pd3dDevice, 1.0f, 20, 20, &pTempSunMesh, NULL);
    D3DXCreateSphere(g_pd3dDevice, 1.0f, 10, 10, &pTempEarthMesh, NULL);
    D3DXCreateSphere(g_pd3dDevice, 0.5f, 8, 8, &g_pMoonMesh, NULL);

    //
    // Unfortunately, the D3DXCreateSphere utility function creates a mesh 
    // with no color, so we'll need to make a clone of the original meshes 
    // using a FVF code that does include color so we can set up the Earth 
    // and Sun with color.
    //
    // Once that's been done, we'll need to set the color values to something 
    // appropriate for our solar system model.
    //

    //
    // Clone the original Earth mesh and make it blue...
    //

    LPDIRECT3DVERTEXBUFFER9 pTempVertexBuffer;

    pTempEarthMesh->CloneMeshFVF( 0, D3DFVF_MY_VERTEX, g_pd3dDevice, &g_pEarthMesh );

    if( SUCCEEDED( g_pEarthMesh->GetVertexBuffer( &pTempVertexBuffer ) ) )
    {
        int nNumVerts = g_pEarthMesh->GetNumVertices();
        Vertex *pVertices = NULL;

        pTempVertexBuffer->Lock( 0, 0, (void**)&pVertices, 0 );
        {
            for( int i = 0; i < nNumVerts; ++i )
                pVertices[i].diffuse = D3DCOLOR_COLORVALUE( 0.0, 0.0, 1.0, 1.0 );
        }
        pTempVertexBuffer->Unlock();

        pTempVertexBuffer->Release();
    }

    //
    // Clone the original Sun mesh and make it yellow...
    //

    pTempSunMesh->CloneMeshFVF( 0, D3DFVF_MY_VERTEX, g_pd3dDevice, &g_pSunMesh );
    
    if( SUCCEEDED( g_pSunMesh->GetVertexBuffer( &pTempVertexBuffer ) ) )
    {
        int nNumVerts = g_pSunMesh->GetNumVertices();
        Vertex *pVertices = NULL;

        pTempVertexBuffer->Lock( 0, 0, (void**)&pVertices, 0 );
        {
            for( int i = 0; i < nNumVerts; ++i )
                pVertices[i].diffuse = D3DCOLOR_COLORVALUE( 1.0, 1.0, 0.0, 1.0 );
        }
        pTempVertexBuffer->Unlock();

        pTempVertexBuffer->Release();
    }

    //
    // The temp meshes are no longer needed...
    //

    pTempEarthMesh->Release();
    pTempSunMesh->Release();

    //
    // Create a matrix stack...
    //

    D3DXCreateMatrixStack( 0, &g_matrixStack );

}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: Release all Direct3D resources.
//-----------------------------------------------------------------------------
void shutDown( void )
{
    if( g_matrixStack != NULL )
        g_matrixStack->Release();

    if( g_pSunMesh != NULL )
        g_pSunMesh->Release();

    if( g_pEarthMesh != NULL )
        g_pEarthMesh->Release();

    if( g_pd3dDevice != NULL )
        g_pd3dDevice->Release();

    if( g_pD3D != NULL )
        g_pD3D->Release();
}

//*
//-----------------------------------------------------------------------------
// Name: render()
// Desc: Render a solar system using the D3DXMATRIX utility class
//-----------------------------------------------------------------------------
void render( void )
{
    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                         D3DCOLOR_COLORVALUE(0.0f,0.0f,0.0f,1.0f), 1.0f, 0 );

    g_pd3dDevice->BeginScene();

    //
    // Have the view matrix move the view move us to a good vantage point so 
    // we can see the Sun sitting at the origin while the Earth orbits it.
    //

    D3DXMATRIX matView;
    D3DXMatrixLookAtLH( &matView, &D3DXVECTOR3( 0.0f, 2.0f, -25.0f ), // Camera position
                                  &D3DXVECTOR3( 0.0f, 0.0f, 0.0f ),   // Look-at point
                                  &D3DXVECTOR3( 0.0f, 1.0f, 0.0f ) ); // Up vector

    g_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

    //
    // Cache rotational positions between frames...
    //

    static float fSunSpin    = 0.0f;    
    static float fEarthSpin  = 0.0f;    
    static float fEarthOrbit = 0.0f;
    static float fMoonSpin   = 0.0f;
    static float fMoonOrbit  = 0.0f;

    if( g_bOrbitOn == true )
    {
        fSunSpin += g_fSpeedmodifier * (g_fElpasedTime * 10.0f);

        fEarthSpin  += g_fSpeedmodifier * (g_fElpasedTime * 100.0f);
        fEarthOrbit += g_fSpeedmodifier * (g_fElpasedTime * 20.0f);

        fMoonSpin  += g_fSpeedmodifier * (g_fElpasedTime * 50.0f);
        fMoonOrbit += g_fSpeedmodifier * (g_fElpasedTime * 200.0f);
    }

    //
    // The Sun is easy because the mesh for it is initially created centered  
    // at origin. All we have to do is spin it by rotating it about the Y axis
    // and scale it by 5.0f.
    //

    D3DXMATRIX mSunScale;
    D3DXMATRIX mSunSpinRotation;
    D3DXMATRIX mSunMatrix;
    
    D3DXMatrixRotationY( &mSunSpinRotation, D3DXToRadian( fSunSpin ) );
    D3DXMatrixScaling( &mSunScale, 5.0f, 5.0f, 5.0f );
    
    // Now, concatenate them together...

    mSunMatrix = mSunScale *       // 1. Uniformly scale the Sun up in size
                 mSunSpinRotation; // 2. and then spin it on its axis.

    g_pd3dDevice->SetTransform( D3DTS_WORLD, &mSunMatrix );
    g_pSunMesh->DrawSubset(0);

    //
    // The Earth is a little more complicated since it needs to spin as well 
    // as orbit the Sun. This can be done by combining three transformations 
    // together.
    //
    
    D3DXMATRIX mEarthTranslationToOrbit;
    D3DXMATRIX mEarthSpinRotation;
    D3DXMATRIX mEarthOrbitRotation;
    D3DXMATRIX mEarthMatrix;

    D3DXMatrixRotationY( &mEarthSpinRotation, D3DXToRadian( fEarthSpin ) );
    D3DXMatrixTranslation( &mEarthTranslationToOrbit, 0.0f, 0.0f, 12.0f );
    D3DXMatrixRotationY( &mEarthOrbitRotation, D3DXToRadian( fEarthOrbit ) );

    // Now, concatenate them together...

    mEarthMatrix = mEarthSpinRotation *       // 1. Spin the Earth on its own axis.
                   mEarthTranslationToOrbit * // 2. Then translate it away from the origin (where the Sun's at)
                   mEarthOrbitRotation;       // 3. and rotate it again to make it orbit the origin (or the Sun).

    g_pd3dDevice->SetTransform( D3DTS_WORLD, &mEarthMatrix );
    g_pEarthMesh->DrawSubset(0);

    //
    // The Moon is the hardest to understand since it needs to not only spin on
    // its own axis and orbit the Earth, but needs to follow the Earth, 
    // which is orbiting the Sun.
    //
    // This can be done by combining five transformations together with the last
    // two being borrowed from the Earth's transformation.
    //

    D3DXMATRIX mMoonTranslationToOrbit;
    D3DXMATRIX mMoonSpinRotation;
    D3DXMATRIX mMoonOrbitRotation;
    D3DXMATRIX mMoonMatrix;

    D3DXMatrixRotationY( &mMoonSpinRotation, D3DXToRadian( fMoonSpin ) );
    D3DXMatrixRotationY( &mMoonOrbitRotation, D3DXToRadian( fMoonOrbit ) );
    D3DXMatrixTranslation( &mMoonTranslationToOrbit, 0.0f, 0.0f, 2.0f );

    //
    // The key to understanding the first three transforms is to pretend that 
    // the Earth is located at the origin. We know it's not, but if we pretend 
    // that it is, we can set up the Moon just like the we did the Earth since 
    // the Moon orbits the Earth just like the Earth orbits the Sun.
    //
    // Once the Moon's transforms are set up we simply reuse the Earth's 
    // translation and rotation matrix, which placed it in orbit, to offset
    // the Moon out to where it should be... following the Earth.
    // 

    // Now, concatenate them together...
    
    mMoonMatrix = mMoonSpinRotation *        // 1. Spin the Moon on its own axis.
                  mMoonTranslationToOrbit *  // 2. Then translate it away from the origin (pretending that the Earth is there)
                  mMoonOrbitRotation *       // 3. and rotate it again to make it orbit the origin (or the pretend Earth).
                  
                  mEarthTranslationToOrbit * // 4. Now, translate out to where the Earth is really at
                  mEarthOrbitRotation;       // 5. and move with it by matching its orbit of the Earth.

    g_pd3dDevice->SetTransform( D3DTS_WORLD, &mMoonMatrix );
    g_pMoonMesh->DrawSubset(0);

    g_pd3dDevice->EndScene();
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}
//*/




/*
//-----------------------------------------------------------------------------
// Name: render()
// Desc: Render a solar system using the D3DXMATRIX utility class and a matrix 
//       stack similar to OpenGL's. See the note below for details.
//       
// Note:
//
// Direct3D uses the world and view matrices that we set to configure several 
// internal data structures. Each time we set a new world or view matrix, the 
// system is forced to recalculate these internal structures. Therefore, 
// setting these matrices frequently, which is the case for applications that 
// require a high frame-rate, is computationally expensive. We can minimize 
// the number of required calculations by concatenating our world and view 
// matrices into a combined world-view matrix that we set as the world matrix. 
//
// With the view matrix combined in with each world matrix that we set, we no 
// longer have to set the view matrix separately and incur its overhead. 
// Instead, we simply set the view matrix to the identity once and leave it 
// untouched during all calculations.
//
// For clarity, Direct3D samples rarely employ this optimization since it 
// confuses beginners.
//
//-----------------------------------------------------------------------------
void render( void )
{
    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                         D3DCOLOR_COLORVALUE(0.0f,0.0f,0.0f,1.0f), 1.0f, 0 );

    g_pd3dDevice->BeginScene();

    //
    // Have the view matrix move the view move us to a good vantage point so 
    // we can see the Sun sitting at the origin while the Earth orbits it.
    //

    D3DXMATRIX matView;
    D3DXMatrixLookAtLH( &matView, &D3DXVECTOR3( 0.0f, 2.0f, -25.0f ), // Camera position
                                  &D3DXVECTOR3( 0.0f, 0.0f, 0.0f ),   // Look-at point
                                  &D3DXVECTOR3( 0.0f, 1.0f, 0.0f ) ); // Up vector

    g_matrixStack->LoadIdentity();
    g_matrixStack->LoadMatrix( &matView );

    //
    // Cache rotational positions between frames...
    //

    static float fSunSpin    = 0.0f;    
    static float fEarthSpin  = 0.0f;    
    static float fEarthOrbit = 0.0f;
    static float fMoonSpin   = 0.0f;
    static float fMoonOrbit  = 0.0f;

    if( g_bOrbitOn == true )
    {
        fSunSpin += g_fSpeedmodifier * (g_fElpasedTime * 10.0f);

        fEarthSpin  += g_fSpeedmodifier * (g_fElpasedTime * 100.0f);
        fEarthOrbit += g_fSpeedmodifier * (g_fElpasedTime * 20.0f);

        fMoonSpin  += g_fSpeedmodifier * (g_fElpasedTime * 50.0f);
        fMoonOrbit += g_fSpeedmodifier * (g_fElpasedTime * 200.0f);
    }

    //
    // The Sun is easy because the mesh for it is initially created centered  
    // at origin. All we have to do is spin it by rotating it about the Y axis
    // and scale it by 5.0f.
    //

    D3DXMATRIX mSunScale;
    D3DXMATRIX mSunSpinRotation;
    D3DXMATRIX mSunMatrix;
    
    D3DXMatrixRotationY( &mSunSpinRotation, D3DXToRadian( fSunSpin ) );
    D3DXMatrixScaling( &mSunScale, 5.0f, 5.0f, 5.0f );
    
    // Now, concatenate them together...

    mSunMatrix = mSunScale *       // 1. Uniformaly scale the Sun up in size
                 mSunSpinRotation; // 2. and then spin it on its axis.

    g_matrixStack->Push();
    {
        g_matrixStack->MultMatrixLocal( &mSunMatrix );

        g_pd3dDevice->SetTransform( D3DTS_WORLD, g_matrixStack->GetTop() );
        g_pSunMesh->DrawSubset(0);
    }
    g_matrixStack->Pop();

    //
    // The Earth is a little more complicated since it needs to spin as well 
    // as orbit the Sun. This can be done by combining three transformations 
    // together.
    //
    
    D3DXMATRIX mEarthTranslationToOrbit;
    D3DXMATRIX mEarthSpinRotation;
    D3DXMATRIX mEarthOrbitRotation;
    D3DXMATRIX mEarthMatrix;

    D3DXMatrixRotationY( &mEarthSpinRotation, D3DXToRadian( fEarthSpin ) );
    D3DXMatrixTranslation( &mEarthTranslationToOrbit, 0.0f, 0.0f, 12.0f );
    D3DXMatrixRotationY( &mEarthOrbitRotation, D3DXToRadian( fEarthOrbit ) );

    // Now, concatenate them together...

    mEarthMatrix = mEarthSpinRotation *       // 1. Spin the Earth on its own axis.
                   mEarthTranslationToOrbit * // 2. Then translate it away from the origin (where the Sun's at)
                   mEarthOrbitRotation;       // 3. and rotate it again to make it orbit the origin (or the Sun).
    
    g_matrixStack->Push();
    {
        g_matrixStack->MultMatrixLocal( &mEarthMatrix );

        g_pd3dDevice->SetTransform( D3DTS_WORLD, g_matrixStack->GetTop() );
        g_pEarthMesh->DrawSubset(0);
    }
    g_matrixStack->Pop();

    //
    // The Moon is the hardest to understand since it needs to not only spin on
    // its own axis and orbit the Earth, but needs to follow the Earth, 
    // which is orbiting the Sun.
    //
    // This can be done by combining five transformations together with the last
    // two being borrowed from the Earth's transformation.
    //

    D3DXMATRIX mMoonTranslationToOrbit;
    D3DXMATRIX mMoonSpinRotation;
    D3DXMATRIX mMoonOrbitRotation;
    D3DXMATRIX mMoonMatrix;

    D3DXMatrixRotationY( &mMoonSpinRotation, D3DXToRadian( fMoonSpin ) );
    D3DXMatrixRotationY( &mMoonOrbitRotation, D3DXToRadian( fMoonOrbit ) );
    D3DXMatrixTranslation( &mMoonTranslationToOrbit, 0.0f, 0.0f, 2.0f );

    //
    // The key to understanding the first three transforms is to pretend that 
    // the Earth is located at the origin. We know it's not, but if we pretend 
    // that it is, we can set up the Moon just like the we did the Earth since 
    // the Moon orbits the Earth just like the Earth orbits the Sun.
    //
    // Once the Moon's transforms are set up we simply reuse the Earth's 
    // translation and rotation matrix, which placed it in orbit, to offset  
    // the Moon out to where it should be... following the Earth.
    // 

    // Now, concatenate them together...
    
    mMoonMatrix = mMoonSpinRotation *        // 1. Spin the Moon on its own axis.
                  mMoonTranslationToOrbit *  // 2. Then translate it away from the origin (pretending that the Earth is there)
                  mMoonOrbitRotation *       // 3. and rotate it again to make it orbit the origin (or the pretend Earth).
                  
                  mEarthTranslationToOrbit * // 4. Now, translate out to where the Earth is really at
                  mEarthOrbitRotation;       // 5. and move with it by matching its orbit of the Earth.
    
    g_matrixStack->Push();
    {
        g_matrixStack->MultMatrixLocal( &mMoonMatrix );

        g_pd3dDevice->SetTransform( D3DTS_WORLD, g_matrixStack->GetTop() );
        g_pMoonMesh->DrawSubset(0);
    }
    g_matrixStack->Pop();

    g_pd3dDevice->EndScene();
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}
//*/
