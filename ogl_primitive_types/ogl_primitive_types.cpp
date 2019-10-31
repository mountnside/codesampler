//-----------------------------------------------------------------------------
//           Name: ogl_primitive_types.cpp
//         Author: Kevin Harris
//  Last Modified: 02/01/05
//    Description: This sample demonstrates how to properly use all the 
//                 primitive types available under OpenGL.
//
//                 The primitive types are:
//
//                 GL_POINTS
//                 GL_LINES
//                 GL_LINE_STRIP
//                 GL_LINE_LOOP
//                 GL_TRIANGLES
//                 GL_TRIANGLE_STRIP
//                 GL_TRIANGLE_FAN
//                 GL_QUADS
//                 GL_QUAD_STRIP
//                 GL_POLYGON
//
//   Control Keys: F1 - Switch the primitive type to be rendered.
//                 F2 - Toggle wire-frame mode.
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <mmsystem.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "resource.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND  g_hWnd = NULL;
HDC	  g_hDC  = NULL;
HGLRC g_hRC  = NULL;

bool g_bRenderInWireFrame = false;
GLenum g_currentPrimitive = GL_POLYGON;

struct Vertex
{
    unsigned char r, g, b, a;
    float x, y, z;
};

Vertex g_points[] = 
{
    { 255,   0,   0, 255,  0.0f, 0.0f, 0.0f },
    {   0, 255,   0, 255,  0.5f, 0.0f, 0.0f },
    {   0,   0, 255, 255, -0.5f, 0.0f, 0.0f },
	{ 255, 255,   0, 255,  0.0f,-0.5f, 0.0f },
    { 255,   0, 255, 255,  0.0f, 0.5f, 0.0f }
};

Vertex g_lines[] = 
{
    { 255,   0,   0, 255, -1.0f, 0.0f, 0.0f },  // Line #1
    { 255,   0,   0, 255,  0.0f, 1.0f, 0.0f },

    {   0, 255,   0, 255,  0.5f, 1.0f, 0.0f },  // Line #2
    {   0, 255,   0, 255,  0.5f,-1.0f, 0.0f },

	{   0,   0, 255, 255,  1.0f, -0.5f, 0.0f }, // Line #3
    {   0,   0, 255, 255, -1.0f, -0.5f, 0.0f }
};

//
// Note: 
// 
// We'll use the same vertices for both the line-strip and line-loop 
// demonstrations. The difference between the two primitive types will not be 
// noticeable until the vertices are rendered. Line-loop works exactly like
// line-strip except it creates a closed loop by automatically connecting 
// the last vertex to the first with a line segment.
//

Vertex g_lineStrip_and_lineLoop[] = 
{
    { 255,   0,   0, 255,  0.5f, 0.5f, 0.0f },
    {   0, 255,   0, 255,  1.0f, 0.0f, 0.0f },
    {   0,   0, 255, 255,  0.0f,-1.0f, 0.0f },
    { 255, 255,   0, 255, -1.0f, 0.0f, 0.0f },
	{ 255,   0,   0, 255,  0.0f, 0.0f, 0.0f },
    { 255,   0, 255, 255,  0.0f, 1.0f, 0.0f }
};

Vertex g_triangles[] =
{
	{ 255,   0,   0, 255, -1.0f, 0.0f, 0.0f }, // Triangle #1
	{   0,   0, 255, 255,  1.0f, 0.0f, 0.0f },
	{   0, 255,   0, 255,  0.0f, 1.0f, 0.0f },
	

	{ 255, 255,   0, 255, -0.5f,-1.0f, 0.0f }, // Triangle #2
	{ 255,   0,   0, 255,  0.5f,-1.0f, 0.0f },
	{   0, 255, 255, 255,  0.0f,-0.5f, 0.0f }
};

Vertex g_triangleStrip[] = 
{
    { 255,   0,   0, 255, -2.0f, 0.0f, 0.0f },
	{   0,   0, 255, 255, -1.0f, 0.0f, 0.0f },	
    {   0, 255,   0, 255, -1.0f, 1.0f, 0.0f },
	{ 255,   0, 255, 255,  0.0f, 0.0f, 0.0f },
    { 255, 255,   0, 255,  0.0f, 1.0f, 0.0f },
    { 255,   0,   0, 255,  1.0f, 0.0f, 0.0f },
    {   0, 255, 255, 255,  1.0f, 1.0f, 0.0f },
	{   0, 255,   0, 255,  2.0f, 1.0f, 0.0f }
};

Vertex g_triangleFan[] = 
{
    { 255,   0,   0, 255,  0.0f,-1.0f, 0.0f },
	{   0, 255, 255, 255,  1.0f, 0.0f, 0.0f },
    { 255,   0, 255, 255,  0.5f, 0.5f, 0.0f },
    { 255, 255,   0, 255,  0.0f, 1.0f, 0.0f },
    {   0,   0, 255, 255, -0.5f, 0.5f, 0.0f },
    {   0, 255,   0, 255, -1.0f, 0.0f, 0.0f }
};

Vertex g_quads[] =
{
    { 255,   0,   0, 255,  -0.5f,-0.5f, 0.0f },  // Quad #1
    {   0, 255,   0, 255,   0.5f,-0.5f, 0.0f },
    {   0,   0, 255, 255,   0.5f, 0.5f, 0.0f },
    { 255, 255,   0, 255,  -0.5f, 0.5f, 0.0f },

	{ 255,   0, 255, 255,  -1.5f, -1.0f, 0.0f }, // Quad #2
    {   0, 255, 255, 255,  -1.0f, -1.0f, 0.0f },
    { 255,   0,   0, 255,  -1.0f,  1.5f, 0.0f },
    {   0, 255,   0, 255,  -1.5f,  1.5f, 0.0f },

	{   0,   0, 255, 255,  1.0f, -0.2f, 0.0f },  // Quad #3
    { 255, 255,   0, 255,  2.0f, -0.2f, 0.0f },
    {   0, 255, 255, 255,  2.0f,  0.2f, 0.0f },
    { 255,   0, 255, 255,  1.0f,  0.2f, 0.0f }
};

Vertex g_quadStrip[] =
{
    { 255,   0,   0, 255,  -0.5f,-1.5f, 0.0f },
    {   0, 255,   0, 255,   0.5f,-1.5f, 0.0f },
    {   0,   0, 255, 255,  -0.2f,-0.5f, 0.0f },
    { 255, 255,   0, 255,   0.2f,-0.5f, 0.0f },
	{ 255,   0, 255, 255,  -0.5f, 0.5f, 0.0f },
    {   0, 255, 255, 255,   0.5f, 0.5f, 0.0f },
    { 255,   0,   0, 255,  -0.4f, 1.5f, 0.0f },
    {   0, 255,   0, 255,   0.4f, 1.5f, 0.0f },
};

Vertex g_polygon[] =
{
    { 255,   0,   0, 255,  -0.3f,-1.5f, 0.0f },
    {   0, 255,   0, 255,   0.3f,-1.5f, 0.0f },
    {   0,   0, 255, 255,   0.5f, 0.5f, 0.0f },
	{ 255, 255,   0, 255,   0.0f, 1.5f, 0.0f },
    { 255,   0, 255, 255,  -0.5f, 0.5f, 0.0f }
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
                             "OpenGL - Primitve Types",
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
				{
					if( g_currentPrimitive == GL_POINTS )
						g_currentPrimitive = GL_LINES;
					else if( g_currentPrimitive == GL_LINES )
						g_currentPrimitive = GL_LINE_STRIP;
					else if( g_currentPrimitive == GL_LINE_STRIP )
						g_currentPrimitive = GL_LINE_LOOP;
					else if( g_currentPrimitive == GL_LINE_LOOP )
						g_currentPrimitive = GL_TRIANGLES;
					else if( g_currentPrimitive == GL_TRIANGLES )
						g_currentPrimitive = GL_TRIANGLE_STRIP;
					else if( g_currentPrimitive == GL_TRIANGLE_STRIP )
						g_currentPrimitive = GL_TRIANGLE_FAN;
					else if( g_currentPrimitive == GL_TRIANGLE_FAN )
						g_currentPrimitive = GL_QUADS;
					else if( g_currentPrimitive == GL_QUADS )
						g_currentPrimitive = GL_QUAD_STRIP;
					else if( g_currentPrimitive == GL_QUAD_STRIP )
						g_currentPrimitive = GL_POLYGON;
					else if( g_currentPrimitive == GL_POLYGON )
						g_currentPrimitive = GL_POINTS;
				}
				break;

				case VK_F2:
                    g_bRenderInWireFrame = !g_bRenderInWireFrame;
                    if( g_bRenderInWireFrame == true )
                        glPolygonMode( GL_FRONT, GL_LINE );
                    else
                        glPolygonMode( GL_FRONT, GL_FILL );
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

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0f, 640.0f / 480.0f, 0.1f, 100.0f);

	glCullFace( GL_BACK );
	glEnable( GL_CULL_FACE );
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
// Name: render()
// Desc: 
//-----------------------------------------------------------------------------
void render( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
	glTranslatef( 0.0f, 0.0f, -5.0f );
	
	switch( g_currentPrimitive )
	{
		case GL_POINTS:
			glInterleavedArrays( GL_C4UB_V3F, 0, g_points );
			glDrawArrays( GL_POINTS, 0, 5 );
		break;

		case GL_LINES:
			glInterleavedArrays( GL_C4UB_V3F, 0, g_lines );
			glDrawArrays( GL_LINES, 0, 6 );
		break;

		case GL_LINE_STRIP:
			glInterleavedArrays( GL_C4UB_V3F, 0, g_lineStrip_and_lineLoop );
			glDrawArrays( GL_LINE_STRIP, 0, 6 );
		break;

		case GL_LINE_LOOP:
			glInterleavedArrays( GL_C4UB_V3F, 0, g_lineStrip_and_lineLoop );
			glDrawArrays( GL_LINE_LOOP, 0, 6 );
		break;

		case GL_TRIANGLES:
			glInterleavedArrays( GL_C4UB_V3F, 0, g_triangles );
			glDrawArrays( GL_TRIANGLES, 0, 6 );
		break;

		case GL_TRIANGLE_STRIP:
			glInterleavedArrays( GL_C4UB_V3F, 0, g_triangleStrip );
			glDrawArrays( GL_TRIANGLE_STRIP, 0, 8 );
		break;

		case GL_TRIANGLE_FAN:
			glInterleavedArrays( GL_C4UB_V3F, 0, g_triangleFan );
			glDrawArrays( GL_TRIANGLE_FAN, 0, 6 );
		break;

		case GL_QUADS:
			glInterleavedArrays( GL_C4UB_V3F, 0, g_quads );
			glDrawArrays( GL_QUADS, 0, 12 );
		break;

		case GL_QUAD_STRIP:
			glInterleavedArrays( GL_C4UB_V3F, 0, g_quadStrip );
			glDrawArrays( GL_QUAD_STRIP, 0, 8 );
		break;

		case GL_POLYGON:
			glInterleavedArrays( GL_C4UB_V3F, 0, g_polygon );
			glDrawArrays( GL_POLYGON, 0, 5 );
		break;

		default: 
			break;
	}

	SwapBuffers( g_hDC );
}

