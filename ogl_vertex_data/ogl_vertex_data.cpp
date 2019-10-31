//-----------------------------------------------------------------------------
//           Name: ogl_vertexdata.cpp
//         Author: Kevin Harris
//  Last Modified: 02/01/05
//    Description: This sample demonstrates how to create 3D geometry with 
//                 OpenGL by loading vertex data into a Vertex Array.
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <mmsystem.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glaux.h>
#include "resource.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND   g_hWnd      = NULL;
HDC	   g_hDC       = NULL;
HGLRC  g_hRC       = NULL;
GLuint g_textureID = -1;

float  g_fElpasedTime;
double g_dCurrentTime;
double g_dLastTime;

struct Vertex
{
    float tu, tv;
    float x, y, z;
};

Vertex g_cubeVertices[] =
{
	{ 0.0f,0.0f, -1.0f,-1.0f, 1.0f },
	{ 1.0f,0.0f,  1.0f,-1.0f, 1.0f },
	{ 1.0f,1.0f,  1.0f, 1.0f, 1.0f },
	{ 0.0f,1.0f, -1.0f, 1.0f, 1.0f },

	{ 1.0f,0.0f, -1.0f,-1.0f,-1.0f },
	{ 1.0f,1.0f, -1.0f, 1.0f,-1.0f },
	{ 0.0f,1.0f,  1.0f, 1.0f,-1.0f },
	{ 0.0f,0.0f,  1.0f,-1.0f,-1.0f },

	{ 0.0f,1.0f, -1.0f, 1.0f,-1.0f },
	{ 0.0f,0.0f, -1.0f, 1.0f, 1.0f },
	{ 1.0f,0.0f,  1.0f, 1.0f, 1.0f },
	{ 1.0f,1.0f,  1.0f, 1.0f,-1.0f },

	{ 1.0f,1.0f, -1.0f,-1.0f,-1.0f },
	{ 0.0f,1.0f,  1.0f,-1.0f,-1.0f },
	{ 0.0f,0.0f,  1.0f,-1.0f, 1.0f },
	{ 1.0f,0.0f, -1.0f,-1.0f, 1.0f },

	{ 1.0f,0.0f,  1.0f,-1.0f,-1.0f },
	{ 1.0f,1.0f,  1.0f, 1.0f,-1.0f },
	{ 0.0f,1.0f,  1.0f, 1.0f, 1.0f },
	{ 0.0f,0.0f,  1.0f,-1.0f, 1.0f },

	{ 0.0f,0.0f, -1.0f,-1.0f,-1.0f },
	{ 1.0f,0.0f, -1.0f,-1.0f, 1.0f },
	{ 1.0f,1.0f, -1.0f, 1.0f, 1.0f },
	{ 0.0f,1.0f, -1.0f, 1.0f,-1.0f }
};

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND g_hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void loadTexture(void);
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
                             "OpenGL - Vertex Data",
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
            g_dCurrentTime = timeGetTime();
            g_fElpasedTime = (float)((g_dCurrentTime - g_dLastTime) * 0.001);
            g_dLastTime    = g_dCurrentTime;

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
LRESULT CALLBACK WindowProc( HWND   g_hWnd, 
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
			return DefWindowProc( g_hWnd, msg, wParam, lParam );
		}
		break;
	}

	return 0;
}

//-----------------------------------------------------------------------------
// Name: loadTexture()
// Desc: 
//-----------------------------------------------------------------------------
void loadTexture( void )	
{
	AUX_RGBImageRec *pTextureImage = auxDIBImageLoad( ".\\test.bmp" );

    if( pTextureImage != NULL )
	{
		glGenTextures( 1, &g_textureID );

		glBindTexture( GL_TEXTURE_2D, g_textureID );

		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

		glTexImage2D( GL_TEXTURE_2D, 0, 3,pTextureImage->sizeX,pTextureImage->sizeY, 0,
			GL_RGB, GL_UNSIGNED_BYTE,pTextureImage->data );
	}

	if( pTextureImage != NULL )
	{
		if( pTextureImage->data != NULL )
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
    pfd.dwFlags    = PFD_DRAW_TO_WINDOW |PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 16;
    pfd.cDepthBits = 16;
	
	g_hDC = GetDC( g_hWnd );
	PixelFormat = ChoosePixelFormat( g_hDC, &pfd );
	SetPixelFormat( g_hDC, PixelFormat, &pfd);
	g_hRC = wglCreateContext( g_hDC );
	wglMakeCurrent( g_hDC, g_hRC );

	loadTexture();

	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	glEnable( GL_TEXTURE_2D );
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
// Name: render()
// Desc: 
//-----------------------------------------------------------------------------
void render(void)	
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
    static float fXrot = 0.0f;
    static float fYrot = 0.0f;
    static float fZrot = 0.0f;

    fXrot += 10.1f * g_fElpasedTime;
    fYrot += 10.2f * g_fElpasedTime;
    fZrot += 10.3f * g_fElpasedTime;

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glTranslatef( 0.0f, 0.0f, -5.0f );
    glRotatef( fXrot, 1.0f, 0.0f, 0.0f );
    glRotatef( fYrot, 0.0f, 1.0f, 0.0f );
    glRotatef( fZrot, 0.0f, 0.0f, 1.0f );

    glBindTexture( GL_TEXTURE_2D, g_textureID );
    glInterleavedArrays( GL_T2F_V3F, 0, g_cubeVertices );
    glDrawArrays( GL_QUADS, 0, 24 );

	SwapBuffers( g_hDC );
}
