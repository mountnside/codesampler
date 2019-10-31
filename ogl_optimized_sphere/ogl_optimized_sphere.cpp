//-----------------------------------------------------------------------------
//           Name: ogl_optimized_sphere.cpp
//         Author: Kevin Harris
//  Last Modified: 04/21/05
//    Description: Renders a sphere as either a triangle list or a combination 
//                 of tri-strips and tri-fans for benchmarking purposes.
//                 Both versions of the sphere are rendered in immediate mode.
//
//   Control Keys: Left Mouse Button - Spin the view.
//                 F1 - Decrease sphere resolution.
//                 F2 - Increase sphere resolution.
//                 F3 - Performs a quick benchmark with 1000 frames, and 
//                      displays elapsed time, frames per second, triangles 
//                      per second.
//                 F4 - Switches between triangle list rendering (unoptimized)  
//                      and fan/strip rendering (optimized)
//                 F5 - Displays the number of triangles and vertices at the  
//                      current detail level.
//                 F6 - Toggle wire-frame mode.
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
// GLOBALS
//-----------------------------------------------------------------------------
HDC	      g_hDC       = NULL;
HGLRC     g_hRC       = NULL;
HWND      g_hWnd      = NULL;
HINSTANCE g_hInstance = NULL;
GLuint    g_textureID = 0;

float g_fSpinX = 0.0f;
float g_fSpinY = 0.0f;

// Globals for rendering the sphere as triangles
GLfloat **g_vertices  = NULL;
GLfloat **g_colors    = NULL;
GLshort **g_triangles = NULL;

// Globals for rendering the sphere using tri-strips and tri-fans
GLshort  *g_northFan  = NULL;
GLshort  *g_southFan  = NULL;
GLshort **g_triStrips = NULL;

int g_numVertices;
int g_numTriangles;
int g_numSlices = 32;

bool g_bUseTriStrippedSphere = false;
bool g_bRenderInWireFrame = false;

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
void cleanupSphereMemory(void);
void generateNewSphere(void);
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
						    "OpenGL - Optimized Tri-stripped Sphere",
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
                    if( g_numSlices > 5 )
                    {
                        cleanupSphereMemory(); //delete memory BEFORE changing number of g_numSlices!
                        g_numSlices -= 2;
                        generateNewSphere();
                        cout << "Sphere resolution reduced to " << g_numSlices << "\r" << endl;
                    }
                    break;

                case VK_F2:
                    if( g_numTriangles < 30000 ) //don't want to overflow our short
                    {
                        cleanupSphereMemory(); //delete memory BEFORE changing number of g_numSlices!
                        g_numSlices +=2;
                        generateNewSphere();
                        cout << "Sphere resolution increased to " << g_numSlices << endl;
                    }
                    break;

                case VK_F3:
                    cout << endl;
                    cout << "Benchmark Initiated - Standby..." << endl;
                    doBenchmark();
                    break;

                case VK_F4:
                    g_bUseTriStrippedSphere = !g_bUseTriStrippedSphere;

                    if( g_bUseTriStrippedSphere == true )
                        cout << "Now rendering using tri-strips and tri-fans." << endl;
                    else
                        cout << "Now rendering using a triangle list." << endl;
                    break;

                case VK_F5:
                    cout << "Triangles: " << g_numTriangles << ", Vertices:  " << g_numVertices << endl;;
                    break;

                case VK_F6:
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
void loadTexture(void)	
{
    AUX_RGBImageRec *pTextureImage = auxDIBImageLoad( ".\\earth.bmp" );

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
    // Generate the first sphere...
    //

    generateNewSphere();

    cout << "Now rendering with triangle lists.\r" << endl;
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )	
{
    cleanupSphereMemory();

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
// Name: cleanupSphereMemory()
// Desc: 
//-----------------------------------------------------------------------------
void cleanupSphereMemory( void )
{
	int i;

    if( g_vertices == NULL ||  g_colors == NULL || g_triangles == NULL ||
        g_northFan == NULL || g_southFan == NULL )
    return;

    for( i = 0; i < g_numVertices; ++i )
    {
        delete[] g_vertices[i];
        delete[] g_colors[i];
    }
    
    for( i = 0; i < g_numTriangles; ++i )
        delete[] g_triangles[i];

    if( g_triStrips != NULL )
    {
        for( i = 0; i < g_numSlices/2 - 2; ++i )
            delete[] g_triStrips[i];

        delete[] g_triStrips;
    }

	delete[] g_vertices;
	delete[] g_colors;
	delete[] g_triangles;
	delete[] g_northFan;
	delete[] g_southFan;
}

//-----------------------------------------------------------------------------
// Name: generateNewSphere()
// Desc: This function is what generates the mesh.  It calculates the number 
//       of triangles and vertices from the number of g_numSlices, allocates new 
//       storage for the vertices and triangles, and fills them in.
//
// Notes on Sphere Generation:
//
// You can think of 'g_numSlices' as the number of vertices you will touch if you 
// walk either around one parallel, or from one pole down a meridian, and up 
// the opposite meridian to the same pole.  This must always be an even number, 
// or you won't have parallels. Starting with 4 gives you an octahedron. 
//-----------------------------------------------------------------------------
void generateNewSphere( void )
{
    // Number of parallels
    GLshort parallels = g_numSlices/2 - 1;

    // One at each pole, and (g_numSlices) at each parallel
    g_numVertices = 2 + g_numSlices*parallels;

    // Triangle fan at each pole, and double for each strip between parallels
    g_numTriangles = 2*g_numSlices + 2*g_numSlices * (parallels - 1); 

    // Allocate new memory
    g_vertices  = new GLfloat *[g_numVertices];
    g_colors    = new GLfloat *[g_numVertices];
    g_triangles = new GLshort *[g_numTriangles];
    
    for( int i = 0; i < g_numVertices; ++i )
    {
        g_vertices[i] = new GLfloat[3];
        g_colors[i] = new GLfloat[3];
    }

    for( i = 0; i < g_numTriangles; ++i )
        g_triangles[i] = new GLshort[3];

    g_northFan = new GLshort[g_numSlices + 2];
    g_southFan = new GLshort[g_numSlices + 2];

    if( parallels > 1 )
    {
        g_triStrips = new GLshort *[parallels - 1];

        for (i = 0; i < parallels - 1; ++i)
            g_triStrips[i] = new GLshort[2*(g_numSlices + 1)];
    }
    else
        g_triStrips = NULL;

    // Now, for the vertex and triangle generation
    GLfloat anglestep = 2 * M_PI / (GLfloat)g_numSlices; //radians per slice

    // Start with the north pole
    g_vertices[0][0] = 0.0f;
    g_vertices[0][1] = 1.0f;
    g_vertices[0][2] = 0.0f;

    g_colors[0][0] = 1.0f;
    g_colors[0][1] = 0.0f;
    g_colors[0][2] = 0.0f;

    // Now, the fun part.  At each parallel, generate a vertex at each meridian.
    for( i = 0; i < parallels; ++i )
    {
        for( int j = 0; j < g_numSlices; ++j )
        {
            int vertex = 1 + i*g_numSlices + j; //our current vertex

            // My spherical coordinates are radius, heading, pitch.  zero pitch looks straight up,
            // and pi pitch looks straight down.  pi/2 pitch looks straight forward.
            // radius is always 1.0, it's a unit sphere.

            // x = radius * sin(pitch) * cos(heading)
            g_vertices[vertex][0] = sinf(anglestep*(i+1)) * cosf(anglestep*j);
            // y = radius * cos(pitch)
            g_vertices[vertex][1] = cosf(anglestep*(i+1));
            // z = radius * sin(pitch) * sin(heading)
            g_vertices[vertex][2] = sinf(anglestep*(i+1)) * sinf(anglestep*j);

            // I chose these colors arbitrarily.  Red is the north pole, Blue 
			// is the south, and the equator is red + blue = magenta.
            // The amount of green varies with the heading.  Maybe it will 
			// look interesting.
            if( g_vertices[vertex][1] >= 0.0f)
            {
                g_colors[vertex][0] = 1.0f;
                g_colors[vertex][2] = 1.0f - g_vertices[vertex][1];
            }
            else
            {
                g_colors[vertex][0] = 1.0f + g_vertices[vertex][1];
                g_colors[vertex][2] = 1.0f;
            }

            GLfloat green = sinf(anglestep*j);

            if( green < 0.0f )
                green *= -1.0f;

            g_colors[vertex][1] = green;
        }
    }

    // Don't forget the south pole.
    g_vertices[g_numVertices - 1][0] =  0.0f;
    g_vertices[g_numVertices - 1][1] = -1.0f;
    g_vertices[g_numVertices - 1][2] =  0.0f;
    g_colors[g_numVertices - 1][0] = 0.0f;
    g_colors[g_numVertices - 1][1] = 0.0f;
    g_colors[g_numVertices - 1][2] = 1.0f;

    // We're not done yet!  Gotta make triangles, of course.
    // Let's start with the north pole again, and make a fan with the first parallel.
    for( i = 0; i < g_numSlices - 1; ++i )
    {
        g_triangles[i][0] = 0;
        g_triangles[i][1] = i + 1;
        g_triangles[i][2] = i + 2;
    }

    // finish the fan, use the pole, and the last and first vertices of the parallel
    g_triangles[g_numSlices-1][0] = 0;
    g_triangles[g_numSlices-1][1] = g_numSlices;
    g_triangles[g_numSlices-1][2] = 1;

    // make the optimized fan
    g_northFan[0] = 0;

    for( i = 0; i < g_numSlices; ++i )
        g_northFan[i+1] = i+1;

    g_northFan[g_numSlices + 1] = 1;

    // I'm lazy, so I'll do the south pole next since it's easy to figure out.
    for( i = 0; i < g_numSlices - 1; ++i )
    {
        g_triangles[i + g_numSlices][0] = g_numVertices - 1;
        g_triangles[i + g_numSlices][1] = g_numVertices - 1 - g_numSlices + i;
        g_triangles[i + g_numSlices][2] = g_numVertices - g_numSlices + i;
    }

    // finish the fan, use the pole, and the last and first vertices of the parallel
    g_triangles[i + g_numSlices][0] = g_numVertices - 1;
    g_triangles[i + g_numSlices][1] = g_numVertices - 2;
    g_triangles[i + g_numSlices][2] = g_numVertices - 1 - g_numSlices;

    // make the optimized fan
    g_southFan[0] = g_numVertices - 1;

    for( i = 0; i < g_numSlices; ++i )
        g_southFan[i+1] = g_numVertices - g_numSlices - 1 + i;

    g_southFan[g_numSlices + 1] = g_numVertices - g_numSlices - 1;

    // Now, if there are any strips to be made, make them...
    // ... but, if there's only 1 parallel, skip this step
    for( i = 1; i < parallels; ++i ) 
    {
        for( int j = 0; j < g_numSlices - 1; ++j )
        {
            // Make a square, use the current and next parallel and the 
			// current and next meridian
            g_triangles[2*(g_numSlices*i + j)  ][0] = (i - 1)*g_numSlices + 1 + j;
            g_triangles[2*(g_numSlices*i + j)  ][1] = (i - 1)*g_numSlices + 2 + j;
            g_triangles[2*(g_numSlices*i + j)  ][2] = i*g_numSlices + 1 + j;
            g_triangles[2*(g_numSlices*i + j)+1][0] = (i - 1)*g_numSlices + 2 + j;
            g_triangles[2*(g_numSlices*i + j)+1][1] = i*g_numSlices + 1 + j;
            g_triangles[2*(g_numSlices*i + j)+1][2] = i*g_numSlices + 2 + j;

            // Make the optimized strip
            g_triStrips[i-1][2*j]     = 1 + g_numSlices*(i-1) + j;
            g_triStrips[i-1][2*j + 1] = 1 + g_numSlices*i + j;
        }

        // Make one more square
        g_triangles[2*(g_numSlices*i + j)  ][0] = (i - 1)*g_numSlices + 1 + j;
        g_triangles[2*(g_numSlices*i + j)  ][1] = (i - 1)*g_numSlices + 1;
        g_triangles[2*(g_numSlices*i + j)  ][2] = i*g_numSlices + 1 + j;
        g_triangles[2*(g_numSlices*i + j)+1][0] = (i - 1)*g_numSlices + 1;
        g_triangles[2*(g_numSlices*i + j)+1][1] = i*g_numSlices + 1 + j;
        g_triangles[2*(g_numSlices*i + j)+1][2] = i*g_numSlices + 1;

        // Finish the optimized strip
        g_triStrips[i-1][2*g_numSlices-2] = g_numSlices*i;
        g_triStrips[i-1][2*g_numSlices-1] = g_numSlices*(i+1);
        g_triStrips[i-1][2*g_numSlices  ] = 1 + g_numSlices*(i-1);
        g_triStrips[i-1][2*g_numSlices+1] = 1 + g_numSlices*i;
    }
}

//-----------------------------------------------------------------------------
// Name: doBenchmark()
// Desc: 
//-----------------------------------------------------------------------------
void doBenchmark( void )
{
    _timeb start, finish;
    int frames = 1000;
    float elapsed;

    _ftime( &start ); // Get the time

    while(frames--) // Loop away
        render();

    _ftime( &finish ); // Get the time again

    elapsed  = (float)(finish.time - start.time);                // This is accurate to one second
    elapsed += (float)((finish.millitm - start.millitm)/1000.0); // This gets it down to one ms

    cout << "-- Benchmark Report --" << endl << endl;
    cout << "Elapsed Time: " << elapsed << " seconds " << endl;
    cout << "Frames Per Second: " << (float)1000.0/elapsed  << endl;
    cout << "Triangles Per Second: " << int((g_numTriangles*1000)/elapsed) << endl;
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
    glTranslatef( 0.0f, 0.0f, -4.0f );
    glRotatef( -g_fSpinY, 1.0f, 0.0f, 0.0f );
    glRotatef( -g_fSpinX, 0.0f, 1.0f, 0.0f );

    if( g_bRenderInWireFrame == true )
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    else
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    //
    // Render test sphere...
    //

    if( g_bUseTriStrippedSphere )
	{
		//
		// Render the optimized sphere using tri-strips and tri-fans...
		//

		GLshort fanLength = g_numSlices + 2;

		glBegin( GL_TRIANGLE_FAN );
		{
			for( int i = 0; i < fanLength; ++i )
			{
				glColor3fv(g_colors[g_northFan[i]]);
				glVertex3fv(g_vertices[g_northFan[i]]);
			}
		}
		glEnd();

		glBegin( GL_TRIANGLE_FAN );
		{
			for( int i = 0; i < fanLength; ++i )
			{
				glColor3fv(g_colors[g_southFan[i]]);
				glVertex3fv(g_vertices[g_southFan[i]]);
			}
		}
		glEnd();

		//
		// Now, the tri-strips...
		//

		GLshort strips = g_numSlices/2 - 2; // Number of strips
		GLshort stripLength = 2*g_numSlices + 2;

		for( int i = 0; i < strips; ++i )
		{
			glBegin( GL_TRIANGLE_STRIP );
			{
				for( int j = 0; j < stripLength; ++j )
				{
					glColor3fv(g_colors[g_triStrips[i][j]]);
					glVertex3fv(g_vertices[g_triStrips[i][j]]);
				}
			}
			glEnd();
		}
	}
	else
	{
		//
		// Simply render the sphere using a triangle list...
		//

		glBegin( GL_TRIANGLES );
		{
			for( int i = 0; i < g_numTriangles; ++i )
			{
				for( int j = 0; j < 3; ++j )
				{
					glColor3fv( g_colors[g_triangles[i][j]] );
					glVertex3fv( g_vertices[g_triangles[i][j]] );
				}
			}
		}
		glEnd();
	}

	SwapBuffers( g_hDC );
}

