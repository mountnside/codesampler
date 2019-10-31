//-----------------------------------------------------------------------------
//           Name: ogl_cg_anisotropic_lighting.cpp
//         Author: Kevin Harris
//  Last Modified: 04/21/05
//    Description: Demonstrates how to write a vertex shader using Cg, which 
//                 calculates Anisotropic Lighting for a single light source. 
//                 The shader uses a texture as a look-up table for the correct 
//                 Anisotropic Lighting values by storing the pre-calculated 
//                 diffuse values in the texture's RGB components and specular 
//                 values in its alpha component. This style of lighting is 
//                 very useful for rendering surfaces like brushed steel where 
//                 the surface is composed of micro facets or microscopic 
//                 scratches that tend to lay parallel or run in the same 
//                 direction.
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
// Do this to access M_PI, which is not officially part of the C/C++ standard.
#define _USE_MATH_DEFINES 
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <Cg/cgGL.h>
#include "resource.h"
#include "geometry.h"
#include "tga.h"

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
HDC	      g_hDC       = NULL;
HGLRC     g_hRC       = NULL;
HWND      g_hWnd      = NULL;
HINSTANCE g_hInstance = NULL;
GLuint    g_textureID = 0;

CGprofile   g_CGprofile;
CGcontext   g_CGcontext;
CGprogram   g_CGprogram;
CGparameter g_CGparam_modelViewProj;
CGparameter g_CGparam_modelViewInv;
CGparameter g_CGparam_model;
CGparameter g_CGparam_lightVector;
CGparameter g_CGparam_eyePosition;

float g_fSpinX = 0.0f;
float g_fSpinY = 0.0f;

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

	g_hWnd = CreateWindowEx( NULL, "MY_WINDOWS_CLASS",
						     "OpenGL - Anisotropic Lighting Shader Using CG",
                             WS_OVERLAPPEDWINDOW, 0, 0, 640, 480, NULL, NULL, 
                             g_hInstance, NULL );

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
    pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 16;
    pfd.cDepthBits = 16;
	
	g_hDC = GetDC( g_hWnd );
	PixelFormat = ChoosePixelFormat( g_hDC, &pfd );
	SetPixelFormat( g_hDC, PixelFormat, &pfd);
	g_hRC = wglCreateContext( g_hDC );
	wglMakeCurrent( g_hDC, g_hRC );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0f, 640.0f / 480.0f, 0.1f, 100.0f);

	//
    // Set up some general rendering states...
    //

	glEnable( GL_DEPTH_TEST );
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

	//
    // This is the texture used by the shader for looking up the anisotropic 
    // lighting values...
    //
	
    tgaImageFile tgaImage;
    tgaImage.load( "ogl_aniso.tga" );

    glGenTextures( 1, &g_textureID );

	glEnable( GL_TEXTURE_2D );

    glBindTexture( GL_TEXTURE_2D, g_textureID );
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT_ARB );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT_ARB );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	glTexImage2D( GL_TEXTURE_2D, 0, tgaImage.m_texFormat, 
		          tgaImage.m_nImageWidth, tgaImage.m_nImageHeight, 
                  0, tgaImage.m_texFormat, GL_UNSIGNED_BYTE, tgaImage.m_nImageData );

	//
    // It's often needed to modulate or increase the diffuse and specular 
    // terms to a small power (between 2 or 4) to account for the fact that  
    // the most-significant normal does not account for the entire lighting 
    // of the anisotropic surface.
    //

    // Turn on programmable texture combiners
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT );

    glTexEnvf( GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_TEXTURE );
	glTexEnvf( GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR );

    glTexEnvf( GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_TEXTURE );
	glTexEnvf( GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_ALPHA );

	// Modulate the texture brightness by 4X
	glTexEnvf( GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE );
    glTexEnvf( GL_TEXTURE_ENV, GL_RGB_SCALE_EXT, 4.0f );
}

//-----------------------------------------------------------------------------
// Name: initShader()
// Desc: Load the vertex shader 
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
        g_CGprofile = CG_PROFILE_ARBVP1;
    else if( cgGLIsProfileSupported(CG_PROFILE_VP40) )
        g_CGprofile = CG_PROFILE_VP40;
    else
    {
        MessageBox( NULL,"Failed to initialize vertex shader! Hardware doesn't "
            "support any of the required vertex shading extensions!",
            "ERROR",MB_OK|MB_ICONEXCLAMATION );
        return;
    }

	// Create the context...
	g_CGcontext = cgCreateContext();

	//
	// Create the vertex shader...
	//
	
	g_CGprogram = cgCreateProgramFromFile( g_CGcontext,
										   CG_SOURCE,
										   "ogl_cg_anisotropic_lighting.cg",
										   g_CGprofile,
										   NULL, 
										   NULL );

	//
	// Load the program using Cg's expanded interface...
	//

	cgGLLoadProgram( g_CGprogram );

	//
	// Bind some parameters by name so we can set them later...
	//

	g_CGparam_modelViewProj = cgGetNamedParameter( g_CGprogram, "modelViewProj" );
	g_CGparam_modelViewInv  = cgGetNamedParameter( g_CGprogram, "modelViewInv" );
	g_CGparam_model         = cgGetNamedParameter( g_CGprogram, "model" );
	g_CGparam_lightVector   = cgGetNamedParameter( g_CGprogram, "lightVector" );
	g_CGparam_eyePosition   = cgGetNamedParameter( g_CGprogram, "eyePosition" );
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown(void)	
{
    glDeleteTextures( 1, &g_textureID );

	cgDestroyProgram(g_CGprogram);
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
// Name: setShaderConstants()
// Desc: 
//-----------------------------------------------------------------------------
void setShaderConstants( void )
{
    //
    // BUG NOTE:
    //
    // There's a Cg bug, that causes the teapot mesh to blow up in size and 
    // offset from the origin if cgGLSetStateMatrixParameter is used to pass 
    // the model-view matrix. As a work-around, you can use the arbvp1 profile
    // and access the model-view matrix through its "glstate" structure 
    // (i.e "glstate.matrix.mvp"). Look in the shader source to see how this
    // is done.
    //

	cgGLSetStateMatrixParameter( g_CGparam_modelViewProj,
							     CG_GL_MODELVIEW_PROJECTION_MATRIX,
							     CG_GL_MATRIX_IDENTITY );

	cgGLSetStateMatrixParameter( g_CGparam_modelViewInv,
							     CG_GL_MODELVIEW_MATRIX,
								 CG_GL_MATRIX_INVERSE_TRANSPOSE );

	cgGLSetStateMatrixParameter( g_CGparam_model,
							     CG_GL_MODELVIEW_MATRIX,
								 CG_GL_MATRIX_IDENTITY );

    float fLightVector[] = { 1.0f, 0.0f, 1.0f, 0.0f };
    float fEyePosition[] = { 0.0f, 0.0f, 0.0f, 1.0f };

    // Normalize light vector
    float fLength = sqrtf( fLightVector[0]*fLightVector[0] +
                           fLightVector[1]*fLightVector[1] +
                           fLightVector[2]*fLightVector[2] );
	fLightVector[0] /= fLength;
	fLightVector[1] /= fLength;
	fLightVector[2] /= fLength;

    cgGLSetParameter4fv( g_CGparam_eyePosition, fEyePosition );
	cgGLSetParameter4fv( g_CGparam_lightVector, fLightVector );
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: 
//-----------------------------------------------------------------------------
void render( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

/*
    //
    // BUG NOTE:
    //
    // If you want to use the API Cg function cgGLSetStateMatrixParameter, use
    // the hacked code below to fix the teapot bug. You'll also need to make
    // some changes to the shader file as well.
    //

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glTranslatef( 0.0f, 0.0f, -4.0f );
    glRotatef( -g_fSpinY-90.0f, 1.0f, 0.0f, 0.0f ); // Correct the teapot's orientation...
    glRotatef( -g_fSpinX, 0.0f, 1.0f, 0.0f );

    glScalef( 0.5f, 0.5f, 0.5f );     // Correct the teapot's scale...
    glTranslatef( 0.0f, 0.0f,-1.5f ); // Correct the teapot's translation...
//*/

//*
    //
    // BUG NOTE:
    //
    // If you want to use arbvp1 and "glstate.matrix.mvp" to do the right
    // thing and not hack your transform, use the correct code below and 
    // access the model-view matrix through arbvp1's "glstate" structure.
    // You'll also need to make some changes to the shader file as well.
    //

	glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
	glTranslatef( 0.0f, 0.0f, -4.0f );
    glRotatef( -g_fSpinY, 1.0f, 0.0f, 0.0f );
    glRotatef( -g_fSpinX, 0.0f, 1.0f, 0.0f );
//*/

    setShaderConstants();

    glBindTexture( GL_TEXTURE_2D, g_textureID );

    cgGLBindProgram( g_CGprogram );
    cgGLEnableProfile( g_CGprofile );

	//
    // Render a teapot...
    //
    renderSolidTeapot( 1.0 );
	
	cgGLDisableProfile( g_CGprofile );

	SwapBuffers( g_hDC );
}
