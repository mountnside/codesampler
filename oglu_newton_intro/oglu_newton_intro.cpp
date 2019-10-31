//-----------------------------------------------------------------------------
//           Name: oglu_newton_intro.cpp
//         Author: Anudhyan Boral (anudhyan@gmail.com)
//  Last Modified: 26/07/06
//    Description: This sample serves as an introductory sample for  
//				   the Newton API for realtime physics simulations.
//				   This is a modification of the official CodeSampler sample -
//				   OpenGL Initialization.
//				   You can download the Newton sdk from 
//				   their website (www.newtongamedynamics.com) 
//	 
//   Control Keys: F1 - Increase angular velocity of the box
//				  
//-----------------------------------------------------------------------------

// ( Original Sample Header )
//-----------------------------------------------------------------------------
//           Name: ogl_initialization.cpp
//         Author: Kevin Harris
//  Last Modified: 02/01/05
//    Description: This sample demonstrates how to initialize OpenGL.
//-----------------------------------------------------------------------------

// Links to the required libraries
#pragma comment (lib , "opengl32.lib" )
#pragma comment (lib , "glu32.lib" )
#pragma comment (lib , "newton.lib" )

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "Newton.h" // The Newton header with this sample
//#include <Newton.h> // Your local Newton header

#include "resource.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND  g_hWnd = NULL;
HDC	  g_hDC  = NULL;
HGLRC g_hRC  = NULL;

// Our Newton World
NewtonWorld* g_nWorld; 

// A Newton rigid body, which will be a simple box in this example. 
NewtonBody *g_box; 

// A basic vertex structure to describe our box
struct Vertex
{
    float x, y, z;
};


// List of vertices defining a cube ( or a box ) 
const int NUM_VERTICES = 24;

Vertex g_boxVertices[NUM_VERTICES] =
{
//   x     y     z 
    // Front face
    {-1.0f,-1.0f, 1.0f },
    { 1.0f,-1.0f, 1.0f },
    { 1.0f, 1.0f, 1.0f },
    {-1.0f, 1.0f, 1.0f },
    // Back face
    {-1.0f,-1.0f,-1.0f },
    {-1.0f, 1.0f,-1.0f },
    { 1.0f, 1.0f,-1.0f },
    { 1.0f,-1.0f,-1.0f },
    // Top face
    {-1.0f, 1.0f,-1.0f },
    {-1.0f, 1.0f, 1.0f },
    { 1.0f, 1.0f, 1.0f },
    { 1.0f, 1.0f,-1.0f },
    // Bottom face
    {-1.0f,-1.0f,-1.0f },
    { 1.0f,-1.0f,-1.0f },
    { 1.0f,-1.0f, 1.0f },
    {-1.0f,-1.0f, 1.0f },
    // Right face
    { 1.0f,-1.0f,-1.0f },
    { 1.0f, 1.0f,-1.0f },
    { 1.0f, 1.0f, 1.0f },
    { 1.0f,-1.0f, 1.0f },
    // Left face
    {-1.0f,-1.0f,-1.0f },
    {-1.0f,-1.0f, 1.0f },
    {-1.0f, 1.0f, 1.0f },
    {-1.0f, 1.0f,-1.0f }
};




//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void InitGL(void);
void InitNewton();
void ShutDown(void);
void Render(void);


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
                             "OpenGL - Introduction to Newton Physics",
						     WS_OVERLAPPEDWINDOW | WS_VISIBLE,
					         0, 0, 640, 480, NULL, NULL, hInstance, NULL );

	if( g_hWnd == NULL )
		return E_FAIL;

    ShowWindow( g_hWnd, nCmdShow );
    UpdateWindow( g_hWnd );

	InitGL();
	InitNewton();

	while( uMsg.message != WM_QUIT )
	{
		if( PeekMessage( &uMsg, NULL, 0, 0, PM_REMOVE ) )
		{
			TranslateMessage( &uMsg );
			DispatchMessage( &uMsg );
		}
        else
			Render();
	}

	ShutDown();

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
	// Array for storing and retrieving the angular velocity of the box
	float omega[] = {1.0f, 1.0f, 1.0f};

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
					// Get the current angular velocity and store it in the array
					NewtonBodyGetOmega (g_box, &omega[0]); 
					// Increase the angular velocity by 0.5
					omega[0]+=0.5f;
					omega[1]+=0.5f;
					omega[2]+=0.5f;
					// Set the increased angular velocity 
					NewtonBodySetOmega (g_box, &omega[0]); 
					break;
			}
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
// Name: InitGL()
// Desc: Initializes OpenGL 
//-----------------------------------------------------------------------------
void InitGL( void )
{
	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

    pfd.nSize      = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion   = 1;
    pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 16;
	pfd.cDepthBits = 16;

	g_hDC = GetDC( g_hWnd );
	GLuint iPixelFormat = ChoosePixelFormat( g_hDC, &pfd );

	if( iPixelFormat != 0 )
	{
		PIXELFORMATDESCRIPTOR bestMatch_pfd;
		DescribePixelFormat( g_hDC, iPixelFormat, sizeof(pfd), &bestMatch_pfd );

		// TO DO: Double-check  the closet match pfd for anything unacceptable...

		if( bestMatch_pfd.cDepthBits < pfd.cDepthBits )
		{
			// POTENTIAL PROBLEM: We need at least a 16-bit z-buffer!
			return;
		}

		if( SetPixelFormat( g_hDC, iPixelFormat, &pfd) == FALSE )
		{
			DWORD dwErrorCode = GetLastError();
			// TO DO: Report cause of failure here...
			return;
		}
	}
	else
	{
		DWORD dwErrorCode = GetLastError();
		// TO DO: Report cause of failure here...
		return;
	}

	g_hRC = wglCreateContext( g_hDC );
	wglMakeCurrent( g_hDC, g_hRC );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0, (GLdouble)640 / (GLdouble)480, 0.1, 100.0);

	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

}

//-----------------------------------------------------------------------------
// Name: InitNewton()
// Desc: Initializes the Newton Physics Engine
//-----------------------------------------------------------------------------
void InitNewton()
{
	// For describing the solid geometry  in the world
	NewtonCollision *collision;

	// Create the world 
	g_nWorld = NewtonCreate (NULL, NULL);

	// Create the collision shape of a box
	collision = NewtonCreateBox (g_nWorld, 2.0f, 2.0f, 2.0f, NULL); 

	// Create the rigid body 
	g_box = NewtonCreateBody (g_nWorld, collision);

	// Release the collision, we won't be describing any more geometry 
	// So we won't need it anymore
	NewtonReleaseCollision (g_nWorld, collision);

	// Set the mass and inertia of our body 
	NewtonBodySetMassMatrix (g_box, 1.0f, 1.0f, 1.0f, 1.0f);

	// We move the body a little behind ( z axis -10 units ) so that it stays a little 
	// far from the camera 
	float matrix[4][4] = 
	{
		1.0f,	0.0f,	0.0f,	0.0f,
		0.0f,	1.0f,	0.0f,	0.0f,
		0.0f,	0.0f,	1.0f,	0.0f,
		0.0f,	1.0f,  -10.0f,	0.0f
	};
	NewtonBodySetMatrix (g_box, &matrix[0][0]);

	// Give the box a little angular velocity. 
	// so that it moves a little. A static box would be boring. 
	float omega[] = {10.0f, 10.0f, 10.0f};
	NewtonBodySetOmega (g_box, &omega[0]); 
}

//-----------------------------------------------------------------------------
// Name: ShutDown()
// Desc: Clean up when shutting down 
//-----------------------------------------------------------------------------
void ShutDown( void )	
{
	// Clean up and destroy the newton world
	NewtonDestroy (g_nWorld);

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
// Name: Render()
// Desc: Render our newton world 
//-----------------------------------------------------------------------------
void Render( void )	
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glMatrixMode(GL_MODELVIEW);

	glPolygonMode(GL_FRONT_AND_BACK,GL_LINE); // wireframe mode

	// Update our world every frame 
	// The second parameter is the elapsed time since last frame ( i.e. the frametime )
	// To avoid time calculating code, we have just sent Newton an estimated time that may elapse
	// NOTE: This is not at all suitable for correct physics simulation  
	NewtonUpdate (g_nWorld, 0.00001f); 

	// Get the current matrix of our box and store in in the 4x4 matrix array 
	// We need to get the matrix for rendering our visual box correctly
	float matrix [4][4] = {0};
	NewtonBodyGetMatrix(g_box, &matrix[0][0]);

	// Push the current OpenGL matrix so that we can retreive it later
	glPushMatrix();

	// Multiply the matrix we got from newton to the modelview matrix
	glMultMatrixf(&matrix[0][0]);

	// Draw our box
    glBegin( GL_QUADS );
    {
        for( int i = 0; i < NUM_VERTICES; ++i )
        {
            glVertex3f( g_boxVertices[i].x, g_boxVertices[i].y, g_boxVertices[i].z );
        }
    }
    glEnd();

	// Retrieve the pushed modelview matrix 
	glPopMatrix();
	
	SwapBuffers( g_hDC );
}
