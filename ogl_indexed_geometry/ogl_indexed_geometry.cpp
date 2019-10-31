//-----------------------------------------------------------------------------
//           Name: ogl_indexed_geometry.cpp
//         Author: Kevin Harris
//  Last Modified: 02/01/05
//    Description: This sample demonstrates how to optimize performance by 
//                 using indexed geometry. As a demonstration, the sample 
//                 reduces the vertex count of a simple cube from 24 to 8 by 
//                 redefining the cube’s geometry using an indices array.
//
//   Control Keys: F1 - Toggle between indexed and non-indexed geoemtry.
//                      Shouldn't produce any noticeable change since they
//                      render the same cube.
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glaux.h>
#include "resource.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND  g_hWnd = NULL;
HDC	  g_hDC  = NULL;
HGLRC g_hRC  = NULL;

float g_fSpinZ = 0.0f;
float g_fSpinY = 0.0f;

bool g_bUseIndexedGeometry = true;

struct Vertex
{
    // GL_C3F_V3F
	float r, g, b;
	float x, y, z;
};

//
// To understand how indexed geometry works, we must first build something
// which can be optimized through the use of indices.
//
// Below, is the vertex data for a simple multi-colored cube, which is defined
// as 6 individual quads, one quad for each of the cube's six sides. At first,
// this doesn’t seem too wasteful, but trust me it is.
//
// You see, we really only need 8 vertices to define a simple cube, but since 
// we're using a quad list, we actually have to repeat the usage of our 8 
// vertices 3 times each. To make this more understandable, I've actually 
// numbered the vertices below so you can see how the vertices get repeated 
// during the cube's definition.
//
// Note how the first 8 vertices are unique. Everting else after that is just
// a repeat of the first 8.
//

Vertex g_cubeVertices[] =
{
	// Quad 0
	{ 1.0f,0.0f,0.0f, -1.0f,-1.0f, 1.0f }, // 0 (unique)
	{ 0.0f,1.0f,0.0f,  1.0f,-1.0f, 1.0f }, // 1 (unique)
	{ 0.0f,0.0f,1.0f,  1.0f, 1.0f, 1.0f }, // 2 (unique)
	{ 1.0f,1.0f,0.0f, -1.0f, 1.0f, 1.0f }, // 3 (unique)

	// Quad 1
	{ 1.0f,0.0f,1.0f, -1.0f,-1.0f,-1.0f }, // 4 (unique)
	{ 0.0f,1.0f,1.0f, -1.0f, 1.0f,-1.0f }, // 5 (unique)
	{ 1.0f,1.0f,1.0f,  1.0f, 1.0f,-1.0f }, // 6 (unique)
	{ 1.0f,0.0f,0.0f,  1.0f,-1.0f,-1.0f }, // 7 (unique)

	// Quad 2
	{ 0.0f,1.0f,1.0f, -1.0f, 1.0f,-1.0f }, // 5 (start repeating here)
	{ 1.0f,1.0f,0.0f, -1.0f, 1.0f, 1.0f }, // 3 (repeat of vertex 3)
	{ 0.0f,0.0f,1.0f,  1.0f, 1.0f, 1.0f }, // 2 (repeat of vertex 2... etc.)
	{ 1.0f,1.0f,1.0f,  1.0f, 1.0f,-1.0f }, // 6

	// Quad 3
	{ 1.0f,0.0f,1.0f, -1.0f,-1.0f,-1.0f }, // 4
	{ 1.0f,0.0f,0.0f,  1.0f,-1.0f,-1.0f }, // 7
	{ 0.0f,1.0f,0.0f,  1.0f,-1.0f, 1.0f }, // 1
	{ 1.0f,0.0f,0.0f, -1.0f,-1.0f, 1.0f }, // 0

	// Quad 4
	{ 1.0f,0.0f,0.0f,  1.0f,-1.0f,-1.0f }, // 7
	{ 1.0f,1.0f,1.0f,  1.0f, 1.0f,-1.0f }, // 6
	{ 0.0f,0.0f,1.0f,  1.0f, 1.0f, 1.0f }, // 2
	{ 0.0f,1.0f,0.0f,  1.0f,-1.0f, 1.0f }, // 1

	// Quad 5
	{ 1.0f,0.0f,1.0f, -1.0f,-1.0f,-1.0f }, // 4
	{ 1.0f,0.0f,0.0f, -1.0f,-1.0f, 1.0f }, // 0
	{ 1.0f,1.0f,0.0f, -1.0f, 1.0f, 1.0f }, // 3
	{ 0.0f,1.0f,1.0f, -1.0f, 1.0f,-1.0f }  // 5
};

//
// Now, to save ourselves the bandwidth of passing a bunch or redundant vertices
// down the graphics pipeline, we shorten our vertex list and pass only the 
// unique vertices. We then create a indices array, which contains index values
// that reference vertices in our vertex array. 
//
// In other words, the vertex array doens't actually define our cube anymore, 
// it only holds the unique vertices; it's the indices array that now defines  
// the cube's geometry.
//

Vertex g_cubeVertices_indexed[] =
{
	{ 1.0f,0.0f,0.0f,  -1.0f,-1.0f, 1.0f }, // 0
	{ 0.0f,1.0f,0.0f,   1.0f,-1.0f, 1.0f }, // 1
	{ 0.0f,0.0f,1.0f,   1.0f, 1.0f, 1.0f }, // 2
	{ 1.0f,1.0f,0.0f,  -1.0f, 1.0f, 1.0f }, // 3
	{ 1.0f,0.0f,1.0f,  -1.0f,-1.0f,-1.0f }, // 4
	{ 0.0f,1.0f,1.0f,  -1.0f, 1.0f,-1.0f }, // 5
	{ 1.0f,1.0f,1.0f,   1.0f, 1.0f,-1.0f }, // 6
	{ 1.0f,0.0f,0.0f,   1.0f,-1.0f,-1.0f }, // 7
};

GLubyte g_cubeIndices[] =
{
	0, 1, 2, 3, // Quad 0
	4, 5, 6, 7, // Quad 1
	5, 3, 2, 6, // Quad 2
	4, 7, 1, 0, // Quad 3
	7, 6, 2, 1, // Quad 4
	4, 0, 3, 5  // Quad 5
};

//
// Note: While the cube above makes for a good example of how indexed geometry
//       works. There are many situations which can prevent you from using 
//       an indices array to its full potential. 
//
//       For example, if our cube required normals for lighting, things would 
//       become problematic since each vertex would be shared between three 
//       faces of the cube. This would not give you the lighting effect that 
//       you really want since the best you could do would be to average the 
//       normal's value between the three faces which used it.
//
//       Another example would be texture coordinates. If our cube required
//       unique texture coordinates for each face, you really wouldn’t gain 
//       much from using an indices array since each vertex would require a 
//       different texture coordinate depending on which face it was being 
//       used in.
//

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND g_hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void init(void);
void render(void);
void shutDown(void);

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
                             "OpenGL - Indexed Geometry",
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
				g_bUseIndexedGeometry = !g_bUseIndexedGeometry;
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
				g_fSpinZ -= (ptCurrentMousePosit.x - ptLastMousePosit.x);
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
			return DefWindowProc( g_hWnd, msg, wParam, lParam );
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

	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	glEnable( GL_DEPTH_TEST );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0f, 640.0f / 480.0f, 0.1f, 100.0f);
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown(void)	
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
// Name: render()
// Desc: 
//-----------------------------------------------------------------------------
void render( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
	glTranslatef( 0.0f, 0.0f, -5.0f );
	glRotatef( -g_fSpinY, 1.0f, 0.0f, 0.0f );
	glRotatef( -g_fSpinZ, 0.0f, 0.0f, 1.0f );

	if( g_bUseIndexedGeometry == true )
	{
		glInterleavedArrays( GL_C3F_V3F, 0, g_cubeVertices_indexed );
		glDrawElements( GL_QUADS, 24, GL_UNSIGNED_BYTE, g_cubeIndices );
	}
	else
	{
		glInterleavedArrays( GL_C3F_V3F, 0, g_cubeVertices );
		glDrawArrays( GL_QUADS, 0, 24 );
	}

	SwapBuffers( g_hDC );
}
