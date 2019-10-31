//-----------------------------------------------------------------------------
//           Name: ogl_benchmark_sphere.cpp
//         Author: Kevin Harris
//  Last Modified: 10/13/05
//    Description: Renders a textured sphere using either Immediate Mode calls,
//                 Immediate Mode calls cached in a Display List, or as a 
//                 collection of geometric data stored in an interleaved 
//                 fashion within a Vertex Array.
//
//   Control Keys: Left Mouse Button - Spin the view.
//                 F1 - Decrease sphere precision.
//                 F2 - Increase sphere precision.
//                 F3 - Use Immediate mode
//                 F4 - Use a Display List
//                 F5 - Use a Vertex Array
//                 F6 - Perform Benchmarking
//                 F7 - Toggle wire-frame mode.
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
// Do this to access M_PI, which is not officially part of the C/C++ standard.
#define _USE_MATH_DEFINES 
#include <math.h>
#include <sys/timeb.h>
#include <iostream>
#include <io.h>
#include <fcntl.h>
using namespace std;
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glaux.h>
#include "resource.h"

//-----------------------------------------------------------------------------
// DEFINES
//-----------------------------------------------------------------------------
#define IMMEDIATE_MODE 0
#define DISPLAY_LIST   1
#define VERTEX_ARRAY   2

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HDC	      g_hDC       = NULL;
HGLRC     g_hRC       = NULL;
HWND      g_hWnd      = NULL;
HINSTANCE g_hInstance = NULL;

float g_fSpinX = 0.0f;
float g_fSpinY = 0.0f;

GLuint g_textureID = 0;
GLuint g_sphereDList;

bool    g_bRenderInWireFrame = false;
GLuint  g_nCurrentMode = IMMEDIATE_MODE;
GLuint  g_nPrecision  = 100;
GLuint  g_nNumSphereVertices;
GLfloat g_fMarsSpin   = 0.0f;

// A custom data structure for our interleaved vertex attributes
// The interleaved layout will be, GL_T2F_N3F_V3F
struct Vertex
{
    float tu, tv;
    float nx, ny, nz;
    float vx, vy, vz;
};

Vertex *g_pSphereVertices = NULL; // Points to Vertex Array

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE g_hInstance,HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND g_hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void redirectIOToConsole(void);
void loadTexture(void);
void init(void);
void render(void);
void shutDown(void);
void renderSphere(float cx, float cy, float cz, float r, int n);
void createSphereDisplayList();
void createSphereGeometry( float cx, float cy, float cz, float r, int n);
void setVertData(int index,float tu, float tv, float nx, float ny, float nz, 
                 float vx, float vy, float vz);
void doBenchmark(void);

//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: The application's entry point
//-----------------------------------------------------------------------------
int WINAPI WinMain(	HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPSTR     lpCmdLine,
					int       nCmdShow )
{
    redirectIOToConsole();

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

	g_hWnd = CreateWindowEx( NULL,"MY_WINDOWS_CLASS",
						    "OpenGL - Benchmark Sphere",
							WS_OVERLAPPEDWINDOW,
					 	    0,0, 640,480, NULL, NULL, g_hInstance, NULL );

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
		    render();
	}

	shutDown();

    UnregisterClass( "MY_WINDOWS_CLASS", g_hInstance );

	return uMsg.wParam;
}

//-----------------------------------------------------------------------------
// Name: WindowProc()
// Desc: The window's message handler
//-----------------------------------------------------------------------------
LRESULT CALLBACK WindowProc( HWND   g_hWnd, 
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
                    if( g_nPrecision > 5 )
                        g_nPrecision -= 2;

                    if( g_nCurrentMode == DISPLAY_LIST )
                    {
                        createSphereDisplayList();
                    }

                    if( g_nCurrentMode == VERTEX_ARRAY ||
                        g_nCurrentMode == IMMEDIATE_MODE)
                    {
                        createSphereGeometry( 0.0f, 0.0f, 0.0f, 1.5f, g_nPrecision );
                    }

                    cout << "Sphere Resolution = " << g_nPrecision << endl;
                    break;

                case VK_F2:
                    if( g_nPrecision < 30000 )
                        g_nPrecision += 2;

                    if( g_nCurrentMode == DISPLAY_LIST )
                    {
                        createSphereDisplayList();
                    }

                    if( g_nCurrentMode == VERTEX_ARRAY ||
                        g_nCurrentMode == IMMEDIATE_MODE )
                    {
                        createSphereGeometry( 0.0f, 0.0f, 0.0f, 1.5f, g_nPrecision );
                    }

                    cout << "Sphere Resolution = " << g_nPrecision << endl;
                    break;

                case VK_F3:
                    g_nCurrentMode = IMMEDIATE_MODE;
                    cout << "Render Method: Immediate Mode" << endl;
                    createSphereGeometry( 0.0f, 0.0f, 0.0f, 1.5f, g_nPrecision );
                    break;

                case VK_F4:
                    g_nCurrentMode = DISPLAY_LIST;
                    createSphereDisplayList();
                    cout << "Render Method: Display List" << endl;
                    break;

                case VK_F5:
                    g_nCurrentMode = VERTEX_ARRAY;
                    createSphereGeometry( 0.0f, 0.0f, 0.0f, 1.5f, g_nPrecision );
                    cout << "Render Method: Vertex Array" << endl;
                    break;

                case VK_F6:
                    cout << endl;
                    cout << "Benchmark Initiated - Standby..." << endl;
                    doBenchmark();
                    break;

                case VK_F7:
                    g_bRenderInWireFrame = !g_bRenderInWireFrame;
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
			return DefWindowProc( g_hWnd, msg, wParam, lParam );
		}
		break;
	}

	return 0;
}

//-----------------------------------------------------------------------------
// Name: redirectIOToConsole()
// Desc: 
//-----------------------------------------------------------------------------
void redirectIOToConsole( void )
{
    // Allocate a console so we can output some useful information.
    AllocConsole();

    // Get the handle for STDOUT's file system.
    HANDLE stdOutputHandle = GetStdHandle( STD_OUTPUT_HANDLE );

    // Redirect STDOUT to the new console by associating STDOUT's file 
    // descriptor with an existing operating-system file handle.
    int hConsoleHandle = _open_osfhandle( (intptr_t)stdOutputHandle, _O_TEXT );
    FILE *pFile = _fdopen( hConsoleHandle, "w" );
    *stdout = *pFile;
    setvbuf( stdout, NULL, _IONBF, 0 );

    // This call ensures that iostream and C run-time library operations occur  
    // in the order that they appear in source code.
    ios::sync_with_stdio();
}

//-----------------------------------------------------------------------------
// Name: loadTexture()
// Desc: 
//-----------------------------------------------------------------------------
void loadTexture( void )	
{
    AUX_RGBImageRec *pTextureImage = auxDIBImageLoad( ".\\mars.bmp" );

    if( pTextureImage != NULL )
    {
        glGenTextures( 1, &g_textureID );

        glBindTexture( GL_TEXTURE_2D, g_textureID );

        glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR );

        glTexImage2D( GL_TEXTURE_2D, 0, 3, pTextureImage->sizeX, pTextureImage->sizeY, 0,
            GL_RGB, GL_UNSIGNED_BYTE, pTextureImage->data );
    }

    if( pTextureImage )
    {
        if( pTextureImage->data )
            free( pTextureImage->data );

        free( pTextureImage );
    }
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
    pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 16;
    pfd.cDepthBits = 16;
	
	g_hDC = GetDC( g_hWnd );
	PixelFormat = ChoosePixelFormat( g_hDC, &pfd );
	SetPixelFormat( g_hDC, PixelFormat, &pfd );
	g_hRC = wglCreateContext( g_hDC );
	wglMakeCurrent( g_hDC, g_hRC );

	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_DEPTH_TEST );

    loadTexture();

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0f, 640.0f / 480.0f, 0.1f, 100.0f );

    //
    // Create the first sphere...
    //

    // Inform the user of the current mode
    cout << "Render Method: Immediate Mode" << endl;

    createSphereGeometry( 0.0f, 0.0f, 0.0f, 1.5f, g_nPrecision );
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )	
{
    glDeleteTextures( 1, &g_textureID );
        
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
// Name: renderSphere()
// Desc: Create a sphere centered at cy, cx, cz with radius r, and 
//       precision p. Based on a function Written by Paul Bourke. 
//       http://astronomy.swin.edu.au/~pbourke/opengl/sphere/
//-----------------------------------------------------------------------------
void renderSphere( float cx, float cy, float cz, float r, int p )
{
    const float PI     = 3.14159265358979f;
    const float TWOPI  = 6.28318530717958f;
    const float PIDIV2 = 1.57079632679489f;

    float theta1 = 0.0;
    float theta2 = 0.0;
    float theta3 = 0.0;

    float ex = 0.0f;
    float ey = 0.0f;
    float ez = 0.0f;

    float px = 0.0f;
    float py = 0.0f;
    float pz = 0.0f;

    // Disallow a negative number for radius.
    if( r < 0 )
        r = -r;

    // Disallow a negative number for precision.
    if( p < 0 )
        p = -p;

    // If the sphere is too small, just render a OpenGL point instead.
    if( p < 4 || r <= 0 ) 
    {
        glBegin( GL_POINTS );
        glVertex3f( cx, cy, cz );
        glEnd();
        return;
    }

    for( int i = 0; i < p/2; ++i )
    {
        theta1 = i * TWOPI / p - PIDIV2;
        theta2 = (i + 1) * TWOPI / p - PIDIV2;

        glBegin( GL_TRIANGLE_STRIP );
        {
            for( int j = 0; j <= p; ++j )
            {
                theta3 = j * TWOPI / p;

                ex = cosf(theta2) * cosf(theta3);
                ey = sinf(theta2);
                ez = cosf(theta2) * sinf(theta3);
                px = cx + r * ex;
                py = cy + r * ey;
                pz = cz + r * ez;

                glNormal3f( ex, ey, ez );
                glTexCoord2f( -(j/(float)p) , 2*(i+1)/(float)p );
                glVertex3f( px, py, pz );

                ex = cosf(theta1) * cosf(theta3);
                ey = sinf(theta1);
                ez = cosf(theta1) * sinf(theta3);
                px = cx + r * ex;
                py = cy + r * ey;
                pz = cz + r * ez;

                glNormal3f( ex, ey, ez );
                glTexCoord2f( -(j/(float)p), 2*i/(float)p );
                glVertex3f( px, py, pz );
            }
        }
        glEnd();
    }
}

//-----------------------------------------------------------------------------
// Name: createSphereDisplayList()
// Desc: Build Sphere Display List
//-----------------------------------------------------------------------------
void createSphereDisplayList()
{
    glDeleteLists( g_sphereDList, 0 );

    static bool firstPass = true;

    if( firstPass )
    {
        g_sphereDList = glGenLists(1);
        firstPass     = false;
    }

    if( g_sphereDList != 0 )
    {
        glNewList( g_sphereDList, GL_COMPILE );
        // Cache the calls needed to render a sphere
        renderSphere( 0.0f, 0.0f, 0.0f, 1.5f, g_nPrecision );
        glEndList();
    }
}

//-----------------------------------------------------------------------------
// Name: setVertData()
// Desc: Helper function for createSphereGeometry()
//-----------------------------------------------------------------------------
void setVertData( int index,
                 float tu, float tv, 
                 float nx, float ny, float nz, 
                 float vx, float vy, float vz )	
{
    (g_pSphereVertices+index)->tu = tu;
    (g_pSphereVertices+index)->tv = tv;
    (g_pSphereVertices+index)->nx = nx;
    (g_pSphereVertices+index)->ny = ny;
    (g_pSphereVertices+index)->nz = nz;
    (g_pSphereVertices+index)->vx = vx;
    (g_pSphereVertices+index)->vy = vy;
    (g_pSphereVertices+index)->vz = vz;
}

//-----------------------------------------------------------------------------
// Name: createSphereGeometry()
// Desc: Creates a sphere as an array of vertex data suitable to be fed into a 
//       OpenGL vertex array. The sphere will be centered at cy, cx, cz with 
//       radius r, and precision p. Based on a function Written by Paul Bourke. 
//       http://astronomy.swin.edu.au/~pbourke/opengl/sphere/
//-----------------------------------------------------------------------------
void createSphereGeometry( float cx, float cy, float cz, float r, int p )	
{
    const float PI = 3.14159265358979f;
    const float TWOPI = 6.28318530717958f;
    const float PIDIV2 = 1.57079632679489f;

    float theta1 = 0.0;
    float theta2 = 0.0;
    float theta3 = 0.0;

    float ex = 0.0f;
    float ey = 0.0f;
    float ez = 0.0f;

    float px = 0.0f;
    float py = 0.0f;
    float pz = 0.0f;

    float tu  = 0.0f;
    float tv  = 0.0f;

    //-------------------------------------------------------------------------
    // If sphere precision is set to 4, then 20 verts will be needed to 
    // hold the array of GL_TRIANGLE_STRIP(s) and so on...
    //
    // Example:
    //
    // total_verts = (p/2) * ((p+1)*2)
    // total_verts = (4/2) * (  5  *2)
    // total_verts =   2   *  10
    // total_verts =      20
    //-------------------------------------------------------------------------

    g_nNumSphereVertices = (p/2) * ((p+1)*2);

    if( g_pSphereVertices != NULL )
    {
        delete []g_pSphereVertices;
        g_pSphereVertices = NULL;
        g_pSphereVertices = new Vertex[g_nNumSphereVertices];
    }
    else
    {
        g_pSphereVertices = new Vertex[g_nNumSphereVertices];
    }

    // Disallow a negative number for radius.
    if( r < 0 )
        r = -r;

    // Disallow a negative number for precision.
    if( p < 4 ) 
        p = 4;

    int k = -1;

    for( int i = 0; i < p/2; ++i )
    {
        theta1 = i * TWOPI / p - PIDIV2;
        theta2 = (i + 1) * TWOPI / p - PIDIV2;

        for( int j = 0; j <= p; ++j )
        {
            theta3 = j * TWOPI / p;

            ex = cosf(theta2) * cosf(theta3);
            ey = sinf(theta2);
            ez = cosf(theta2) * sinf(theta3);
            px = cx + r * ex;
            py = cy + r * ey;
            pz = cz + r * ez;
            tu  = -(j/(float)p);
            tv  = 2*(i+1)/(float)p;

            ++k;
            setVertData( k, tu, tv, ex, ey, ez, px, py, pz );

            ex = cosf(theta1) * cosf(theta3);
            ey = sinf(theta1);
            ez = cosf(theta1) * sinf(theta3);
            px = cx + r * ex;
            py = cy + r * ey;
            pz = cz + r * ez;
            tu  = -(j/(float)p);
            tv  = 2*i/(float)p;

            ++k;
            setVertData( k, tu, tv, ex, ey, ez, px, py, pz );
        }
    }
}

//-----------------------------------------------------------------------------
// Name: doBenchmark()
// Desc: 
//-----------------------------------------------------------------------------
void doBenchmark()
{
    timeb start;
    timeb finish;
    float  fElapsed = 0.0f;
    int    nFrames  = 1000;

    ftime( &start );  // Get the time

    while( nFrames-- ) // Loop away
        render();

    ftime( &finish ); // Get the time again

    fElapsed  = (float)(finish.time - start.time);                // This is accurate to one second
    fElapsed += (float)((finish.millitm - start.millitm)/1000.0); // This gets it down to one ms

    cout << endl;
    cout << "-- Benchmark Report --" << endl;

    if( g_nCurrentMode == IMMEDIATE_MODE )
        cout << "Render Method:     Immediate Mode" << endl;
    if( g_nCurrentMode == DISPLAY_LIST )
        cout << "Render Method:     Display List" << endl;
    if( g_nCurrentMode == VERTEX_ARRAY )
        cout << "Render Method:     Vertex Array" << endl;

    cout << "Frames Rendered:   1000" << endl;
    cout << "Sphere Resolution: " << g_nPrecision << endl;
    cout << "Primitive Used:    GL_TRIANGLE_STRIP" << endl;
    cout << "Elapsed Time:      " << fElapsed << endl;
    cout << "Frames Per Second: " << 1000.0/fElapsed << endl;
    cout << endl;
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: 
//-----------------------------------------------------------------------------
void render( void )
{
	// Clear the screen and the depth buffer
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glTranslatef( 0.0f, 0.0f, -5.0f );
    glRotatef( -g_fSpinY, 1.0f, 0.0f, 0.0f );
    glRotatef( -g_fSpinX, 0.0f, 1.0f, 0.0f );

    if( g_bRenderInWireFrame == true )
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    else
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    //
    // Render test sphere...
    //

    glBindTexture( GL_TEXTURE_2D, g_textureID );

    if( g_nCurrentMode == IMMEDIATE_MODE )
    {
        // Render a textured sphere using immediate mode

        // To be fair to immediate mode, we won't force it incur the overhead 
        // of calling hundreds of math subroutines to generate a sphere each 
        // frame, instead, we'll use the same array that we would use for 
        // testing the vertex array, but we'll make the immediate mode calls 
        // ourselves. This is more typical of how a real app would use 
        // immediate mode calls.

        glBegin( GL_TRIANGLE_STRIP );
        {
            for( GLuint i = 0; i < g_nNumSphereVertices; ++i )
            {
                glNormal3f( (g_pSphereVertices+i)->nx,
                            (g_pSphereVertices+i)->ny,
                            (g_pSphereVertices+i)->nz );

                glTexCoord2f( (g_pSphereVertices+i)->tu,
                              (g_pSphereVertices+i)->tv );

                glVertex3f( (g_pSphereVertices+i)->vx,
                            (g_pSphereVertices+i)->vy,
                            (g_pSphereVertices+i)->vz );
            }
        }
        glEnd();
    }

    if( g_nCurrentMode == DISPLAY_LIST )
    {
        // Render a textured sphere as a display list
        glCallList( g_sphereDList );
    }

    if( g_nCurrentMode == VERTEX_ARRAY )
    {
        // Render a textured sphere using a vertex array
        glInterleavedArrays( GL_T2F_N3F_V3F, 0, g_pSphereVertices );
        glDrawArrays( GL_TRIANGLE_STRIP, 0, g_nNumSphereVertices );
    }

	SwapBuffers( g_hDC );
}

