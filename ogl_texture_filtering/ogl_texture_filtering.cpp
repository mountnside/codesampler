//-----------------------------------------------------------------------------
//           Name: ogl_texture_filtering.cpp
//         Author: Kevin Harris
//  Last Modified: 02/01/05
//    Description: This sample demonstrates how to filter textures with 
//                 OpenGL.
//
//                 OpenGL texture filtering is controlled by calling 
//                 glTexParameteri() and setting either GL_TEXTURE_MIN_FILTER, 
//                 or GL_TEXTURE_MAG_FILTER to one of the following filtering 
//                 modes:
//
//                 GL_NEAREST
//                 GL_LINEAR
//                 GL_NEAREST_MIPMAP_NEAREST
//                 GL_LINEAR_MIPMAP_NEAREST
//                 GL_NEAREST_MIPMAP_LINEAR
//                 GL_LINEAR_MIPMAP_LINEAR
//
//  Control Keys: F1 - Change Min filter
//                F2 - Change Mag filter
//                Left Mouse Button - Spin the view.
//                Up Arrow - Move the test quad away
//                Down Arrow - Move the test quad away
// ----------------------------------------------------------------------------
//
// GL_TEXTURE_MIN_FILTER
//
// Accepted Values:
//
//  GL_NEAREST
//  GL_LINEAR
//  GL_NEAREST_MIPMAP_NEAREST
//  GL_LINEAR_MIPMAP_NEAREST
//  GL_NEAREST_MIPMAP_LINEAR
//  GL_LINEAR_MIPMAP_LINEAR
//
// Default Value:
//
//  GL_NEAREST_MIPMAP_LINEAR
//
// ----------------------------------------------------------------------------
// 
// GL_TEXTURE_MAG_FILTER
//
// Accepted Values:
//
//  GL_NEAREST
//  GL_LINEAR
//
// Default Value:
//
//  GL_LINEAR
//
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <mmsystem.h>
#include <stdlib.h>
#include <stdio.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glaux.h>
#include "resource.h"
#include "bitmap_fonts.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND   g_hWnd          = NULL;
HDC	   g_hDC           = NULL;
HGLRC  g_hRC           = NULL;
int    g_nWindowWidth  = 640;
int    g_nWindowHeight = 480;
GLuint g_textureID     = -1;

float g_fDistance = -4.0f;
float g_fSpinX    =  0.0f;
float g_fSpinY    =  0.0f;

struct Vertex
{
    float tu, tv;
    float x, y, z;
};

Vertex g_quadVertices[] =
{
    { 0.0f,0.0f, -1.0f,-1.0f, 0.0f },
    { 1.0f,0.0f,  1.0f,-1.0f, 0.0f },
    { 1.0f,1.0f,  1.0f, 1.0f, 0.0f },
    { 0.0f,1.0f, -1.0f, 1.0f, 0.0f }
};

enum FilterTypes
{
	FILTER_TYPE_NEAREST = 0,
	FILTER_TYPE_LINEAR,
    FILTER_TYPE_NEAREST_MIPMAP_NEAREST,
    FILTER_TYPE_LINEAR_MIPMAP_NEAREST,
    FILTER_TYPE_NEAREST_MIPMAP_LINEAR,
    FILTER_TYPE_LINEAR_MIPMAP_LINEAR
};

int	 g_MinFilterType  = FILTER_TYPE_NEAREST;
int	 g_MagFilterType  = FILTER_TYPE_NEAREST;
bool g_bChangeFilters = true;

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void loadTexture(void);
void init(void);
void render(void);
void shutDown(void);
void setMagnificationFilter(void);
void setMinificationFilter(void);
void setMipMapFilter(void);

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
                             "OpenGL - Texturing Filtering",
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
					++g_MinFilterType;
					if(g_MinFilterType > 5)
						g_MinFilterType = 0;
					g_bChangeFilters = true;
					break;

                case VK_F2:
					++g_MagFilterType;
					if(g_MagFilterType > 1)
						g_MagFilterType = 0;
					g_bChangeFilters = true;
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
			g_nWindowWidth  = LOWORD(lParam); 
			g_nWindowHeight = HIWORD(lParam);
			glViewport(0, 0, g_nWindowWidth, g_nWindowHeight);

			glMatrixMode( GL_PROJECTION );
			glLoadIdentity();
			gluPerspective( 45.0, (GLdouble)g_nWindowWidth / (GLdouble)g_nWindowHeight, 0.1, 100.0);
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
// Name: loadTexture()
// Desc: 
//-----------------------------------------------------------------------------
void loadTexture(void)	
{
	AUX_RGBImageRec *pTextureImage = auxDIBImageLoad( ".\\test.bmp" );

    if( pTextureImage != NULL )
	{
        glGenTextures( 1, &g_textureID );

		glBindTexture( GL_TEXTURE_2D, g_textureID );

        gluBuild2DMipmaps(GL_TEXTURE_2D, 3, pTextureImage->sizeX, pTextureImage->sizeY, 
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
	glEnable(GL_TEXTURE_2D);

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0, (GLdouble)g_nWindowWidth / g_nWindowHeight, 0.1, 100.0 );
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
// Name : setMinificationFilter()
// Desc : 
//-----------------------------------------------------------------------------
void setMinificationFilter( void )
{
	if( g_MinFilterType == 0 )
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	
	if( g_MinFilterType == 1 )
		glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR );

	if( g_MinFilterType == 2 )
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);

	if( g_MinFilterType == 3 )
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

    if( g_MinFilterType == 4 )
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);

	if( g_MinFilterType == 5 )
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
}

//-----------------------------------------------------------------------------
// Name : setMagnificationFilter()
// Desc : 
//-----------------------------------------------------------------------------
void setMagnificationFilter( void )
{
	if( g_MagFilterType == 0 )
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	if( g_MagFilterType == 1 )
		glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR );
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

	if( g_bChangeFilters == true )
	{
		setMinificationFilter();
		setMagnificationFilter();
		g_bChangeFilters = false;
	}

	glBindTexture( GL_TEXTURE_2D, g_textureID );
	
    glInterleavedArrays( GL_T2F_V3F, 0, g_quadVertices );
    glDrawArrays( GL_QUADS, 0, 4 );

    //
    // Output the current settings...
    //

	static char strMinFilter[255];
	static char strMagFilter[255];

	if( g_MinFilterType == FILTER_TYPE_NEAREST )
		sprintf( strMinFilter, "GL_TEXTURE_MIN_FILTER = GL_NEAREST    (Change: F1)" );
	else if( g_MinFilterType == FILTER_TYPE_LINEAR )
		sprintf( strMinFilter, "GL_TEXTURE_MIN_FILTER = GL_LINEAR    (Change: F1)" );
	else if( g_MinFilterType == FILTER_TYPE_NEAREST_MIPMAP_NEAREST )
		sprintf( strMinFilter, "GL_TEXTURE_MIN_FILTER = GL_NEAREST_MIPMAP_NEAREST    (Change: F1)" );
	else if( g_MinFilterType == FILTER_TYPE_LINEAR_MIPMAP_NEAREST )
		sprintf( strMinFilter, "GL_TEXTURE_MIN_FILTER = GL_LINEAR_MIPMAP_NEAREST    (Change: F1)" );
	else if( g_MinFilterType == FILTER_TYPE_NEAREST_MIPMAP_LINEAR )
		sprintf( strMinFilter, "GL_TEXTURE_MIN_FILTER = GL_NEAREST_MIPMAP_LINEAR    (Change: F1)" );
    else if( g_MinFilterType == FILTER_TYPE_LINEAR_MIPMAP_LINEAR )
        sprintf( strMinFilter, "GL_TEXTURE_MIN_FILTER = GL_LINEAR_MIPMAP_LINEAR    (Change: F1)" );

	if( g_MagFilterType == FILTER_TYPE_NEAREST )
		sprintf( strMagFilter, "GL_TEXTURE_MAG_FILTER = GL_NEAREST    (Change: F2)" );
	else if( g_MagFilterType == FILTER_TYPE_LINEAR )
		sprintf( strMagFilter, "GL_TEXTURE_MAG_FILTER = GL_LINEAR    (Change: F2)" );

	beginRenderText( g_nWindowWidth, g_nWindowHeight );
	{
		glColor3f( 1.0f, 1.0f, 1.0f );
		renderText( 5, 15, BITMAP_FONT_TYPE_HELVETICA_12, strMinFilter );
		renderText( 5, 30, BITMAP_FONT_TYPE_HELVETICA_12, strMagFilter );
	}
	endRenderText();

	SwapBuffers( g_hDC );
}
