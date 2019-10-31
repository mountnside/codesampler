//-----------------------------------------------------------------------------
//           Name: ogl_cg_simple_vs2ps.cpp
//         Author: Kevin Harris
//  Last Modified: 04/21/05
//    Description: This sample demonstrates how to write both a simple Vertex 
//                 and Pixel Shader with Cg under OpenGL. The two Shaders 
//                 also have matching connectors and can be used simultaneously 
//                 on the same piece of geometry. The sample should be 
//                 considered a starting point or framework for more advanced 
//                 samples.
//
//   Control Keys: F1 - Toggle usage of vertex and pixel shaders.
//
// Note: The pixel shader has been changed slightly from what the 
//       fixed-function pipeline does by default so you can see a noticeable 
//       change when toggling the shaders on and off. Instead of modulating 
//       the vertex color with the texture's texel, the pixel shader adds the 
//       two together, which causes the pixel shader to produce a brighter, 
//       washed-out image. This modification can be switched back in the pixel 
//       shader file.
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glaux.h>
#include <Cg/Cg.h>
#include <Cg/cgGL.h>
#include "resource.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HDC	      g_hDC       = NULL;
HGLRC     g_hRC       = NULL;
HWND      g_hWnd      = NULL;
HINSTANCE g_hInstance = NULL;
GLuint    g_textureID = 0;

CGprofile   g_CGprofile_vertex;
CGprofile   g_CGprofile_pixel;
CGcontext   g_CGcontext;
CGprogram   g_CGprogram_vertex;
CGprogram   g_CGprogram_pixel;
CGparameter g_CGparam_testTexture;

bool g_bUseShaders = true;

float g_fSpinX = 0.0f;
float g_fSpinY = 0.0f;

// GL_T2F_C3F_V3F
struct Vertex
{
	float tu, tv;
    float r, g, b;
    float x, y, z;
};

Vertex g_quadVertices[] =
{
	// tu,  tv     r    g    b       x     y     z 
    { 0.0f,0.0f,  1.0f,1.0f,0.0f, -1.0f,-1.0f, 0.0f },
    { 1.0f,0.0f,  1.0f,0.0f,0.0f,  1.0f,-1.0f, 0.0f },
    { 1.0f,1.0f,  0.0f,1.0f,0.0f,  1.0f, 1.0f, 0.0f },
    { 0.0f,1.0f,  0.0f,0.0f,1.0f, -1.0f, 1.0f, 0.0f },
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
void initShader(void);

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
						    "OpenGL - Simple Vertex & Pixel Shader Using CG",
							WS_OVERLAPPEDWINDOW,
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

				case VK_F1:
					g_bUseShaders = !g_bUseShaders;
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

	glEnable( GL_TEXTURE_2D );
	glEnable( GL_DEPTH_TEST );

	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0f, 640.0f / 480.0f, 0.1f, 100.0f );

	//
	// Create a texture to test out our pixel shader...
	//

	AUX_RGBImageRec *pTextureImage = auxDIBImageLoad( ".\\test.bmp" );

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
// Name: initShader()
// Desc: Load the CG shader 
//-----------------------------------------------------------------------------
void initShader( void )
{
    //
    // Search for a valid vertex shader profile in this order:
    //
    // CG_PROFILE_ARBVP1 - GL_ARB_vertex_program
    // CG_PROFILE_VP40   - GL_ARB_vertex_program + GL_NV_vertex_program3
    //

    if( cgGLIsProfileSupported(CG_PROFILE_ARBVP1) )
        g_CGprofile_vertex = CG_PROFILE_ARBVP1;
    else if( cgGLIsProfileSupported(CG_PROFILE_VP40) )
        g_CGprofile_vertex = CG_PROFILE_VP40;
    else
    {
        MessageBox( NULL,"Failed to initialize vertex shader! Hardware doesn't "
            "support any of the required vertex shading extensions!",
            "ERROR",MB_OK|MB_ICONEXCLAMATION );
        return;
    }

	//
	// Search for a valid pixel shader profile in this order:
	//
	// CG_PROFILE_ARBFP1 - GL_ARB_fragment_program
	// CG_PROFILE_FP30   - GL_NV_fragment_program
	// CG_PROFILE_FP20   - NV_texture_shader & NV_register_combiners
	//
	
	if( cgGLIsProfileSupported(CG_PROFILE_ARBFP1) )
        g_CGprofile_pixel = CG_PROFILE_ARBFP1;
    else if( cgGLIsProfileSupported(CG_PROFILE_FP30) )
        g_CGprofile_pixel = CG_PROFILE_FP30;
	else if( cgGLIsProfileSupported(CG_PROFILE_FP20) )
        g_CGprofile_pixel = CG_PROFILE_FP20;
    else
    {
        MessageBox( NULL,"Failed to initialize pixel shader! Hardware doesn't "
			        "support any of the required pixel shading extensions!",
			        "ERROR",MB_OK|MB_ICONEXCLAMATION );
		return;
    }

	// Create the context...
	g_CGcontext = cgCreateContext();

	//
	// Create the vertex and pixel shader...
	//
	
	g_CGprogram_vertex = cgCreateProgramFromFile( g_CGcontext,
										          CG_SOURCE,
										         "vertex_shader.cg",
										         g_CGprofile_vertex,
										         NULL, 
										         NULL );

	g_CGprogram_pixel = cgCreateProgramFromFile( g_CGcontext,
										         CG_SOURCE,
										         "pixel_shader.cg",
										         g_CGprofile_pixel,
										         NULL, 
										         NULL );

	//
	// Load the programs using Cg's expanded interface...
	//

	cgGLLoadProgram( g_CGprogram_vertex );
	cgGLLoadProgram( g_CGprogram_pixel );

	//
	// Bind some parameters by name so we can set them later...
	//

	g_CGparam_testTexture   = cgGetNamedParameter(g_CGprogram_pixel, "testTexture");
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )	
{
    glDeleteTextures( 1, &g_textureID );

    cgDestroyProgram(g_CGprogram_vertex);
	cgDestroyProgram(g_CGprogram_pixel);
	cgDestroyContext(g_CGcontext);
        
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
	// Clear the screen and the depth buffer
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glTranslatef( 0.0f, 0.0f, -4.0f );
    glRotatef( -g_fSpinY, 1.0f, 0.0f, 0.0f );
    glRotatef( -g_fSpinX, 0.0f, 1.0f, 0.0f );

	if( g_bUseShaders == true )
	{
		//
		// Use vertex and pixel shaders...
		//

        // Identify the texture to use
        cgGLSetTextureParameter( g_CGparam_testTexture, g_textureID );

		cgGLBindProgram( g_CGprogram_vertex );
		cgGLEnableProfile( g_CGprofile_vertex );

		cgGLBindProgram( g_CGprogram_pixel );
		cgGLEnableProfile( g_CGprofile_pixel );

		cgGLEnableTextureParameter( g_CGparam_testTexture );

		glInterleavedArrays( GL_T2F_C3F_V3F, 0, g_quadVertices );
		glDrawArrays( GL_QUADS, 0, 4 );

		cgGLDisableTextureParameter( g_CGparam_testTexture );

		cgGLDisableProfile( g_CGprofile_vertex );
		cgGLDisableProfile( g_CGprofile_pixel );
	}
	else
	{
		//
		// Render the normal way...
		//

		glBindTexture( GL_TEXTURE_2D, g_textureID );
		glInterleavedArrays( GL_T2F_C3F_V3F, 0, g_quadVertices );
		glDrawArrays( GL_QUADS, 0, 4 );
	}

	SwapBuffers( g_hDC );
}

