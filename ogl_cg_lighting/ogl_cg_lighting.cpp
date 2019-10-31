//-----------------------------------------------------------------------------
//           Name: ogl_cg_simple.cpp
//         Author: Kevin Harris
//  Last Modified: 04/28/05
//    Description: This sample demonstrates how to write a simple CG style
//                 shader that duplicates basic diffuse lighting with OpenGL.
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <Cg/cgGL.h>
#include "resource.h"

//#define USE_FIXED_FUNCTION_OPENGL_LIGHTING

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HDC	      g_hDC       = NULL;
HGLRC     g_hRC       = NULL;
HWND      g_hWnd      = NULL;
HINSTANCE g_hInstance = NULL;

CGprofile   g_CGprofile;
CGcontext   g_CGcontext;
CGprogram   g_CGprogram;
CGparameter g_CGparam_modelViewProj;
CGparameter g_CGparam_modelViewInverse;
CGparameter g_CGparam_eyePosition;
CGparameter g_CGparam_lightVector;

float g_fSpinX = 0.0f;
float g_fSpinY = 0.0f;

struct Vertex
{
    float tu, tv;
    float r, g, b, a;
    float nx, ny, nz;
    float x, y, z;
};

Vertex g_pVertexArray[24] =
{
//    tu   tv     r    g    b    a      nx    ny    nz       x     y     z 
    // Front face
    { 0.0f,0.0f,  1.0f,1.0f,1.0f,1.0f,  0.0f, 0.0f, 1.0f,  -1.0f,-1.0f, 1.0f },
    { 1.0f,0.0f,  1.0f,1.0f,1.0f,1.0f,  0.0f, 0.0f, 1.0f,   1.0f,-1.0f, 1.0f },
    { 1.0f,1.0f,  1.0f,1.0f,1.0f,1.0f,  0.0f, 0.0f, 1.0f,   1.0f, 1.0f, 1.0f },
    { 0.0f,1.0f,  1.0f,1.0f,1.0f,1.0f,  0.0f, 0.0f, 1.0f,  -1.0f, 1.0f, 1.0f },
    // Back face
    { 1.0f,0.0f,  1.0f,1.0f,1.0f,1.0f,  0.0f, 0.0f,-1.0f,  -1.0f,-1.0f,-1.0f },
    { 1.0f,1.0f,  1.0f,1.0f,1.0f,1.0f,  0.0f, 0.0f,-1.0f,  -1.0f, 1.0f,-1.0f },
    { 0.0f,1.0f,  1.0f,1.0f,1.0f,1.0f,  0.0f, 0.0f,-1.0f,   1.0f, 1.0f,-1.0f },
    { 0.0f,0.0f,  1.0f,1.0f,1.0f,1.0f,  0.0f, 0.0f,-1.0f,   1.0f,-1.0f,-1.0f },
    // Top face
    { 0.0f,1.0f,  1.0f,1.0f,1.0f,1.0f,  0.0f, 1.0f, 0.0f,  -1.0f, 1.0f,-1.0f },
    { 0.0f,0.0f,  1.0f,1.0f,1.0f,1.0f,  0.0f, 1.0f, 0.0f,  -1.0f, 1.0f, 1.0f },
    { 1.0f,0.0f,  1.0f,1.0f,1.0f,1.0f,  0.0f, 1.0f, 0.0f,   1.0f, 1.0f, 1.0f },
    { 1.0f,1.0f,  1.0f,1.0f,1.0f,1.0f,  0.0f, 1.0f, 0.0f,   1.0f, 1.0f,-1.0f },
    // Bottom face
    { 1.0f,1.0f,  1.0f,1.0f,1.0f,1.0f,  0.0f,-1.0f, 0.0f,  -1.0f,-1.0f,-1.0f },
    { 0.0f,1.0f,  1.0f,1.0f,1.0f,1.0f,  0.0f,-1.0f, 0.0f,   1.0f,-1.0f,-1.0f },
    { 0.0f,0.0f,  1.0f,1.0f,1.0f,1.0f,  0.0f,-1.0f, 0.0f,   1.0f,-1.0f, 1.0f },
    { 1.0f,0.0f,  1.0f,1.0f,1.0f,1.0f,  0.0f,-1.0f, 0.0f,  -1.0f,-1.0f, 1.0f },
    // Right face
    { 1.0f,0.0f,  1.0f,1.0f,1.0f,1.0f,  1.0f, 0.0f, 0.0f,   1.0f,-1.0f,-1.0f },
    { 1.0f,1.0f,  1.0f,1.0f,1.0f,1.0f,  1.0f, 0.0f, 0.0f,   1.0f, 1.0f,-1.0f },
    { 0.0f,1.0f,  1.0f,1.0f,1.0f,1.0f,  1.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f },
    { 0.0f,0.0f,  1.0f,1.0f,1.0f,1.0f,  1.0f, 0.0f, 0.0f,   1.0f,-1.0f, 1.0f },
    // Left face
    { 0.0f,0.0f,  1.0f,1.0f,1.0f,1.0f, -1.0f, 0.0f, 0.0f,  -1.0f,-1.0f,-1.0f },
    { 1.0f,0.0f,  1.0f,1.0f,1.0f,1.0f, -1.0f, 0.0f, 0.0f,  -1.0f,-1.0f, 1.0f },
    { 1.0f,1.0f,  1.0f,1.0f,1.0f,1.0f, -1.0f, 0.0f, 0.0f,  -1.0f, 1.0f, 1.0f },
    { 0.0f,1.0f,  1.0f,1.0f,1.0f,1.0f, -1.0f, 0.0f, 0.0f,  -1.0f, 1.0f,-1.0f }
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
						    "OpenGL - Lighting Vertex Shader Using CG",WS_OVERLAPPEDWINDOW,
					 	    0,0, 640,480, NULL, NULL, g_hInstance, NULL );

	if( g_hWnd == NULL )
		return E_FAIL;

    ShowWindow( g_hWnd, nCmdShow );
    UpdateWindow( g_hWnd );

	init();

#ifndef USE_FIXED_FUNCTION_OPENGL_LIGHTING
	initShader();
#endif

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

	glEnable( GL_TEXTURE_2D );
	glEnable( GL_DEPTH_TEST );

    glEnable( GL_CULL_FACE );
	glCullFace( GL_BACK );

	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0f, 640.0f / 480.0f, 0.1f, 100.0f);

#ifdef USE_FIXED_FUNCTION_OPENGL_LIGHTING

	// Set up the view matrix
	glMatrixMode( GL_MODELVIEW );
	gluLookAt( 0.0, 0.0, 0.0,   // eye location
	           0.0, 0.0, 1.0,   // center is at (0,0,0)
	           0.0, 1.0, 0.0 ); // up is in positive Y direction

    glEnable( GL_LIGHTING );

	// Set up a material
	GLfloat ambient_mtrl[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat diffuse_mtrl[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glMaterialfv( GL_FRONT, GL_AMBIENT, ambient_mtrl ); 
	glMaterialfv( GL_FRONT, GL_DIFFUSE, diffuse_mtrl );

    // Set light 0 to be a pure white directional light
    GLfloat diffuse_light0[]  = { 1.0f, 1.0f,  1.0f, 1.0f };
    GLfloat specular_light0[] = { 1.0f, 1.0f,  1.0f, 1.0f };
    GLfloat position_light0[] = { 0.0f, 0.0f, -1.0f, 0.0f };
    glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuse_light0 );
    glLightfv( GL_LIGHT0, GL_SPECULAR, specular_light0 );
    glLightfv( GL_LIGHT0, GL_POSITION, position_light0 );
    glEnable( GL_LIGHT0 );

    // Enable some dim, grey ambient lighting so objects that are not lit 
    // by the other lights are not completely black.
	GLfloat ambient_lightModel[] = { 0.2f, 0.2f, 0.2f, 0.2f };
	glLightModelfv( GL_LIGHT_MODEL_AMBIENT, ambient_lightModel );

#endif

}

//-----------------------------------------------------------------------------
// Name: initShader()
// Desc: Assemble the shader 
//-----------------------------------------------------------------------------
void initShader(void)
{
	//
	// Search for a valid vertex shader profile in this order:
	//
	// CG_PROFILE_ARBVP1 - GL_ARB_vertex_program
	// CG_PROFILE_VP30   - GL_NV_vertex_program2
	// CG_PROFILE_VP20   - GL_NV_vertex_program
	//

    if( cgGLIsProfileSupported(CG_PROFILE_ARBVP1) )
        g_CGprofile = CG_PROFILE_ARBVP1;
    else if( cgGLIsProfileSupported(CG_PROFILE_VP30) )
        g_CGprofile = CG_PROFILE_VP30;
	else if( cgGLIsProfileSupported(CG_PROFILE_VP20) )
        g_CGprofile = CG_PROFILE_VP20;
    else
    {
        MessageBox( NULL,"Failed to initialize vertex shader! Hardware doesn't "
			        "support any of the vertex shading extensions!",
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
										   "ogl_cg_lighting.cg",
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

	g_CGparam_modelViewProj    = cgGetNamedParameter( g_CGprogram, "modelViewProjection" );
	g_CGparam_modelViewInverse = cgGetNamedParameter( g_CGprogram, "modelViewInverse" );
	g_CGparam_eyePosition      = cgGetNamedParameter( g_CGprogram, "eyePosition" );
	g_CGparam_lightVector      = cgGetNamedParameter( g_CGprogram, "lightVector");
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown(void)	
{

#ifndef USE_FIXED_FUNCTION_OPENGL_LIGHTING

	cgDestroyProgram(g_CGprogram);
	cgDestroyContext(g_CGcontext);

#endif

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
	glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
	glTranslatef( 0.0f, 0.0f, -5.0f );
    glRotatef( -g_fSpinY, 1.0f, 0.0f, 0.0f );
    glRotatef( -g_fSpinX, 0.0f, 1.0f, 0.0f );

	// This matrix will be used to transform the vertices from model-space to clip-space
	cgGLSetStateMatrixParameter( g_CGparam_modelViewProj,
							     CG_GL_MODELVIEW_PROJECTION_MATRIX,
							     CG_GL_MATRIX_IDENTITY );

	// This matrix will be used to transform the normals from model-space to view-space
	cgGLSetStateMatrixParameter( g_CGparam_modelViewInverse,
							     CG_GL_MODELVIEW_MATRIX,
                                 CG_GL_MATRIX_INVERSE_TRANSPOSE );

    float fEyePosition[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	float fLightVector[] = { 0.0f, 0.0f, 1.0f, 0.0f };

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
	// Clear the screen and the depth buffer
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
#ifdef USE_FIXED_FUNCTION_OPENGL_LIGHTING

	glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
	glTranslatef( 0.0f, 0.0f, -5.0f );
    glRotatef( -g_fSpinY, 1.0f, 0.0f, 0.0f );
    glRotatef( -g_fSpinX, 0.0f, 1.0f, 0.0f );

    glInterleavedArrays( GL_T2F_C4F_N3F_V3F, 0, g_pVertexArray );
    glDrawArrays( GL_QUADS, 0, 24 );

#else // Use the shader...

	setShaderConstants();

	cgGLBindProgram( g_CGprogram );
	cgGLEnableProfile( g_CGprofile );

    glInterleavedArrays( GL_T2F_C4F_N3F_V3F, 0, g_pVertexArray );
    glDrawArrays( GL_QUADS, 0, 24 );

	cgGLDisableProfile( g_CGprofile );

#endif

	SwapBuffers( g_hDC );
}
