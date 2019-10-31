//-----------------------------------------------------------------------------
//           Name: ogl_cg_fixed_function.cpp
//         Author: Kevin Harris
//  Last Modified: 04/21/05
//    Description: This sample demonstrates how to make use of nVIDIA's 
//                 "fixed_function.cg" file to recreate certain portions of 
//                 OpenGL's fixed-function-pipeline.
//
// The "fixed_function.cg" shader source file came from here:
//
// http://developer.nvidia.com/view.asp?IO=cg_fixed_function
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdlib.h>
#include <math.h>
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

float g_fSpinX = 0.0f;
float g_fSpinY = 0.0f;

//-----------------------------------------------------------------------------

CGprofile g_CGprofile;
CGcontext g_CGcontext;
CGprogram g_CGprogram;

// Matrices
CGparameter g_CGparam_ModelViewProj;
CGparameter g_CGparam_ModelViewIT;
CGparameter g_CGparam_ModelView;

// Material properties
CGparameter g_CGparam_DiffMat;
CGparameter g_CGparam_SpecMat;
CGparameter g_CGparam_AmbMat;
CGparameter g_CGparam_ShineMat;
CGparameter g_CGparam_EmisMat;

// Lighting properties
CGparameter g_CGparam_LightVec;
CGparameter g_CGparam_LightPos;
CGparameter g_CGparam_DiffLight;
CGparameter g_CGparam_SpecLight;
CGparameter g_CGparam_AmbLight;
CGparameter g_CGparam_AttenLight;
CGparameter g_CGparam_SpotLight;

// Texture transforms & settings
CGparameter g_CGparam_TexPlaneX;
CGparameter g_CGparam_TexPlaneY;
CGparameter g_CGparam_TexPlaneZ;
CGparameter g_CGparam_TexPlaneW;
CGparameter g_CGparam_TexMatrix;
CGparameter g_CGparam_TexIMV;

// Varying, Per-vertex attributes
CGparameter g_CGparam_PSize;

//-----------------------------------------------------------------------------

struct Vertex
{
	//GL_T2F_C4F_N3F_V3F
	float tu, tv;
	float r, g, b, a;
	float nx, ny, nz;
	float x, y, z;
};

Vertex g_quadVertices[] =
{
	// tu,  tv     r    g    b    a      nx   ny  nz     x     y     z 
	{ 0.0f,0.0f,  1.0f,1.0f,1.0f,1.0f,  0.0f,0.0f,1.0, -1.0f,-1.0f, 0.0f },
	{ 1.0f,0.0f,  1.0f,1.0f,1.0f,1.0f,  0.0f,0.0f,1.0,  1.0f,-1.0f, 0.0f },
	{ 1.0f,1.0f,  1.0f,1.0f,1.0f,1.0f,  0.0f,0.0f,1.0,  1.0f, 1.0f, 0.0f },
	{ 0.0f,1.0f,  1.0f,1.0f,1.0f,1.0f,  0.0f,0.0f,1.0, -1.0f, 1.0f, 0.0f },
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
						    "OpenGL - Cg Shader Version of OpenGL's Fixed Function Pipeline",
							WS_OVERLAPPEDWINDOW,0,0, 640,480, NULL, NULL, g_hInstance, NULL );

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
	SetPixelFormat( g_hDC, PixelFormat, &pfd );
	g_hRC = wglCreateContext( g_hDC );
	wglMakeCurrent( g_hDC, g_hRC );

	glEnable( GL_LIGHTING );
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_DEPTH_TEST );

	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0f, 640.0f / 480.0f, 0.1f, 100.0f );

	//
	// Create a texture...
	//

	AUX_RGBImageRec *pTextureImage = auxDIBImageLoad( ".\\nvidia.bmp" );

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
	// Create the vertex shaders...
	//
	
	g_CGprogram = cgCreateProgramFromFile( g_CGcontext,
										   CG_SOURCE,
										   "fixed_function.cg",
										   g_CGprofile,
										   //"main_point_local",
										   //"main_spot_local",
										   //"main_directional_local",
										   "main_directional_inf",
										   //"main_point_local",
										   //"main_simple_point",
										   //"main_texgen_eye",
										   //"main_texgen_object",
										   //"main_texgen_sphere",
										   NULL );

	//
	// Load the program using Cg's expanded interface...
	//

	cgGLLoadProgram( g_CGprogram );

	//
	// If you're going to replace the fixed function pipeline with a Cg 
    // shader... you got a lot of name binding to do my friend.
	//

	// Matrices
	g_CGparam_ModelViewProj = cgGetNamedParameter(g_CGprogram, "mx.ModelViewProj");
	g_CGparam_ModelViewIT = cgGetNamedParameter(g_CGprogram, "mx.ModelViewIT");
	g_CGparam_ModelView = cgGetNamedParameter(g_CGprogram, "mx.ModelView");

	// Material properties
	g_CGparam_DiffMat = cgGetNamedParameter(g_CGprogram, "mat.DiffMat");
	g_CGparam_SpecMat = cgGetNamedParameter(g_CGprogram, "mat.SpecMat");
	g_CGparam_AmbMat = cgGetNamedParameter(g_CGprogram, "mat.AmbMat");
	g_CGparam_ShineMat = cgGetNamedParameter(g_CGprogram, "mat.ShineMat");
	g_CGparam_EmisMat = cgGetNamedParameter(g_CGprogram, "mat.EmisMat");

	// Lighting properties
	g_CGparam_LightVec = cgGetNamedParameter(g_CGprogram, "lt.LightVec");
	g_CGparam_LightPos = cgGetNamedParameter(g_CGprogram, "lt.LightPos");
	g_CGparam_DiffLight = cgGetNamedParameter(g_CGprogram, "lt.DiffLight");
	g_CGparam_SpecLight = cgGetNamedParameter(g_CGprogram, "lt.SpecLight");
	g_CGparam_AmbLight = cgGetNamedParameter(g_CGprogram, "lt.AmbLight");
	g_CGparam_AttenLight = cgGetNamedParameter(g_CGprogram, "lt.AttenLight");
	g_CGparam_SpotLight = cgGetNamedParameter(g_CGprogram, "lt.SpotLight");

	// Texgen Planes & Texture transforms
	g_CGparam_TexPlaneX = cgGetNamedParameter(g_CGprogram, "tex.TexPlaneX");
	g_CGparam_TexPlaneY = cgGetNamedParameter(g_CGprogram, "tex.TexPlaneY");
	g_CGparam_TexPlaneZ = cgGetNamedParameter(g_CGprogram, "tex.TexPlaneZ");
	g_CGparam_TexPlaneW = cgGetNamedParameter(g_CGprogram, "tex.TexPlaneW ");
	g_CGparam_TexMatrix = cgGetNamedParameter(g_CGprogram, "tex.TexMatrix");
	g_CGparam_TexIMV = cgGetNamedParameter(g_CGprogram, "tex.TexIMV");

	// Varying, Per-vertex attributes
	g_CGparam_PSize = cgGetNamedParameter(g_CGprogram, "IN.PSize");
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )	
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
	glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
	glTranslatef( 0.0f, 0.0f, -4.0f );
    glRotatef( -g_fSpinY, 1.0f, 0.0f, 0.0f );
    glRotatef( -g_fSpinX, 0.0f, 1.0f, 0.0f );

	//
	// Matrices
	//

	cgGLSetStateMatrixParameter( g_CGparam_ModelViewProj,
							     CG_GL_MODELVIEW_PROJECTION_MATRIX,
							     CG_GL_MATRIX_IDENTITY );

	cgGLSetStateMatrixParameter( g_CGparam_ModelViewIT,
							     CG_GL_MODELVIEW_MATRIX,
							     CG_GL_MATRIX_INVERSE_TRANSPOSE );

	cgGLSetStateMatrixParameter( g_CGparam_ModelView,
							     CG_GL_MODELVIEW_MATRIX,
							     CG_GL_MATRIX_IDENTITY );

	//
	// Material properties
	//

	cgGLSetParameter4f( g_CGparam_DiffMat,  0.8f, 0.8f, 0.8f, 1.0f );
	cgGLSetParameter4f( g_CGparam_SpecMat,  0.0f, 0.0f, 0.0f, 1.0f );
	cgGLSetParameter4f( g_CGparam_AmbMat,   0.2f, 0.2f, 0.2f, 1.0f );
	cgGLSetParameter4f( g_CGparam_ShineMat, 0.0f, 0.0f, 0.0f, 0.0f );
	cgGLSetParameter4f( g_CGparam_EmisMat,  0.0f, 0.0f, 0.0f, 1.0f );

	//
	// Lighting properties
	//

	cgGLSetParameter4f( g_CGparam_LightVec,  0.0f, 0.0f, -1.0f, 1.0f );
	cgGLSetParameter4f( g_CGparam_LightPos,  0.0f, 0.0f,  1.0f, 0.0f  );
	cgGLSetParameter4f( g_CGparam_DiffLight, 1.0f, 1.0f,  1.0f, 1.0f  );
	cgGLSetParameter4f( g_CGparam_SpecLight, 1.0f, 1.0f,  1.0f, 1.0f );
	cgGLSetParameter4f( g_CGparam_AmbLight,  0.0f, 0.0f,  0.0f, 1.0f );
	cgGLSetParameter3f( g_CGparam_AttenLight, 1.0f, 0.0f, 0.0f );  // constant, linear, quadratic
	cgGLSetParameter2f( g_CGparam_SpotLight, cosf(180.0f), 0.0f ); //cosf(spot_cuttoff_angle), spot power

	//
	// Texgen Planes & Texture transforms
	//

	cgGLSetParameter4f( g_CGparam_TexPlaneX, 1.0f, 0.0f, 0.0f, 0.0f ); // s
	cgGLSetParameter4f( g_CGparam_TexPlaneY, 0.0f, 1.0f, 0.0f, 0.0f ); // t
	cgGLSetParameter4f( g_CGparam_TexPlaneZ, 0.0f, 0.0f, 0.0f, 0.0f ); // r
	cgGLSetParameter4f( g_CGparam_TexPlaneW, 0.0f, 0.0f, 0.0f, 0.0f ); // q

	cgGLSetStateMatrixParameter( g_CGparam_TexMatrix,
							     CG_GL_TEXTURE_MATRIX,
							     CG_GL_MATRIX_IDENTITY );

	cgGLSetStateMatrixParameter( g_CGparam_TexIMV,
							     CG_GL_TEXTURE_MATRIX,
							     CG_GL_MATRIX_INVERSE );
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

    cgGLBindProgram( g_CGprogram );
	cgGLEnableProfile( g_CGprofile );

	glBindTexture( GL_TEXTURE_2D, g_textureID );

    glInterleavedArrays( GL_T2F_C4F_N3F_V3F, 0, g_quadVertices );
    glDrawArrays( GL_QUADS, 0, 4 );

	/*

	//
	// If for some reason you need to pass some data which varies on a per-vertex 
	// basis, you can use one of the pre-defined register slots like PSIZE0 to 
	// pass your custom data to the shader. Of course, you'll have to stop using
	// glInterleavedArrays since it doesn't have special pre-defined enums like
	// GL_T2F_C4F_N3F_V3F, which include point size.
	//
	// Here's how you would send your custom per-vertex data using 
	// g_CGparam_PSize, which I've already bound to slot PSIZE0 for you.
	//

	glBegin( GL_QUADS );
	{
		for( int i = 0; i < 4; ++i )
		{
            // For every vertex passed, you can set g_CGparam_PSize to what ever you want...
            cgGLSetParameter4f( g_CGparam_PSize, 0.0f, 0.0f, 0.0f, 0.0f );

			glTexCoord2f( g_quadVertices[i].tu, g_quadVertices[i].tv );
			glColor4f( g_quadVertices[i].r, g_quadVertices[i].g, g_quadVertices[i].b, g_quadVertices[i].a );
			glNormal3f( g_quadVertices[i].nx, g_quadVertices[i].ny, g_quadVertices[i].nz );
			glVertex3f( g_quadVertices[i].x, g_quadVertices[i].y, g_quadVertices[i].z );
		}
	}
	glEnd();

	//*/

    cgGLDisableProfile( g_CGprofile );

	SwapBuffers( g_hDC );
}
