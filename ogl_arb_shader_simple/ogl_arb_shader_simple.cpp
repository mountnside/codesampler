//-----------------------------------------------------------------------------
//           Name: ogl_arb_shader_simple.cpp
//         Author: Kevin Harris
//  Last Modified: 02/01/05
//    Description: This sample demonstrates how to write a vertex shader using 
//                 OpenGL's ARB_vertex_program extension.
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <sys/stat.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "resource.h"

//-----------------------------------------------------------------------------
// FUNCTION POINTERS FOR OPENGL EXTENSIONS
//-----------------------------------------------------------------------------

// For convenience, this project ships with its own "glext.h" extension header 
// file. If you have trouble running this sample, it may be that this "glext.h" 
// file is defining something that your hardware doesn’t actually support. 
// Try recompiling the sample using your own local, vendor-specific "glext.h" 
// header file.

#include "glext.h"      // Sample's header file
//#include <GL/glext.h> // Your local header file

PFNGLGENPROGRAMSARBPROC           glGenProgramsARB           = NULL;
PFNGLDELETEPROGRAMSARBPROC        glDeleteProgramsARB        = NULL;
PFNGLBINDPROGRAMARBPROC           glBindProgramARB           = NULL;
PFNGLPROGRAMSTRINGARBPROC         glProgramStringARB         = NULL;
PFNGLPROGRAMENVPARAMETER4FARBPROC glProgramEnvParameter4fARB = NULL;

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HDC	      g_hDC       = NULL;
HGLRC     g_hRC       = NULL;
HWND      g_hWnd      = NULL;
HINSTANCE g_hInstance = NULL;
GLuint    g_vertexProgramID;

float g_fSpinX = 0.0f;
float g_fSpinY = 0.0f;

struct Vertex
{
    unsigned char r, g, b, a;
    float x, y, z;
};

Vertex g_quadVertices[] =
{
//     r    g    b    a      x     y     z
    { 255, 255,   0, 255, -1.0f,-1.0f, 0.0f, }, // Bottom-Left,  color = yellow
    { 255,   0,   0, 255,  1.0f,-1.0f, 0.0f, }, // Bottom-Right, color = red
    {   0, 255,   0, 255,  1.0f, 1.0f, 0.0f, }, // Top-Right,    color = green
    {   0,   0, 255, 255, -1.0f, 1.0f, 0.0f  }, // Top-Left,     color = blue
};

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE g_hInstance,HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND g_hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void init(void);
void render(void);
void shutDown(void);
unsigned char *readShaderFile(const char *fileName);
void initShader(void);
void setShaderConstants(void);

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

	g_hWnd = CreateWindowEx( NULL,"MY_WINDOWS_CLASS",
						    "OpenGL - Simple Vertex Shader Using ARB_vertex_program",WS_OVERLAPPEDWINDOW,
					 	    0,0, 640,480, NULL, NULL, g_hInstance, NULL );

	if( g_hWnd == NULL )
		return E_FAIL;

    ShowWindow( g_hWnd, nCmdShow );
    UpdateWindow( g_hWnd );

	init();
	initShader();

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

	glEnable( GL_TEXTURE_2D );
	glEnable( GL_DEPTH_TEST );

	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0f, 640.0f / 480.0f, 0.1f, 100.0f);
}

//-----------------------------------------------------------------------------
// Name: readShaderFile()
// Desc: 
//-----------------------------------------------------------------------------
unsigned char *readShaderFile( const char *fileName )
{
    FILE *file = fopen( fileName, "r" );

    if( file == NULL )
    {
        MessageBox( NULL, "Cannot open shader file!", "ERROR",
            MB_OK | MB_ICONEXCLAMATION );
		return 0;
    }

    struct _stat fileStats;

    if( _stat( fileName, &fileStats ) != 0 )
    {
        MessageBox( NULL, "Cannot get file stats for shader file!", "ERROR",
                    MB_OK | MB_ICONEXCLAMATION );
        return 0;
    }

    unsigned char *buffer = new unsigned char[fileStats.st_size];

	int bytes = fread( buffer, 1, fileStats.st_size, file );

    buffer[bytes] = 0;

	fclose( file );

	return buffer;
}

//-----------------------------------------------------------------------------
// Name: initShader()
// Desc: Assemble the shader 
//-----------------------------------------------------------------------------
void initShader( void )
{
	//
	// If the required extension is present, get the addresses of its 
	// functions that we wish to use...
	//

	char *ext = (char*)glGetString( GL_EXTENSIONS );

	if( strstr( ext, "GL_ARB_vertex_program" ) == NULL )
	{
		MessageBox(NULL,"GL_ARB_vertex_program extension was not found",
			"ERROR",MB_OK|MB_ICONEXCLAMATION);
		return;
	}
	else
	{
		glGenProgramsARB           = (PFNGLGENPROGRAMSARBPROC)wglGetProcAddress("glGenProgramsARB");
		glDeleteProgramsARB        = (PFNGLDELETEPROGRAMSARBPROC)wglGetProcAddress("glDeleteProgramsARB");
		glBindProgramARB           = (PFNGLBINDPROGRAMARBPROC)wglGetProcAddress("glBindProgramARB");
		glProgramStringARB         = (PFNGLPROGRAMSTRINGARBPROC)wglGetProcAddress("glProgramStringARB");
		glProgramEnvParameter4fARB = (PFNGLPROGRAMENVPARAMETER4FARBPROC)wglGetProcAddress("glProgramEnvParameter4fARB");

		if( !glGenProgramsARB || !glDeleteProgramsARB || !glBindProgramARB || 
	        !glProgramStringARB || !glProgramEnvParameter4fARB )
		{
			MessageBox(NULL,"One or more GL_ARB_vertex_program functions were not found",
				"ERROR",MB_OK|MB_ICONEXCLAMATION);
			return;
		}
	}

	// Create the vertex program
    glGenProgramsARB( 1, &g_vertexProgramID );
    glBindProgramARB( GL_VERTEX_PROGRAM_ARB, g_vertexProgramID );
    
    unsigned char *shader_assembly = readShaderFile( "ogl_arb_shader_simple.txt" );

    glProgramStringARB( GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
                        strlen((char*) shader_assembly), shader_assembly );

    delete shader_assembly;
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )
{
	glDeleteProgramsARB( 1, &g_vertexProgramID );

	if( g_hRC != NULL )
	{
		wglMakeCurrent( NULL, NULL );
		wglDeleteContext( g_hRC );
		g_hRC = NULL;							
	}

	if( g_hRC != NULL )
	{
		ReleaseDC( g_hWnd, g_hDC );
		g_hDC = NULL;
	}
}

//-----------------------------------------------------------------------------
// Name: setShaderConstants()
// Desc: 
//-----------------------------------------------------------------------------
void setShaderConstants( void )
{
	glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
	glTranslatef( 0.0f, 0.0f, -4.0f );
    glRotatef( -g_fSpinY, 1.0f, 0.0f, 0.0f );
    glRotatef( -g_fSpinX, 0.0f, 1.0f, 0.0f );

    // Load a constant color into "program.env[0]"
    glProgramEnvParameter4fARB( GL_VERTEX_PROGRAM_ARB, 0, 0.0f, 1.0f, 0.0f, 0.0f );
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: 
//-----------------------------------------------------------------------------
void render( void )
{
	// Clear the screen and the depth buffer
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
    setShaderConstants();

	glBindProgramARB( GL_VERTEX_PROGRAM_ARB, g_vertexProgramID );
	glEnable( GL_VERTEX_PROGRAM_ARB );

	glInterleavedArrays( GL_C4UB_V3F, 0, g_quadVertices );
	glDrawArrays( GL_QUADS, 0, 4 );

	glDisable( GL_VERTEX_PROGRAM_ARB );

	SwapBuffers( g_hDC );
}
