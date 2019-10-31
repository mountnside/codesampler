//-----------------------------------------------------------------------------
//           Name: ogl_novodex_simple.cpp
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
#include <GL/gl.h>
#include <GL/glu.h>
#include "geometry.h"
#include "matrix4x4f.h"
#include "vector3f.h"
#include "resource.h"

#undef random
#include <NxPhysics.h>
#include "ErrorStream.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND  g_hWnd = NULL;
HDC	  g_hDC  = NULL;
HGLRC g_hRC  = NULL;

POINT  g_ptLastMousePosit;
POINT  g_ptCurrentMousePosit;
bool   g_bMousing = false;
float  g_fMoveSpeed = 25.0f;
float  g_fElpasedTime;
double g_dCurTime;
double g_dLastTime;

vector3f g_vEye(40.0f, 40.0f, 40.0f ); // Eye Position
vector3f g_vLook(-0.5f,-0.3f,-0.5f );  // Look Vector
vector3f g_vUp(0.0f, 1.0f, 0.0f);      // Up Vector
vector3f g_vRight(1.0f, 0.0f, 0.0f);   // Right Vector

// Physics
bool          g_bPause = false;
NxPhysicsSDK *g_pPhysicsSDK = NULL;
NxScene      *g_pScene = NULL;
NxVec3        g_vGravity( 0.0f, -9.81f, 0.0f );
ErrorStream   g_errorStream;

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void init(void);
void render(void);
void shutDown(void);
void initNovodeX(void);
void shutdownNovodeX(void);
void createBox(void);
void getRealTimeUserInput(void);
void updateViewMatrix(void);

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
	winClass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	winClass.lpfnWndProc   = WindowProc;
	winClass.hInstance     = hInstance;
    winClass.hIcon	       = LoadIcon(hInstance, (LPCTSTR)IDI_OPENGL_ICON);
    winClass.hIconSm	   = LoadIcon(hInstance, (LPCTSTR)IDI_OPENGL_ICON);
	winClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	winClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	winClass.lpszMenuName  = NULL;
	winClass.cbClsExtra    = 0;
	winClass.cbWndExtra    = 0;
	
	if( !RegisterClassEx(&winClass) )
		return E_FAIL;

	g_hWnd = CreateWindowEx( NULL, "MY_WINDOWS_CLASS", 
                             "OpenGL - Simple test of the NovodeX SDK",
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

		case WM_SIZE:
		{
			int nWidth  = LOWORD(lParam); 
			int nHeight = HIWORD(lParam);
			glViewport(0, 0, nWidth, nHeight);

			glMatrixMode( GL_PROJECTION );
			glLoadIdentity();
			gluPerspective( 45.0, (GLdouble)nWidth / (GLdouble)nHeight, 0.1, 1000.0);
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

	matrix4x4f matRotation;

    if( g_bMousing )
    {
		int nXDiff = (g_ptCurrentMousePosit.x - g_ptLastMousePosit.x);
        int nYDiff = (g_ptCurrentMousePosit.y - g_ptLastMousePosit.y);
        
        if( nYDiff != 0 )
        {
            matRotation.rotate( -(float)nYDiff / 3.0f, g_vRight );
            matRotation.transformVector( &g_vLook );
            matRotation.transformVector( &g_vUp );
        }

        if( nXDiff != 0 )
        {
            matRotation.rotate( -(float)nXDiff / 3.0f, vector3f(0.0f, 1.0f, 0.0f) );
            matRotation.transformVector( &g_vLook );
            matRotation.transformVector( &g_vUp );
        }
    }

    g_ptLastMousePosit.x = g_ptCurrentMousePosit.x;
    g_ptLastMousePosit.y = g_ptCurrentMousePosit.y;

	//
	// Get keyboard input...
	//

	unsigned char keys[256];
	GetKeyboardState( keys );

	vector3f tmpLook  = g_vLook;
	vector3f tmpRight = g_vRight;

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
// Name: init()
// Desc: 
//-----------------------------------------------------------------------------
void init( void )
{
	GLuint PixelFormat;

	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

    pfd.nSize      = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion   = 1;
    pfd.dwFlags    = PFD_DRAW_TO_WINDOW |PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 16;
    pfd.cDepthBits = 16;
	
	g_hDC = GetDC( g_hWnd );
	PixelFormat = ChoosePixelFormat( g_hDC, &pfd );
	SetPixelFormat( g_hDC, PixelFormat, &pfd);
	g_hRC = wglCreateContext( g_hDC );
	wglMakeCurrent( g_hDC, g_hRC );

    // Setup some default render states for OpenGL
    glClearColor( 0.35f, 0.53f, 0.7f, 1.0f );
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_CULL_FACE );

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective( 45.0f, 640.0f / 480.0f, 0.1f, 1000.0f);

    // Setup lighting
    glEnable( GL_LIGHTING );
    glEnable( GL_LIGHT0 );

    float fAmbientColor[] = { 0.2f, 0.2f, 0.2f, 0.0f };       
    glLightfv( GL_LIGHT0, GL_AMBIENT, fAmbientColor );

    float fDiffuseColor[] = { 1.0f, 1.0f, 1.0f, 0.0f };       
    glLightfv( GL_LIGHT0, GL_DIFFUSE, fDiffuseColor );

    float fSpecularColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };       
    glLightfv( GL_LIGHT0, GL_SPECULAR, fSpecularColor );

    float fPosition[] = { 50.0f, 50.0f, 100.0f, 1.0f };     
    glLightfv( GL_LIGHT0, GL_POSITION, fPosition );

	// For lighting, pull our material colors from the vertex color...
	glEnable( GL_COLOR_MATERIAL );
	glColorMaterial( GL_FRONT, GL_AMBIENT_AND_DIFFUSE );
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )	
{
	if( g_hRC != NULL )
	{
		wglMakeCurrent( NULL, NULL );
		wglDeleteContext( g_hRC );
		g_hRC = NULL;
	}

	if( g_hDC != NULL )
	{
		ReleaseDC( g_hWnd, g_hDC );
		g_hDC = NULL;
	}
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
// Name : updateViewMatrix() 
// Desc : Builds a view matrix suitable for OpenGL.
//
// Here's what the final view matrix should look like:
//
//  |  rx   ry   rz  -(r.e) |
//  |  ux   uy   uz  -(u.e) |
//  | -lx  -ly  -lz   (l.e) |
//  |   0    0    0     1   |
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
	matrix4x4f view;
	view.identity();

	g_vLook.normalize();

	g_vRight = crossProduct(g_vLook, g_vUp);
	g_vRight.normalize();

	g_vUp = crossProduct(g_vRight, g_vLook);
	g_vUp.normalize();

	view.m[0] =  g_vRight.x;
	view.m[1] =  g_vUp.x;
	view.m[2] = -g_vLook.x;
	view.m[3] =  0.0f;

	view.m[4] =  g_vRight.y;
	view.m[5] =  g_vUp.y;
	view.m[6] = -g_vLook.y;
	view.m[7] =  0.0f;

	view.m[8]  =  g_vRight.z;
	view.m[9]  =  g_vUp.z;
	view.m[10] = -g_vLook.z;
	view.m[11] =  0.0f;

	view.m[12] = -dotProduct(g_vRight, g_vEye);
	view.m[13] = -dotProduct(g_vUp, g_vEye);
	view.m[14] =  dotProduct(g_vLook, g_vEye);
	view.m[15] =  1.0f;

	glMultMatrixf( view.m );
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: 
//-----------------------------------------------------------------------------
void render( void )
{
    if( g_pScene && !g_bPause )
        g_pScene->simulate( 1.0f/60.0f );

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	getRealTimeUserInput();

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
	updateViewMatrix();

    //
    // Now, keep physics & graphics in sync by rendering a solid cube for 
    // each cube represented by the physics engine.
    //

    int numActors = g_pScene->getNbActors();  // Get the total number of actors. 
    NxActor** actors = g_pScene->getActors(); // Get the actors as an array of actor pointers
    float glmat[16];

    glColor3f( 0.9f, 0.2f, 0.2f );

    while( numActors-- )
    {
        NxActor* actor = *actors++;

        if( !actor->userData )
            continue;

        glPushMatrix();
        {
            actor->getGlobalPose().getColumnMajor44( glmat );
            glMultMatrixf( glmat );
            renderSolidCube( float(int(actor->userData))*2.0f );
        }
        glPopMatrix();
    }

	SwapBuffers( g_hDC );

    if( g_pScene && !g_bPause )
    {
        g_pScene->flushStream();
        g_pScene->fetchResults( NX_RIGID_BODY_FINISHED, true );
    }
}
