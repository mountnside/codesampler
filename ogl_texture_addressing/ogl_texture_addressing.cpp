//-----------------------------------------------------------------------------
//           Name: ogl_texture_addressing.cpp
//         Author: Kevin Harris
//  Last Modified: 02/01/05
//    Description: This sample demonstrates the two methods of texture  
//                 addressing that are available under OpenGL:
//
//                 GL_REPEAT
//                 GL_CLAMP
//                 GL_MIRRORED_REPEAT_ARB ( GL_ARB_texture_mirrored_repeat )
//                 GL_CLAMP_TO_BORDER_ARB ( GL_ARB_texture_border_clamp )
//                 GL_CLAMP_TO_EDGE		  ( GL_SGIS_texture_edge_clamp )
//
//   Control Keys: F1 - Changes addressing method for the S coordinates
//                 F2 - Changes addressing method for the T coordinates
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glaux.h>
#include "bitmap_fonts.h"
#include "resource.h"

// For convenience, this project ships with its own "glext.h" extension header 
// file. If you have trouble running this sample, it may be that this "glext.h" 
// file is defining something that your hardware doesn’t actually support. 
// Try recompiling the sample using your own local, vendor-specific "glext.h" 
// header file.

#include "glext.h"      // Sample's header file
//#include <GL/glext.h> // Your local header file


//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND   g_hWnd          = NULL;
HDC	   g_hDC           = NULL;
HGLRC  g_hRC           = NULL;
int    g_nWindowWidth  = 640;
int    g_nWindowHeight = 480;
GLuint g_textureID     = -1;

// Set the border color to purple.
float g_borderColor[] = { 1.0f, 0.0f, 1.0f, 1.0f };

struct Vertex
{
    float tu, tv;
    float x, y, z;
};

Vertex g_quadVertices[] =
{
    { 0.0f,0.0f, -1.0f,-1.0f, 0.0f },
    { 3.0f,0.0f,  1.0f,-1.0f, 0.0f },
    { 3.0f,3.0f,  1.0f, 1.0f, 0.0f },
    { 0.0f,3.0f, -1.0f, 1.0f, 0.0f }
};

enum AddressingMethods
{
	ADDRESSING_METHOD_REPEAT = 0,
	ADDRESSING_METHOD_CLAMP,
	ADDRESSING_METHOD_MIRRORED_REPEAT_ARB,
	ADDRESSING_METHOD_CLAMP_TO_BORDER_ARB,
	ADDRESSING_METHOD_CLAMP_TO_EDGE
};

int	 g_addressingMethod_S = ADDRESSING_METHOD_REPEAT;
int	 g_addressingMethod_T = ADDRESSING_METHOD_REPEAT;
bool g_bChangeAddressingMethod = true;

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
void setAddresingFor_S_Coordinate(void);
void setAddresingFor_T_Coordinate(void);

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
                             "OpenGL - Texture Addressing",
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
					++g_addressingMethod_S;
					if(g_addressingMethod_S > 4)
						g_addressingMethod_S = 0;
					g_bChangeAddressingMethod = true;
					break;

				case VK_F2:
					++g_addressingMethod_T;
					if(g_addressingMethod_T > 4)
						g_addressingMethod_T = 0;
					g_bChangeAddressingMethod = true;
					break;
			}
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
void loadTexture( void )	
{
	AUX_RGBImageRec *pTextureImage = auxDIBImageLoad( ".\\five.bmp" );

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
// Name : setAddresingFor_S_Coordinate()
// Desc : 
//-----------------------------------------------------------------------------
void setAddresingFor_S_Coordinate( void )
{
    if( g_addressingMethod_S == ADDRESSING_METHOD_REPEAT )
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    else if( g_addressingMethod_S == ADDRESSING_METHOD_CLAMP )
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    else if( g_addressingMethod_S == ADDRESSING_METHOD_MIRRORED_REPEAT_ARB )
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT_ARB);
    else if( g_addressingMethod_S == ADDRESSING_METHOD_CLAMP_TO_BORDER_ARB )
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER_ARB);
    else if( g_addressingMethod_S == ADDRESSING_METHOD_CLAMP_TO_EDGE )
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
}

//-----------------------------------------------------------------------------
// Name : setAddresingFor_T_Coordinate()
// Desc : 
//-----------------------------------------------------------------------------
void setAddresingFor_T_Coordinate( void )
{
    if( g_addressingMethod_T == ADDRESSING_METHOD_REPEAT )
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    else if( g_addressingMethod_T == ADDRESSING_METHOD_CLAMP )
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    else if( g_addressingMethod_T == ADDRESSING_METHOD_MIRRORED_REPEAT_ARB )
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT_ARB);
    else if( g_addressingMethod_T == ADDRESSING_METHOD_CLAMP_TO_BORDER_ARB )
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER_ARB);
    else if( g_addressingMethod_T == ADDRESSING_METHOD_CLAMP_TO_EDGE )
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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
	glTranslatef( 0.0f, 0.0f, -4.0f );

	if( g_bChangeAddressingMethod == true )
	{
		setAddresingFor_S_Coordinate();
		setAddresingFor_T_Coordinate();

        // Set the border color. This is used by GL_CLAMP_TO_BORDER_ARB.
        glTexParameterfv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, g_borderColor );

		g_bChangeAddressingMethod = false;
	}

	glBindTexture( GL_TEXTURE_2D, g_textureID );
    glInterleavedArrays( GL_T2F_V3F, 0, g_quadVertices );
    glDrawArrays( GL_QUADS, 0, 4 );

    //
    // Output the current settings...
    //

	static char strS[255];
	static char strT[255];

	if( g_addressingMethod_S == ADDRESSING_METHOD_REPEAT )
		sprintf( strS, "GL_TEXTURE_WRAP_S = GL_REPEAT    (Change: F1)" );
	else if( g_addressingMethod_S == ADDRESSING_METHOD_CLAMP )
		sprintf( strS, "GL_TEXTURE_WRAP_S = GL_CLAMP    (Change: F1)" );
	else if( g_addressingMethod_S == ADDRESSING_METHOD_MIRRORED_REPEAT_ARB )
		sprintf( strS, "GL_TEXTURE_WRAP_S = GL_MIRRORED_REPEAT_ARB    (Change: F1)" );
	else if( g_addressingMethod_S == ADDRESSING_METHOD_CLAMP_TO_BORDER_ARB )
		sprintf( strS, "GL_TEXTURE_WRAP_S = GL_CLAMP_TO_BORDER_ARB    (Change: F1)" );
	else if( g_addressingMethod_S == ADDRESSING_METHOD_CLAMP_TO_EDGE )
		sprintf( strS, "GL_TEXTURE_WRAP_S = GL_CLAMP_TO_EDGE    (Change: F1)" );

	if( g_addressingMethod_T == ADDRESSING_METHOD_REPEAT )
		sprintf( strT, "GL_TEXTURE_WRAP_T = GL_REPEAT    (Change: F2)" );
	else if( g_addressingMethod_T == ADDRESSING_METHOD_CLAMP )
		sprintf( strT, "GL_TEXTURE_WRAP_T = GL_CLAMP    (Change: F2)" );
	else if( g_addressingMethod_T == ADDRESSING_METHOD_MIRRORED_REPEAT_ARB )
		sprintf( strT, "GL_TEXTURE_WRAP_T = GL_MIRRORED_REPEAT_ARB    (Change: F2)" );
	else if( g_addressingMethod_T == ADDRESSING_METHOD_CLAMP_TO_BORDER_ARB )
		sprintf( strT, "GL_TEXTURE_WRAP_T = GL_CLAMP_TO_BORDER_ARB    (Change: F2)" );
	else if( g_addressingMethod_T == ADDRESSING_METHOD_CLAMP_TO_EDGE )
		sprintf( strT, "GL_TEXTURE_WRAP_T = GL_CLAMP_TO_EDGE    (Change: F2)" );

	beginRenderText( g_nWindowWidth, g_nWindowHeight );
	{
		glColor3f( 1.0f, 1.0f, 1.0f );
		renderText( 5, 15, BITMAP_FONT_TYPE_HELVETICA_12, strS );
		renderText( 5, 30, BITMAP_FONT_TYPE_HELVETICA_12, strT );
	}
	endRenderText();

	SwapBuffers( g_hDC );
}
