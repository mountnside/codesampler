//-----------------------------------------------------------------------------
//           Name: dx9_tokamak_geometry.cpp
//         Author: Kevin Harris
//  Last Modified: 04/20/05
//    Description: This sample demonstrates how to perform collision detection 
//                 using the box, sphere, and cylinder geometry types of the 
//                 Tokamak SDK. To prevent confusion, keep in mind that 
//                 Tokamak’s usage of the word, "geometry" does not relate to 
//                 3D rendering. Tokamak’s box, sphere, and cylinder geometry 
//                 types are collision primitives. These primitive shapes which 
//                 can be used to either, calculate the exact collision and 
//                 response of a box, sphere, or a cylinder, or to roughly 
//                 approximate more complex shapes using them.
//
//                 For demonstration purposes, you can use the F1 key to lift 
//                 up on all the rigid bodies, which will obviously disturb 
//                 the stack. You can also use the F2 key to tilt the blue 
//                 platform and dump them over the side.
//
//   Control Keys: Up         - View moves forward
//                 Down       - View moves backward
//                 Left       - View strafes left
//                 Right      - View strafes Right
//                 Left Mouse - Perform looking
//                 Mouse      - Look about the scene
//                 F1         - Lift up on all rigid bodies
//                 F2         - Tilt the blue platform
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <mmsystem.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <tokamak.h>
#include "resource.h"

// This macro converts a Tokamak matrix (neT3) to a Direct3D matrix (D3DXMATRIX).
#define NET3_TO_D3DXMATRIX( tmat )                                 \
D3DXMATRIX( tmat.rot[0][0], tmat.rot[0][1], tmat.rot[0][2], 0.0f,  \
            tmat.rot[1][0], tmat.rot[1][1], tmat.rot[1][2], 0.0f,  \
            tmat.rot[2][0], tmat.rot[2][1], tmat.rot[2][2], 0.0f,  \
            tmat.pos[0],    tmat.pos[1],    tmat.pos[2],    1.0f );

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND              g_hWnd          = NULL;
LPDIRECT3D9       g_pD3D          = NULL;
LPDIRECT3DDEVICE9 g_pd3dDevice    = NULL;
LPD3DXMESH        g_pCubeMesh     = NULL;
LPD3DXMESH        g_pSphereMesh   = NULL;
LPD3DXMESH        g_pCylinderMesh = NULL;
LPD3DXMESH        g_pPlatformMesh = NULL;

// Tokamak related globals ----------------------------------------------------
LARGE_INTEGER g_performanceFrequency; // Used to store the performance frequency of the target box
LARGE_INTEGER g_startTime;            // Used to store the tick count at the beginning of each game loop
LARGE_INTEGER g_endTime;              // Used to store the tick count at the end of each game loop
float		  g_fFrameTime;           // Used to store the resulting time taken from start to end of each game loop (1 frame)
float		  g_fTimeSinceLastUpdate = 0.0f;
const float	  CONST_FRAME_INTERVAL = 0.016666f;

const int NUM_RIGID_BODIES = 3;
const int NUM_ANIMATED_BODIES = 1;

neSimulator    *g_simulator     = NULL;
neRigidBody    *g_redCube       = NULL;
neRigidBody    *g_yellowSphere  = NULL;
neRigidBody    *g_greenCylinder = NULL;
neAnimatedBody *g_bluePlatform  = NULL;

float g_fTiltOfBluePlatform = 0.0f;
//-----------------------------------------------------------------------------

POINT  g_ptLastMousePosit;
POINT  g_ptCurrentMousePosit;
bool   g_bMousing = false;
float  g_fMoveSpeed = 25.0f;
float  g_fElpasedTime;
double g_dCurTime;
double g_dLastTime;

D3DXVECTOR3	g_vEye(-8.5f, 1.8f, -11.1f); // Eye Position
D3DXVECTOR3	g_vLook(0.6f, 0.0f, 0.75f);  // Look Vector
D3DXVECTOR3	g_vUp(0.0f, 1.0f, 0.0f);    // Up Vector
D3DXVECTOR3	g_vRight(1.0f, 0.0f, 0.0f); // Right Vector

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
void getRealTimeUserInput(void);
void updateViewMatrix(void);
void initPhysics(void);
void updatePhysics(void);

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
                             "Direct3D (DX9) - Tokamak Geometry",
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
        case WM_KEYDOWN:
        {
            switch( wParam )
            {
                case VK_ESCAPE:
                    PostQuitMessage(0);
                    break;

                case VK_F1:
					// To mix things up a bit, pull up on our rigid bodies
					neV3 vel;
					vel.Set( 0.0f, 5.0f, -0.5f );
					g_redCube->SetVelocity( vel );
					g_yellowSphere->SetVelocity( vel );
					g_greenCylinder->SetVelocity( vel );
					break;

				case VK_F2:
					g_fTiltOfBluePlatform += 0.01f;
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
                                640.0f / 480.0f, 0.1f, 100.0f );
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

    LPD3DXMESH pTempMesh = NULL;
    LPDIRECT3DVERTEXBUFFER9 pTempVertexBuffer;

    D3DXLoadMeshFromX( "cube3.x", D3DXMESH_SYSTEMMEM, g_pd3dDevice, 
                       NULL, NULL, NULL, NULL, &pTempMesh );

    pTempMesh->CloneMeshFVF( 0, Vertex::FVF_Flags, g_pd3dDevice, &g_pCubeMesh );

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

    pTempMesh->Release();

	//
	// Clone the cylinder mesh and make it green...
	//

	D3DXLoadMeshFromX( "cylinder2.x", D3DXMESH_SYSTEMMEM, g_pd3dDevice, 
		               NULL, NULL, NULL, NULL, &pTempMesh );

	pTempMesh->CloneMeshFVF( 0, Vertex::FVF_Flags, g_pd3dDevice, &g_pCylinderMesh );

	if( SUCCEEDED( g_pCylinderMesh->GetVertexBuffer( &pTempVertexBuffer ) ) )
	{
		int nNumVerts = g_pCylinderMesh->GetNumVertices();
		Vertex *pVertices = NULL;

		pTempVertexBuffer->Lock( 0, 0, (void**)&pVertices, 0 );
		{
			// Make each vertex green...
			for( int i = 0; i < nNumVerts; ++i )
				pVertices[i].diffuse = D3DCOLOR_COLORVALUE( 0.0f, 1.0f, 0.0f, 1.0f );
		}
		pTempVertexBuffer->Unlock();

		pTempVertexBuffer->Release();
	}

	pTempMesh->Release();

	//
	// Clone the sphere mesh and make it yellow...
	//

	D3DXLoadMeshFromX( "sphere2.x", D3DXMESH_SYSTEMMEM, g_pd3dDevice, 
		               NULL, NULL, NULL, NULL, &pTempMesh );

	pTempMesh->CloneMeshFVF( 0, Vertex::FVF_Flags, g_pd3dDevice, &g_pSphereMesh );

	if( SUCCEEDED( g_pSphereMesh->GetVertexBuffer( &pTempVertexBuffer ) ) )
	{
		int nNumVerts = g_pSphereMesh->GetNumVertices();
		Vertex *pVertices = NULL;

		pTempVertexBuffer->Lock( 0, 0, (void**)&pVertices, 0 );
		{
			// Make each vertex yellow...
			for( int i = 0; i < nNumVerts; ++i )
				pVertices[i].diffuse = D3DCOLOR_COLORVALUE( 1.0f, 1.0f, 0.0f, 1.0f );
		}
		pTempVertexBuffer->Unlock();

		pTempVertexBuffer->Release();
	}

	pTempMesh->Release();

	//
	// Clone the cube mesh again and make it blue for our platfrom...
	//

	D3DXLoadMeshFromX( "cube3.x", D3DXMESH_SYSTEMMEM, g_pd3dDevice, 
		               NULL, NULL, NULL, NULL, &pTempMesh );

	pTempMesh->CloneMeshFVF( 0, Vertex::FVF_Flags, g_pd3dDevice, &g_pPlatformMesh );

	if( SUCCEEDED( g_pPlatformMesh->GetVertexBuffer( &pTempVertexBuffer ) ) )
	{
		int nNumVerts = g_pPlatformMesh->GetNumVertices();
		Vertex *pVertices = NULL;

		pTempVertexBuffer->Lock( 0, 0, (void**)&pVertices, 0 );
		{
			// Make each vertex blue...
			for( int i = 0; i < nNumVerts; ++i )
				pVertices[i].diffuse = D3DCOLOR_COLORVALUE( 0.0f, 0.0f, 1.0f, 1.0f );
		}
		pTempVertexBuffer->Unlock();

		pTempVertexBuffer->Release();
	}

	pTempMesh->Release();

	//
	// Init Tokamak based physics...
	//

    initPhysics();
}

//-----------------------------------------------------------------------------
// Name: initPhysics()
// Desc: 
//-----------------------------------------------------------------------------
void initPhysics( void )
{
    QueryPerformanceFrequency( &g_performanceFrequency );
    QueryPerformanceCounter( &g_startTime );

    //
    // First, create the simulator object...
    //
    // Note how sizeInfo is used to specify how many of each object the 
	// simulator will need to allocate.
    //

    neSimulatorSizeInfo sizeInfo;

    sizeInfo.rigidBodiesCount        = NUM_RIGID_BODIES;
    sizeInfo.animatedBodiesCount     = NUM_ANIMATED_BODIES;
    sizeInfo.rigidParticleCount      = 0;
    sizeInfo.controllersCount        = 0;
	// Use the following formula to compute the number of overlapped-pairs required: 
	//(num_rigid_bodies * num_animated_bodies) + num_rigid_bodies * (num_rigid_bodies - 1) / 2
	sizeInfo.overlappedPairsCount    = (NUM_RIGID_BODIES * NUM_ANIMATED_BODIES) + NUM_RIGID_BODIES * (NUM_RIGID_BODIES - 1) / 2;
    sizeInfo.geometriesCount         = NUM_RIGID_BODIES + NUM_ANIMATED_BODIES;
    sizeInfo.constraintsCount        = 0;
    sizeInfo.constraintSetsCount     = 0;
    sizeInfo.constraintBufferSize    = 0;
    sizeInfo.sensorsCount            = 0;
    sizeInfo.terrainNodesStartCount  = 0;
    sizeInfo.terrainNodesGrowByCount = 0;

    neV3 gravity;
    gravity.Set( 0.0f, -10.0f, 0.0f );

    g_simulator = neSimulator::CreateSimulator( sizeInfo, NULL, &gravity );

	//
	// Physics for the blue platform, which will support everything...
	//

	g_bluePlatform = g_simulator->CreateAnimatedBody();

	neGeometry *platformGeom = g_bluePlatform->AddGeometry();

	neV3 sizeOfPlatform;
	sizeOfPlatform.Set( 10.0f, 1.0f, 10.0f );
	platformGeom->SetBoxSize( sizeOfPlatform );

	g_bluePlatform->UpdateBoundingInfo();

	neV3 positionOfPlatform;
	positionOfPlatform.Set( 0.0f, 0.0f, 0.0f );
	g_bluePlatform->SetPos( positionOfPlatform );

	//
	// Physics for the green cylinder...
	//

	g_greenCylinder = g_simulator->CreateRigidBody();

	neGeometry *cylinderGeom = g_greenCylinder->AddGeometry();

	float cylinderDiameter = 1.0f;
	float cylinderHeight   = 1.0f;
	cylinderGeom->SetCylinder( cylinderDiameter, cylinderHeight );

	float massOfCylinder = 0.1f;
	g_greenCylinder->SetMass( massOfCylinder );
	g_greenCylinder->SetInertiaTensor( neCylinderInertiaTensor( cylinderDiameter, cylinderHeight, massOfCylinder ) );

	neV3 positionOfCylinder;
	positionOfCylinder.Set( 0.0f, 2.0f, 0.0f );
	g_greenCylinder->SetPos( positionOfCylinder );

	g_greenCylinder->UpdateBoundingInfo();

	//
	// Physics for the red cube...
	//

	g_redCube = g_simulator->CreateRigidBody();

	neGeometry *cubeGeom = g_redCube->AddGeometry();

	neV3 sizeOfCube;
	sizeOfCube.Set( 1.0f, 1.0f, 1.0f );
	cubeGeom->SetBoxSize( sizeOfCube );

	f32 massOfCube = 0.1f;
	g_redCube->SetMass( massOfCube );
	g_redCube->SetInertiaTensor( neBoxInertiaTensor( sizeOfCube, massOfCube ) );

	neV3 positionOfCube;
	positionOfCube.Set( 0.0f, 5.0f, 0.0f );
	g_redCube->SetPos( positionOfCube );

	g_redCube->UpdateBoundingInfo();

	//
	// Physics for the yellow sphere...
	//

	g_yellowSphere = g_simulator->CreateRigidBody();

	neGeometry *sphereGeom = g_yellowSphere->AddGeometry();

	float sphereDiameter = 1.0f;
	sphereGeom->SetSphereDiameter( sphereDiameter );

	float massOfSphere = 0.1f;
	g_yellowSphere->SetMass( massOfSphere );
	g_yellowSphere->SetInertiaTensor( neSphereInertiaTensor( sphereDiameter, massOfSphere ) );

	neV3 positionOfSphere;
	positionOfSphere.Set( 0.0f, 8.0f, 0.0f );
	g_yellowSphere->SetPos( positionOfSphere );

	g_yellowSphere->UpdateBoundingInfo();
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
    neSimulator::DestroySimulator( g_simulator );

	if( g_pCubeMesh != NULL )
		g_pCubeMesh->Release();

	if( g_pSphereMesh != NULL )
		g_pSphereMesh->Release();

	if( g_pCylinderMesh != NULL )
		g_pCylinderMesh->Release();

	if( g_pPlatformMesh != NULL )
		g_pPlatformMesh->Release();

    if( g_pd3dDevice != NULL )
        g_pd3dDevice->Release();

    if( g_pD3D != NULL )
        g_pD3D->Release();
}

//-----------------------------------------------------------------------------
// Name: updatePhysics()
// Desc: 
//-----------------------------------------------------------------------------
void updatePhysics( void )
{
    //
    // Before we advance the simulation, lets update the orientation of our  
    // blue platform just in case the user adjusted it.
    //

    neT3 rotMatrix;
    neV3 rotation;
    rotation.Set( 0.0f, 0.0f, g_fTiltOfBluePlatform );
    rotMatrix.rot.RotateXYZ( rotation );

    g_bluePlatform->SetRotation( rotMatrix.rot );

    //
    // Now, advance the simulation...
    //

    //
    // A note from the Tokamak developers:
    //
    // This sample advances the simulation by placing an upper limit on 
    // the speed with which the physics can update so that if the frame time 
    // is less than 1/60th, the physics is not updated. If you are running on a 
    // faster machine the physics rate will be the same to the observer.
    //
    // Unfortunately, a problem arises when the frame time is substantially 
    // more than 1/60th of a second. The risk here is that on a slow machine 
    // the physics will appreciably slow down. One approach to solving this is 
    // to have several different render calls of varying "costs". When you 
    // reach a physics spike you use one of the lower cost render calls, until 
    // the spike is passed. This requires a more sophisticated approach which 
    // involves keeping track of the physics cost as well as the render cost 
    // (not shown in this sample).
    //

    QueryPerformanceCounter( &g_endTime );

    // Calculate the elapsed time since the physics was last updated.
    g_fFrameTime = ( (float)g_endTime.QuadPart - (float)g_startTime.QuadPart ) 
                   / g_performanceFrequency.QuadPart;

    g_fTimeSinceLastUpdate += g_fFrameTime;

    // If the time is greater than 1/60th of a second update the physics
    if( g_fTimeSinceLastUpdate > CONST_FRAME_INTERVAL )
    {
        g_simulator->Advance( CONST_FRAME_INTERVAL, 1, NULL );
        g_fTimeSinceLastUpdate -= CONST_FRAME_INTERVAL;
    }

    QueryPerformanceCounter( &g_startTime );
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: 
//-----------------------------------------------------------------------------
void render( void )
{
    updatePhysics();

    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                         D3DCOLOR_COLORVALUE( 0.35f, 0.53f, 0.7, 1.0f ), 1.0f, 0 );

    g_pd3dDevice->BeginScene();

	getRealTimeUserInput();
	updateViewMatrix();

	D3DXMATRIX matWorld;
	D3DXMATRIX matScale;

	//
	// Render the blue platform...
	//

	matWorld = NET3_TO_D3DXMATRIX( g_bluePlatform->GetTransform() );

	D3DXMatrixScaling( &matScale, 10.0f, 1.0f, 10.0f ); // Scale it so it matches the Tokamak object
	matWorld = matScale * matWorld;
	g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

	g_pPlatformMesh->DrawSubset(0);

	//
	// Render the green cylinder...
	//

	matWorld = NET3_TO_D3DXMATRIX( g_greenCylinder->GetTransform() );

	D3DXMatrixScaling( &matScale, 1.0f, 2.0f, 1.0f ); // Scale it so it matches the Tokamak object
	matWorld = matScale * matWorld;

	g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );
	g_pCylinderMesh->DrawSubset(0);

	//
	// Render the red cube...
	//

	matWorld = NET3_TO_D3DXMATRIX( g_redCube->GetTransform() );

    g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );
    g_pCubeMesh->DrawSubset(0);
 
	//
	// Render the yellow sphere...
	//

	matWorld = NET3_TO_D3DXMATRIX( g_yellowSphere->GetTransform() );

	g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );
	g_pSphereMesh->DrawSubset(0);


	g_pd3dDevice->EndScene();
	g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}
