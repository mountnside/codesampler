//-----------------------------------------------------------------------------
//           Name: ogl_stanford_bunny.cpp
//         Author: Kevin Harris
//  Last Modified: 02/01/05
//    Description: This sample demonstrates how to make use of the useful 
//                 Stanford Bunny model or data set for testing and prototyping 
//                 purposes.
//
//   Control Keys: Left Mouse Button - Spin the view
//                 Up Arrow - Move the bunny model away
//                 Down Arrow - Move the bunny model away
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "resource.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND   g_hWnd = NULL;
HDC	   g_hDC  = NULL;
HGLRC  g_hRC  = NULL;

GLuint g_stanfordBunnyDL = -1;

bool g_bRenderInWireFrame = false;

float g_fDistance = -2.0f;
float g_fSpinX    =  0.0f;
float g_fSpinY    =  0.0f;

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND g_hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void init(void);
void render(void);
void shutDown(void);

extern GLint Gen3DObjectList();

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
		                    "OpenGL - Stanford Bunny Data Set",
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
				g_bRenderInWireFrame = !g_bRenderInWireFrame;
				if( g_bRenderInWireFrame == true )
					glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
				else
					glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
				break;

			case 38: // Up Arrow Key
				g_fDistance -= 0.1f;
				break;

			case 40: // Down Arrow Key
				g_fDistance += 0.1f;
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

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0f, 640.0f / 480.0f, 0.1f, 100.0f);

	glEnable( GL_LIGHT0 );
	GLfloat position_light0[] = { -1.0f, 0.0f, 1.0f, 0.0f };
	GLfloat diffuse_light0[]  = {  1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat specular_light0[] = {  1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv( GL_LIGHT0, GL_POSITION, position_light0 );
	glLightfv( GL_LIGHT0, GL_DIFFUSE,  diffuse_light0 );
	glLightfv( GL_LIGHT0, GL_SPECULAR, specular_light0 );

	GLfloat ambient_lightModel[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	glLightModelfv( GL_LIGHT_MODEL_AMBIENT, ambient_lightModel );

	//
	// Generate a display list to hold the Stanford Bunny data set...
	//

	g_stanfordBunnyDL = Gen3DObjectList();
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
	glTranslatef( 0.0f, 0.0f, g_fDistance );
	glRotatef( -g_fSpinY, 1.0f, 0.0f, 0.0f );
	glRotatef( -g_fSpinX, 0.0f, 1.0f, 0.0f );

	glCallList( g_stanfordBunnyDL );
	
	SwapBuffers( g_hDC );
}
