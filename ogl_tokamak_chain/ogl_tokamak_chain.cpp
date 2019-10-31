//-----------------------------------------------------------------------------
//           Name: ogl_tokamak_chain.cpp
//         Author: Kevin Harris
//  Last Modified: 04/28/05
//    Description: This sample demonstrates how to use the Tokamak Physics SDK 
//                 to create a chain or rope like object by using small 
//                 collision cubes joined by ball-joints. 
//
//                 For demonstration purposes, the chain is dropped onto a
//                 platform, which doesn’t quite catch it all. This allows 
//                 the chain to hang off and swing about. For fun, you can use 
//                 the F1 key to lift up on the chain's last segment by 
//                 applying some velocity to it.
//
//   Control Keys: Up         - View moves forward
//                 Down       - View moves backward
//                 Left       - View strafes left
//                 Right      - View strafes Right
//                 Left Mouse - Perform looking
//                 Mouse      - Look about the scene
//                 F1         - Lift up on the chain's last segment by applying 
//                              some velocity to it
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <mmsystem.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glaux.h>
#include <tokamak.h>
#include "resource.h"
#include "matrix4x4f.h"
#include "vector3f.h"

// This macro converts a Tokamak matrix (neT3) to my custom matrix type (matrix4x4f).
#define NET3_TO_MATRIX4X4f( tmat )                                        \
matrix4x4f( tmat.rot[0][0], tmat.rot[1][0], tmat.rot[2][0], tmat.pos[0],  \
            tmat.rot[0][1], tmat.rot[1][1], tmat.rot[2][1], tmat.pos[1],  \
            tmat.rot[0][2], tmat.rot[1][2], tmat.rot[2][2], tmat.pos[2],  \
            0.0f,           0.0f,           0.0f,           1.0f           );

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND  g_hWnd = NULL;
HDC	  g_hDC  = NULL;
HGLRC g_hRC  = NULL;

// Tokamak related globals ----------------------------------------------------
LARGE_INTEGER g_performanceFrequency; // Used to store the performance frequency of the target box
LARGE_INTEGER g_startTime;            // Used to store the tick count at the beginning of each game loop
LARGE_INTEGER g_endTime;              // Used to store the tick count at the end of each game loop
float		  g_fFrameTime;           // Used to store the resulting time taken from start to end of each game loop (1 frame)
float		  g_fTimeSinceLastUpdate = 0.0f;
const float	  CONST_FRAME_INTERVAL = 0.016666f;

const int NUM_RED_CUBES = 30;
const int NUM_BLUE_PLATFORMS = 1;

neSimulator    *g_simulator = NULL;
neRigidBody    *g_redCubes[NUM_RED_CUBES];
neAnimatedBody *g_bluePlatform;
//-----------------------------------------------------------------------------

POINT  g_ptLastMousePosit;
POINT  g_ptCurrentMousePosit;
bool   g_bMousing = false;
float  g_fMoveSpeed = 25.0f;
float  g_fElpasedTime;
double g_dCurTime;
double g_dLastTime;

vector3f g_vEye(-48.0f, 11.7f, 46.0f); // Eye Position
vector3f g_vLook(0.6f, -0.2f, -0.6f);  // Look Vector
vector3f g_vUp(0.0f, 1.0f, 0.0f);      // Up Vector
vector3f g_vRight(1.0f, 0.0f, 0.0f);   // Right Vector

struct Vertex
{
	// GL_N3F_V3F
	float nx, ny, nz;
	float x, y, z;
};

Vertex g_cubeVertices[] =
{
	{ 0.0f, 0.0f, 1.0f,  -0.5f,-0.5f, 0.5f },
	{ 0.0f, 0.0f, 1.0f,   0.5f,-0.5f, 0.5f },
	{ 0.0f, 0.0f, 1.0f,   0.5f, 0.5f, 0.5f },
	{ 0.0f, 0.0f, 1.0f,  -0.5f, 0.5f, 0.5f },

	{ 0.0f, 0.0f,-1.0f,  -0.5f,-0.5f,-0.5f },
	{ 0.0f, 0.0f,-1.0f,  -0.5f, 0.5f,-0.5f },
	{ 0.0f, 0.0f,-1.0f,   0.5f, 0.5f,-0.5f },
	{ 0.0f, 0.0f,-1.0f,   0.5f,-0.5f,-0.5f },

	{ 0.0f, 1.0f, 0.0f,  -0.5f, 0.5f,-0.5f },
	{ 0.0f, 1.0f, 0.0f,  -0.5f, 0.5f, 0.5f },
	{ 0.0f, 1.0f, 0.0f,   0.5f, 0.5f, 0.5f },
	{ 0.0f, 1.0f, 0.0f,   0.5f, 0.5f,-0.5f },

	{ 0.0f,-1.0f, 0.0f,  -0.5f,-0.5f,-0.5f },
	{ 0.0f,-1.0f, 0.0f,   0.5f,-0.5f,-0.5f },
	{ 0.0f,-1.0f, 0.0f,   0.5f,-0.5f, 0.5f },
	{ 0.0f,-1.0f, 0.0f,  -0.5f,-0.5f, 0.5f },

	{ 1.0f, 0.0f, 0.0f,   0.5f,-0.5f,-0.5f },
	{ 1.0f, 0.0f, 0.0f,   0.5f, 0.5f,-0.5f },
	{ 1.0f, 0.0f, 0.0f,   0.5f, 0.5f, 0.5f },
	{ 1.0f, 0.0f, 0.0f,   0.5f,-0.5f, 0.5f },

	{-1.0f, 0.0f, 0.0f,  -0.5f,-0.5f,-0.5f },
	{-1.0f, 0.0f, 0.0f,  -0.5f,-0.5f, 0.5f },
	{-1.0f, 0.0f, 0.0f,  -0.5f, 0.5f, 0.5f },
	{-1.0f, 0.0f, 0.0f,  -0.5f, 0.5f,-0.5f } 
};

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void init(void);
void render(void);
void shutDown(void);
void getRealTimeUserInput(void);
void updateViewMatrix(void);
void initPhysics(void);
void updatePhysics(void);

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
                             "OpenGL - Tokamak Chain Sample Using Ball-Joints",
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
                    // Pull up on the last of the jointed cubes
                    neV3 vel;
                    vel.Set( 0.0f, 30.0f, 0.0f );
                    g_redCubes[ NUM_RED_CUBES - 1 ]->SetVelocity( vel );
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
			gluPerspective( 45.0, (GLdouble)nWidth / (GLdouble)nHeight, 0.1, 100.0);
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

	glClearColor( 0.35f, 0.53f, 0.7f, 1.0f );
    glEnable( GL_DEPTH_TEST );
	glEnable( GL_LIGHTING );
	glEnable( GL_NORMALIZE );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0f, 640.0f / 480.0f, 0.1f, 1000.0f);

	//
	// Setup a simple directional light and some ambient...
	//

	glEnable( GL_LIGHT0 );

	GLfloat diffuse_light0[]  = {  1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat specular_light0[] = {  1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv( GL_LIGHT0, GL_DIFFUSE,  diffuse_light0 );
	glLightfv( GL_LIGHT0, GL_SPECULAR, specular_light0 );

	GLfloat ambient_lightModel[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	glLightModelfv( GL_LIGHT_MODEL_AMBIENT, ambient_lightModel );

	// For lighting, pull our material colors from the vertex color...
	glEnable( GL_COLOR_MATERIAL );
	glColorMaterial( GL_FRONT, GL_AMBIENT_AND_DIFFUSE );

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
    // Setup the physics environment...
    //

    neV3 gravity;
    gravity.Set( 0.0f, -10.0f, 0.0f );

    neSimulatorSizeInfo sizeInfo;

	sizeInfo.rigidBodiesCount        = NUM_RED_CUBES;
	sizeInfo.animatedBodiesCount     = NUM_BLUE_PLATFORMS;
	sizeInfo.rigidParticleCount      = 0;
	sizeInfo.controllersCount        = 0;
	// Use the following formula to compute the number of overlapped-pairs required: 
	//(num_rigid_bodies * num_animated_bodies) + num_rigid_bodies * (num_rigid_bodies - 1) / 2
	sizeInfo.overlappedPairsCount    = (NUM_RED_CUBES * NUM_BLUE_PLATFORMS) + NUM_RED_CUBES * (NUM_RED_CUBES - 1) / 2;
	sizeInfo.geometriesCount         = NUM_RED_CUBES + NUM_BLUE_PLATFORMS;
	sizeInfo.constraintsCount        = 29; // We'll need 29 constraints. (ball-joints used by the red cube's)
	sizeInfo.constraintSetsCount     = 29; // I'm not sure what this means, but it's needed to run.
	sizeInfo.constraintBufferSize    = 0;
	sizeInfo.sensorsCount            = 0;
	sizeInfo.terrainNodesStartCount  = 0;
	sizeInfo.terrainNodesGrowByCount = 0;

    g_simulator = neSimulator::CreateSimulator( sizeInfo, NULL, &gravity );

    //
    // Physics for the 30 cubes connected by ball-joints...
    //

    neRigidBody *lastbox = NULL;
    neJoint *lastJoint = NULL;
    float linkLength = 1.2f;

    for( int i = 0; i < NUM_RED_CUBES; i++ )
    {
        float fMass = 0.1f;

        neRigidBody *rigidBody = g_simulator->CreateRigidBody();

        rigidBody->CollideConnected( true );

        neGeometry *cubeGeometry = rigidBody->AddGeometry();

        neV3 cubeSize;
        cubeSize.Set( 1.2f, 0.5f, 0.5f );
        cubeGeometry->SetBoxSize( cubeSize );

        rigidBody->UpdateBoundingInfo();

        rigidBody->SetInertiaTensor( neBoxInertiaTensor( cubeSize, fMass ) );
        rigidBody->SetMass( fMass );

        neV3 position;

        if( i == 0 )
            position.Set( -linkLength, 0.0f, 0.0f );
        else if( i != 0 )
            position.Set( -linkLength * (i+1), 0.0f, 0.0f );

        rigidBody->SetPos( position );

        neJoint *joint = NULL;
        neT3 jointFrame;

        jointFrame.SetIdentity();

        if( i != 0 )
        {
            joint = g_simulator->CreateJoint( rigidBody, lastbox );
            jointFrame.pos.Set( -linkLength * (0.5f + i), 0.0f, 0.0f );
            joint->SetJointFrameWorld( jointFrame );
        }

        if( i == NUM_RED_CUBES - 1 )
            lastJoint = joint;

        if( joint )
        {
            joint->SetType( neJoint::NE_JOINT_BALLSOCKET );
            joint->Enable( true );
        }

        g_redCubes[i] = rigidBody;
        lastbox = rigidBody;
    }

    if( lastJoint )
    {
        lastJoint->SetEpsilon( 0.1f );
        lastJoint->SetIteration( 5 );
    }

    //
    // Physics for the blue platform...
    //

    g_bluePlatform = g_simulator->CreateAnimatedBody();

    neGeometry *platformGeometry = g_bluePlatform->AddGeometry();

    platformGeometry->SetBoxSize( 50.0f, 2.0f, 50.0f );

    g_bluePlatform->UpdateBoundingInfo();

    neV3 position;
    position.Set( 0.0f, -3.0f, 0.0f );

    g_bluePlatform->SetPos( position );
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )	
{
    neSimulator::DestroySimulator( g_simulator );

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
// Name: updatePhysics()
// Desc: 
//-----------------------------------------------------------------------------
void updatePhysics( void )
{
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

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	getRealTimeUserInput();

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
	updateViewMatrix();

	GLfloat position_light0[] = { -0.5f, 0.8f, 0.7f, 0.0f };
	glLightfv( GL_LIGHT0, GL_POSITION, position_light0 );

    //
	// Render our chain, which is composed of 30 red cubes joined by ball-joints
	//

	matrix4x4f matWorld;
	matrix4x4f matScale;

	for( int i = 0; i < NUM_RED_CUBES; i++ )
	{
		if( g_redCubes[i] )
		{
            matWorld = NET3_TO_MATRIX4X4f( g_redCubes[i]->GetTransform() );

            matScale.scale( vector3f( 1.2f, 0.5f, 0.5f ) ); // Scale it so it matches the Tokamak object
            matWorld = matWorld * matScale;

            glPushMatrix();
            {
                glMatrixMode( GL_MODELVIEW );
                glMultMatrixf( matWorld.m );
				glColor3f( 1.0f, 0.0f, 0.0f );
                glInterleavedArrays( GL_N3F_V3F, 0, g_cubeVertices );
                glDrawArrays( GL_QUADS, 0, 24 );
            }
            glPopMatrix();
		}
	}

    //
    // Render the large blue platform, which will keep the chain from falling
    //

    matWorld = NET3_TO_MATRIX4X4f( g_bluePlatform->GetTransform() );

    matScale.scale( vector3f( 50.0f, 2.0f, 50.0f ) ); // Scale it so it matches the Tokamak object
    matWorld = matWorld * matScale;

    glPushMatrix();
    {
        glMatrixMode( GL_MODELVIEW );
        glMultMatrixf( matWorld.m );
		glColor3f( 0.0f, 0.0f, 1.0f );
        glInterleavedArrays( GL_N3F_V3F, 0, g_cubeVertices );
        glDrawArrays( GL_QUADS, 0, 24 );
    }
    glPopMatrix();

	//
	// That's all... just swap the buffers...
	//

	SwapBuffers( g_hDC );
}
