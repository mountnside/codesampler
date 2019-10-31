//-----------------------------------------------------------------------------
//           Name: dx9_novodex_simple.cpp
//         Author: Kevin Harris
//  Last Modified: 06/10/05
//    Description: This sample demonstrates how to use the NovodeX Physics SDK 
//                 (v2.2) by simply creating and dropping boxes on to a plane  
//                 that represents the ground.
//
//   Control Keys: Space      - Create and drop a single box
//                 F1         - Pause the simulation
//                 Up         - View moves forward
//                 Down       - View moves backward
//                 Left       - View strafes left
//                 Right      - View strafes Right
//                 Left Mouse - Perform looking
//                 Mouse      - Look about the scene
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX // Inhibit the macros: min(a,b) and max(a,b)

#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "resource.h"

#undef random
#include <NxPhysics.h>
#include "ErrorStream.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND              g_hWnd          = NULL;
LPDIRECT3D9       g_pD3D          = NULL;
LPDIRECT3DDEVICE9 g_pd3dDevice    = NULL;
LPD3DXMESH        g_pCubeMesh     = NULL;
LPD3DXMESH        g_pPlatformMesh = NULL;

POINT  g_ptLastMousePosit;
POINT  g_ptCurrentMousePosit;
bool   g_bMousing = false;
float  g_fMoveSpeed = 25.0f;
float  g_fElpasedTime;
double g_dCurTime;
double g_dLastTime;

D3DXVECTOR3 g_vEye(40.0f, 40.0f, -40.0f ); // Eye Position
D3DXVECTOR3 g_vLook(-0.5f,-0.3f,0.5f );    // Look Vector
D3DXVECTOR3 g_vUp(0.0f, 1.0f, 0.0f);       // Up Vector
D3DXVECTOR3 g_vRight(1.0f, 0.0f, 0.0f);    // Right Vector

// Physics
bool          g_bPause = false;
NxPhysicsSDK *g_pPhysicsSDK = NULL;
NxScene      *g_pScene = NULL;
NxVec3        g_vGravity( 0.0f, -9.81f, 0.0f );
ErrorStream   g_errorStream;

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
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void init(void);
void shutDown(void);
void render(void);
void initNovodeX(void);
void shutdownNovodeX(void);
void createBox(void);
void getRealTimeUserInput(void);
void updateViewMatrix(void);

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

    if( RegisterClassEx(&winClass) == 0 )
        return E_FAIL;

    g_hWnd = CreateWindowEx( NULL, "MY_WINDOWS_CLASS", 
                             "Direct3D (DX9) - Simple test of the NovodeX SDK",
                             WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                             0, 0, 640, 480, NULL, NULL, hInstance, NULL );

    if( g_hWnd == NULL )
        return E_FAIL;

    ShowWindow( g_hWnd, nCmdShow );
    UpdateWindow( g_hWnd );

    init();
    initNovodeX();
    
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

    shutdownNovodeX();
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

                case VK_SPACE:
                    createBox();
                    break;

                case VK_F1:
                    g_bPause = !g_bPause; 
                    break;
            }
        }
        break;

		case WM_LBUTTONDOWN:
		{
			g_bMousing = true;
		}
		break;

		case WM_LBUTTONUP:
		{
			g_bMousing = false;
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

    g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, 
                          D3DDEVTYPE_HAL,
                          g_hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                          &d3dpp, &g_pd3dDevice );

    D3DXMATRIX matProj;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DXToRadian( 45.0f ), 
                                640.0f / 480.0f, 0.1f, 1000.0f );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

    g_pd3dDevice->SetRenderState( D3DRS_NORMALIZENORMALS, TRUE );

    //
    // Setup a simple directional light and some ambient...
    //

    g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
    g_pd3dDevice->SetRenderState( D3DRS_AMBIENT, D3DCOLOR_COLORVALUE( 0.2f, 0.2f, 0.2f, 1.0f ) );

    D3DLIGHT9 light0;
    ZeroMemory( &light0, sizeof(D3DLIGHT9) );
    light0.Type = D3DLIGHT_DIRECTIONAL;
    light0.Direction = D3DXVECTOR3( 0.5f, -0.8f, 0.7f );

    light0.Diffuse.r = 1.0f;
    light0.Diffuse.g = 1.0f;
    light0.Diffuse.b = 1.0f;
    light0.Diffuse.a = 1.0f;

    g_pd3dDevice->SetLight( 0, &light0 );
    g_pd3dDevice->LightEnable( 0, TRUE );

    //
    // Set up a material...
    //

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

    //
    // Load up the cube mesh and then clone it so we can add per-vertex colors 
	// to it. Once per-vertex colors have been added, color the cube mesh red.
    //

    LPD3DXMESH pTempCubeMesh = NULL;
    LPDIRECT3DVERTEXBUFFER9 pTempVertexBuffer;

    D3DXLoadMeshFromX( "cube3.x", D3DXMESH_SYSTEMMEM, g_pd3dDevice,
                       NULL, NULL, NULL, NULL, &pTempCubeMesh );

    pTempCubeMesh->CloneMeshFVF( 0, Vertex::FVF_Flags, g_pd3dDevice, &g_pCubeMesh );

    if( SUCCEEDED( g_pCubeMesh->GetVertexBuffer( &pTempVertexBuffer ) ) )
    {
        int nNumVerts = g_pCubeMesh->GetNumVertices();
        Vertex *pVertices = NULL;

        pTempVertexBuffer->Lock( 0, 0, (void**)&pVertices, 0 );
        {
            // Make each vertex red...
            for( int i = 0; i < nNumVerts; ++i )
                pVertices[i].diffuse = D3DCOLOR_COLORVALUE( 1.0f, 0.0f, 0.0f, 1.0f );
        }
        pTempVertexBuffer->Unlock();

        pTempVertexBuffer->Release();
    }

	pTempCubeMesh->Release();
}

//-----------------------------------------------------------------------------
// Name: initNovodeX()
// Desc: Initialize the NovodeX Physics SDK.
//-----------------------------------------------------------------------------
void initNovodeX( void )
{
    // Initialize the NovodeX Physics SDK
    g_pPhysicsSDK = NxCreatePhysicsSDK( NX_PHYSICS_SDK_VERSION, 0, &g_errorStream );

    if( !g_pPhysicsSDK )
        return;

    // Create a scene
    NxSceneDesc sceneDesc;
    sceneDesc.gravity            = g_vGravity;
    sceneDesc.broadPhase         = NX_BROADPHASE_COHERENT;
    sceneDesc.collisionDetection = true;
    g_pScene = g_pPhysicsSDK->createScene( sceneDesc );

	NxMaterial *defaultMaterial = g_pScene->getMaterialFromIndex(0); 
	defaultMaterial->setRestitution(0.0f);
	defaultMaterial->setStaticFriction(0.5f);
	defaultMaterial->setDynamicFriction(0.5f);

    // Create a ground plane for our cubes to collide with.
    NxPlaneShapeDesc PlaneDesc;
    NxActorDesc ActorDesc;
    ActorDesc.shapes.pushBack( &PlaneDesc );
    g_pScene->createActor(ActorDesc);

	// Let's go ahead and drop one box to start off with...
	createBox();
}

//-----------------------------------------------------------------------------
// Name: shutdownNovodeX()
// Desc: Shutdown the NovodeX Physics SDK.
//-----------------------------------------------------------------------------
void shutdownNovodeX( void )
{
    if( g_pPhysicsSDK && g_pScene )   
        g_pPhysicsSDK->releaseScene( *g_pScene );

    g_pPhysicsSDK->release();
}

//-----------------------------------------------------------------------------
// Name: createBox()
// Desc: Creates and drops a single box into the physics scene.
//-----------------------------------------------------------------------------
void createBox( void )
{
    NxVec3 vPosition( 0.0f, 25.0f, 0.0f );
    int nSize = 1;

    // Create body
    NxBodyDesc BodyDesc;
    BodyDesc.angularDamping = 0.5f;

    NxBoxShapeDesc BoxDesc;
    BoxDesc.dimensions = NxVec3( float(nSize), float(nSize), float(nSize) );

    NxActorDesc ActorDesc;
    ActorDesc.shapes.pushBack( &BoxDesc );
    ActorDesc.body          = &BodyDesc;
    ActorDesc.density       = 10.0f;
    ActorDesc.globalPose.t  = vPosition;

    // Create the actor and use its userData pointer to store its size.
    g_pScene->createActor( ActorDesc )->userData = (void*)nSize;

    printf("Total: %d box-based actors\n", g_pScene->getNbActors());
}

//-----------------------------------------------------------------------------
// Name: getRealTimeUserInput()
// Desc: 
//-----------------------------------------------------------------------------
void getRealTimeUserInput( void )
{
	//
	// Get mouse input...
	//

	POINT mousePosit;
	GetCursorPos( &mousePosit );
	ScreenToClient( g_hWnd, &mousePosit );

	g_ptCurrentMousePosit.x = mousePosit.x;
	g_ptCurrentMousePosit.y = mousePosit.y;

	D3DXMATRIX matRotation;

	if( g_bMousing )
	{
		int nXDiff = (g_ptCurrentMousePosit.x - g_ptLastMousePosit.x);
		int nYDiff = (g_ptCurrentMousePosit.y - g_ptLastMousePosit.y);

		if( nYDiff != 0 )
		{
			D3DXMatrixRotationAxis( &matRotation, &g_vRight, D3DXToRadian((float)nYDiff / 3.0f));
			D3DXVec3TransformCoord( &g_vLook, &g_vLook, &matRotation );
			D3DXVec3TransformCoord( &g_vUp, &g_vUp, &matRotation );
		}

		if( nXDiff != 0 )
		{
			D3DXMatrixRotationAxis( &matRotation, &D3DXVECTOR3(0,1,0), D3DXToRadian((float)nXDiff / 3.0f) );
			D3DXVec3TransformCoord( &g_vLook, &g_vLook, &matRotation );
			D3DXVec3TransformCoord( &g_vUp, &g_vUp, &matRotation );
		}
	}

	g_ptLastMousePosit.x = g_ptCurrentMousePosit.x;
	g_ptLastMousePosit.y = g_ptCurrentMousePosit.y;

	//
	// Get keyboard input...
	//

	unsigned char keys[256];
	GetKeyboardState( keys );

	D3DXVECTOR3 tmpLook  = g_vLook;
	D3DXVECTOR3 tmpRight = g_vRight;

	// Up Arrow Key - View moves forward
	if( keys[VK_UP] & 0x80 )
		g_vEye -= tmpLook*-g_fMoveSpeed*g_fElpasedTime;

	// Down Arrow Key - View moves backward
	if( keys[VK_DOWN] & 0x80 )
		g_vEye += (tmpLook*-g_fMoveSpeed)*g_fElpasedTime;

	// Left Arrow Key - View side-steps or strafes to the left
	if( keys[VK_LEFT] & 0x80 )
		g_vEye -= (tmpRight*g_fMoveSpeed)*g_fElpasedTime;

	// Right Arrow Key - View side-steps or strafes to the right
	if( keys[VK_RIGHT] & 0x80 )
		g_vEye += (tmpRight*g_fMoveSpeed)*g_fElpasedTime;

	// Home Key - View elevates up
	if( keys[VK_HOME] & 0x80 )
		g_vEye.y += g_fMoveSpeed*g_fElpasedTime; 

	// End Key - View elevates down
	if( keys[VK_END] & 0x80 )
		g_vEye.y -= g_fMoveSpeed*g_fElpasedTime;
}

//-----------------------------------------------------------------------------
// Name : updateViewMatrix()
// Desc : Builds a view matrix suitable for Direct3D.
//
// Here's what the final matrix should look like:
//
//  |   rx     ux     lx    0 |
//  |   ry     uy     ly    0 |
//  |   rz     uz     lz    0 |
//  | -(r.e) -(u.e) -(l.e)  1 |
//
// Where r = Right vector
//       u = Up vector
//       l = Look vector
//       e = Eye position in world space
//       . = Dot-product operation
//
//-----------------------------------------------------------------------------
void updateViewMatrix( void )
{
	D3DXMATRIX view;
	D3DXMatrixIdentity( &view );

	D3DXVec3Normalize( &g_vLook, &g_vLook );
	D3DXVec3Cross( &g_vRight, &g_vUp, &g_vLook );
	D3DXVec3Normalize( &g_vRight, &g_vRight );
	D3DXVec3Cross( &g_vUp, &g_vLook, &g_vRight );
	D3DXVec3Normalize( &g_vUp, &g_vUp );

	view._11 = g_vRight.x;
	view._12 = g_vUp.x;
	view._13 = g_vLook.x;
	view._14 = 0.0f;

	view._21 = g_vRight.y;
	view._22 = g_vUp.y;
	view._23 = g_vLook.y;
	view._24 = 0.0f;

	view._31 = g_vRight.z;
	view._32 = g_vUp.z;
	view._33 = g_vLook.z;
	view._34 = 0.0f;

	view._41 = -D3DXVec3Dot( &g_vEye, &g_vRight );
	view._42 = -D3DXVec3Dot( &g_vEye, &g_vUp );
	view._43 = -D3DXVec3Dot( &g_vEye, &g_vLook );
	view._44 =  1.0f;

	g_pd3dDevice->SetTransform( D3DTS_VIEW, &view ); 
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )
{
    if( g_pd3dDevice != NULL )
        g_pd3dDevice->Release();

    if( g_pD3D != NULL )
        g_pD3D->Release();
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: 
//-----------------------------------------------------------------------------
void render( void )
{
    if( g_pScene && !g_bPause )
        g_pScene->simulate( 1.0f/60.0f );

    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                         D3DCOLOR_COLORVALUE( 0.35f, 0.53f, 0.7, 1.0f ), 1.0f, 0 );

    g_pd3dDevice->BeginScene();

	getRealTimeUserInput();
	updateViewMatrix();

    //
    // Now, keep physics & graphics in sync by rendering a solid cube for 
    // each cube represented by the physics engine.
    //

    int numActors = g_pScene->getNbActors();  // Get the total number of actors. 
    NxActor** actors = g_pScene->getActors(); // Get the actors as an array of actor pointers
    float d3dMat[16];

    D3DXMATRIX matWorld;
    D3DXMATRIX matScale;

    while( numActors-- )
    {
        NxActor* actor = *actors++;

        if( !actor->userData )
            continue;

        actor->getGlobalPose().getColumnMajor44( d3dMat );
        matWorld = d3dMat;

        // Scale it so it matches the NovodeX object
        D3DXMatrixScaling( &matScale, 
                           float(int(actor->userData))*2.0f, 
                           float(int(actor->userData))*2.0f, 
                           float(int(actor->userData))*2.0f);

        matWorld = matScale * matWorld;

        g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );
        g_pCubeMesh->DrawSubset(0);
    }

    g_pd3dDevice->EndScene();
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );

    if( g_pScene && !g_bPause )
    {
        g_pScene->flushStream();
        g_pScene->fetchResults( NX_RIGID_BODY_FINISHED, true );
    }
}
