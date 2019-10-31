//-----------------------------------------------------------------------------
//           Name: ogl_glew_demo.cpp
//         Author: Kevin Harris
//  Last Modified: 03/09/05
//    Description: Test sample for exercising GLEW 1.3.1.
//
//                 http://glew.sourceforge.net/
//
//   Control Keys: F1 = Toggle between GL_FUNC_SUBTRACT_EXT and 
//                      GL_FUNC_ADD_EXT, which are defined by the OpenGL
//                      extension, GLEW_EXT_blend_minmax.
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "resource.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND   g_hWnd          = NULL;
HDC	   g_hDC           = NULL;
HGLRC  g_hRC           = NULL;

float g_fSpinX =  0.0f;
float g_fSpinY =  0.0f;

bool g_bUseBlendEquation = false;

struct Vertex
{
    //GL_T2F_C4F_N3F_V3F
    float tu, tv;
    float r, g, b, a;
    float nx, ny, nz;
    float x, y, z;
};

Vertex g_whiteSemiTransparentQuad[] =
{
    // tu,  tv     r    g    b    a      nx   ny  nz     x     y     z 
    { 0.0f,0.0f,  1.0f,1.0f,1.0f,0.5f,  0.0f,0.0f,1.0, -1.0f,-1.0f, 0.0f },
    { 1.0f,0.0f,  1.0f,1.0f,1.0f,0.5f,  0.0f,0.0f,1.0,  1.0f,-1.0f, 0.0f },
    { 1.0f,1.0f,  1.0f,1.0f,1.0f,0.5f,  0.0f,0.0f,1.0,  1.0f, 1.0f, 0.0f },
    { 0.0f,1.0f,  1.0f,1.0f,1.0f,0.5f,  0.0f,0.0f,1.0, -1.0f, 1.0f, 0.0f },
};

Vertex g_redOpaqueQuad[] =
{
    // tu,  tv     r    g    b    a      nx   ny  nz     x     y     z 
    { 0.0f,0.0f,  1.0f,0.0f,0.0f,1.0f,  0.0f,0.0f,1.0, -1.0f,-1.0f, 0.0f },
    { 1.0f,0.0f,  1.0f,0.0f,0.0f,1.0f,  0.0f,0.0f,1.0,  1.0f,-1.0f, 0.0f },
    { 1.0f,1.0f,  1.0f,0.0f,0.0f,1.0f,  0.0f,0.0f,1.0,  1.0f, 1.0f, 0.0f },
    { 0.0f,1.0f,  1.0f,0.0f,0.0f,1.0f,  0.0f,0.0f,1.0, -1.0f, 1.0f, 0.0f },
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
                             "OpenGL - GLEW Demo Sample",
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
					g_bUseBlendEquation = !g_bUseBlendEquation;
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
	glEnable(GL_TEXTURE_2D);

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0f, 640.0f / 480.0f, 0.1f, 100.0f);

    //
    // Init GLEW and verify tat we got the extension we wanted...
    //

    GLenum err = glewInit();

    if( err != GLEW_OK )
    {
        static char str[255];
        sprintf( str, "Error: %s\n", glewGetErrorString(err) );
        MessageBox( NULL, str, "ERROR", MB_OK|MB_ICONEXCLAMATION );
    }
    else
    {
        static char str[255];
        sprintf( str, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION) );
        MessageBox( NULL, str, "SUCCESS", MB_OK|MB_ICONINFORMATION );
    }

    if( GLEW_EXT_blend_minmax )
    {
        static char str[255];
        sprintf( str, "Status: GL_EXT_blend_minmax - Supported!" );
        MessageBox( NULL, str, "SUCCESS", MB_OK|MB_ICONINFORMATION );
    }
    else
    {
        static char str[255];
        sprintf( str, "Error: GL_EXT_blend_minmax - Not Supported!" );
        MessageBox( NULL, str, "ERROR", MB_OK|MB_ICONEXCLAMATION );
    }
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
void render(void)	
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
	glTranslatef( 0.0f, 0.0f, -4.0f );

    glEnable( GL_BLEND );
    glDisable( GL_DEPTH_TEST );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE );

    if( g_bUseBlendEquation )
        glBlendEquationEXT( GL_FUNC_SUBTRACT_EXT );
    else
        glBlendEquationEXT( GL_FUNC_ADD_EXT );

    glPushMatrix();
    {
        glTranslatef( 0.0f, 0.0f, 0.5f );
        glRotatef( -g_fSpinX, 0.0f, 0.0f, 1.0f );
        glInterleavedArrays( GL_T2F_C4F_N3F_V3F, 0, g_redOpaqueQuad );
        glDrawArrays( GL_QUADS, 0, 4 );
    }
    glPopMatrix();

    glPushMatrix();
    {
        glRotatef( g_fSpinY, -1.0f, 0.0f, 0.0f );
        glInterleavedArrays( GL_T2F_C4F_N3F_V3F, 0, g_whiteSemiTransparentQuad );
        glDrawArrays( GL_QUADS, 0, 4 );
    }
    glPopMatrix();

	SwapBuffers( g_hDC );
}
