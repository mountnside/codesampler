//-----------------------------------------------------------------------------
//           Name: ogl_cg_multitexture_blending.cpp
//         Author: Kevin Harris
//  Last Modified: 04/26/05
//    Description: This sample demonstrates how to use a Cg fragment shader to
//                 blend three textures together by passing the desired 
//                 contribution of each texture into the shader via the 
//                 vertex's color.
//
//                 Techniques like this are becoming very popular in terrain 
//                 rendering engines which need to blend dramatically different 
//                 textures such as rock and grass together with out creating a 
//                 noticeable edge. For example, with three textures consisting 
//                 of stone, grass, and sand you can render a mountain that 
//                 blends in  patches of grass and sand at its base.
//
//   Control Keys: F1   - Increase contribution of texture 0
//                 F2   - Decrease contribution of texture 0
//                 F3   - Increase contribution of texture 2
//                 F4   - Decrease contribution of texture 2
//                 F5   - Toggle wire-frame mode
//                 Up   - View moves forward
//                 Down - View moves backward
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
// Note: An older, fall-back technique for older hardware, which doesn’t require 
//       a fragment shader, can be found here:
//
// http://www.codesampler.com/oglsrc/oglsrc_4.htm#ogl_multitexture_blending
//----------------------------------------------------------------------------- 

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <mmsystem.h>
#include <sys/timeb.h>
#include <stdlib.h>
#include <stdio.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glaux.h>
#include <Cg/Cg.h>
#include <Cg/cgGL.h>
#include "resource.h"
#include "bitmap_fonts.h"

// Don't forget to change the code in the pixel shader to match your selection here!
//#define USE_OPENGL_METHOD_OF_PASSING_MULTITEXTURE_ID
#define USE_CG_METHOD_OF_PASSING_MULTITEXTURE_ID

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

PFNGLACTIVETEXTUREARBPROC       glActiveTextureARB       = NULL;
PFNGLMULTITEXCOORD2FARBPROC     glMultiTexCoord2fARB     = NULL;
PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB = NULL;

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND  g_hWnd = NULL;
HDC	  g_hDC  = NULL;
HGLRC g_hRC  = NULL;

int g_nWindowWidth  = 640;
int g_nWindowHeight = 480;

float g_fDistance = -4.0f;
float g_fSpinX    =  0.0f;
float g_fSpinY    =  0.0f;

GLuint g_textureID_0 = -1;
GLuint g_textureID_1 = -1;
GLuint g_textureID_2 = -1;

CGprofile   g_CGprofile_pixel;
CGcontext   g_CGcontext;
CGprogram   g_CGprogram_pixel;
CGparameter g_CGparam_testTexture0;
CGparameter g_CGparam_testTexture1;
CGparameter g_CGparam_testTexture2;

bool g_bWireFrameMode = false;

float g_fContributionOfTex0 = 0.33f;
float g_fContributionOfTex1 = 0.33f;
float g_fContributionOfTex2 = 0.33f;

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

	g_hWnd = CreateWindowEx( NULL, "MY_WINDOWS_CLASS", 
                             "OpenGL - Multi-Texture Blending With Cg",
						     WS_OVERLAPPEDWINDOW | WS_VISIBLE,
					         0, 0, 640, 480, NULL, NULL, hInstance, NULL );

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

    UnregisterClass( "MY_WINDOWS_CLASS", hInstance );

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
                    {
                        g_fContributionOfTex0 += 0.01f;
                        if( g_fContributionOfTex0 > 1.0f ) 
                            g_fContributionOfTex0 = 1.0f;

                        // If the total contribution of textures 0 and 2 is
                        // greater than 1.0f after we increased the 
                        // contribution of texture 0, we need to reduce the 
                        // contribution from texture 2 to balance it out.
                        while( (g_fContributionOfTex0 + g_fContributionOfTex2) > 1.0f )
                            g_fContributionOfTex2 -= 0.01f;

                        if( g_fContributionOfTex2 < 0.0f )
                            g_fContributionOfTex2 = 0.0f;
                    }
                    break;

                case VK_F2:
                    g_fContributionOfTex0 -= 0.01f;
                    if( g_fContributionOfTex0 < 0.0f )
                        g_fContributionOfTex0 = 0.0f;
                    break;

                case VK_F3:
                    {
                        g_fContributionOfTex2 += 0.01f;
                        if( g_fContributionOfTex2 > 1.0f ) 
                            g_fContributionOfTex2 = 1.0f;

                        // If the total contribution of textures 0 and 2 is
                        // greater than 1.0f after we increased the 
                        // contribution of texture 2, we need to reduce the 
                        // contribution from texture 0 to balance it out.
                        while( (g_fContributionOfTex0 + g_fContributionOfTex2) > 1.0f )
                            g_fContributionOfTex0 -= 0.01f;

                        if( g_fContributionOfTex0 < 0.0f )
                            g_fContributionOfTex0 = 0.0f;
                    }
                    break;

                case VK_F4:
                    g_fContributionOfTex2 -= 0.01f;
                    if( g_fContributionOfTex2 < 0.0f ) 
                        g_fContributionOfTex2 = 0.0f;
                    break;

                case VK_F5:
                    g_bWireFrameMode = !g_bWireFrameMode;
                    break;

                case 38: // Up Arrow Key
                    g_fDistance += 1.0f;
                    break;

                case 40: // Down Arrow Key
                    g_fDistance -= 1.0f;
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
void loadTexture( void )	
{
    AUX_RGBImageRec *pTextureImage = auxDIBImageLoad( ".\\texture0.bmp" );

    if( pTextureImage != NULL )
	{
        glGenTextures( 1, &g_textureID_0 );

		glBindTexture( GL_TEXTURE_2D, g_textureID_0 );

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

    //
    //
    //

    pTextureImage = auxDIBImageLoad( ".\\texture1.bmp" );

    if( pTextureImage != NULL )
	{
        glGenTextures( 1, &g_textureID_1 );

		glBindTexture( GL_TEXTURE_2D, g_textureID_1 );

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

    //
    //
    //

    pTextureImage = auxDIBImageLoad( ".\\texture2.bmp" );

    if( pTextureImage != NULL )
	{
        glGenTextures( 1, &g_textureID_2 );

		glBindTexture( GL_TEXTURE_2D, g_textureID_2 );

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
			        "support any of the pixel shading extensions!",
			        "ERROR",MB_OK|MB_ICONEXCLAMATION );
		return;
    }

	// Create the context...
	g_CGcontext = cgCreateContext();

	//
	// Create the pixel shader...
	//

	g_CGprogram_pixel = cgCreateProgramFromFile( g_CGcontext,
										         CG_SOURCE,
										         "ogl_cg_multitexture_blending.cg",
										         g_CGprofile_pixel,
										         NULL, 
										         NULL );

	//
	// Load the programs using Cg's expanded interface...
	//

	cgGLLoadProgram( g_CGprogram_pixel );

	//
	// Bind some parameters by name so we can set them later...
	//
#ifdef USE_CG_METHOD_OF_PASSING_MULTITEXTURE_ID
	g_CGparam_testTexture0 = cgGetNamedParameter(g_CGprogram_pixel, "testTexture0");
    g_CGparam_testTexture1 = cgGetNamedParameter(g_CGprogram_pixel, "testTexture1");
    g_CGparam_testTexture2 = cgGetNamedParameter(g_CGprogram_pixel, "testTexture2");
#endif
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

    int nNumTextureUnits = 0;
    glGetIntegerv( GL_MAX_TEXTURE_UNITS_ARB, &nNumTextureUnits );

	//
	// If the required extension is present, get the addresses of its 
	// functions that we wish to use...
	//

	char *ext = (char*)glGetString( GL_EXTENSIONS );

	if( strstr( ext, "GL_ARB_multitexture" ) == NULL )
	{
		MessageBox(NULL,"GL_ARB_multitexture extension was not found",
			"ERROR",MB_OK|MB_ICONEXCLAMATION);
		return;
	}
	else
	{
		glActiveTextureARB       = (PFNGLCLIENTACTIVETEXTUREARBPROC)wglGetProcAddress("glActiveTextureARB");
		glMultiTexCoord2fARB     = (PFNGLMULTITEXCOORD2FARBPROC)wglGetProcAddress("glMultiTexCoord2fARB");
		glClientActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)wglGetProcAddress("glClientActiveTextureARB");

		if( !glActiveTextureARB || !glMultiTexCoord2fARB || !glClientActiveTextureARB )
		{
			MessageBox(NULL,"One or more GL_ARB_multitexture functions were not found",
				"ERROR",MB_OK|MB_ICONEXCLAMATION);
			return;
		}
	}
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )	
{
    glDeleteTextures( 1, &g_textureID_0 );
    glDeleteTextures( 1, &g_textureID_1 );
    glDeleteTextures( 1, &g_textureID_2 );

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
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glTranslatef( 0.0f, 0.0f, g_fDistance );
    glRotatef( -g_fSpinY, 1.0f, 0.0f, 0.0f );
    glRotatef( -g_fSpinX, 0.0f, 1.0f, 0.0f );

    if( g_bWireFrameMode )
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    else
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    // Identify the textures to use for the pixel shader...
    cgGLSetTextureParameter( g_CGparam_testTexture0, g_textureID_0 );
    cgGLSetTextureParameter( g_CGparam_testTexture1, g_textureID_1 );
    cgGLSetTextureParameter( g_CGparam_testTexture2, g_textureID_2 );

#ifdef USE_OPENGL_METHOD_OF_PASSING_MULTITEXTURE_ID
    // STAGE 0
    glActiveTextureARB( GL_TEXTURE0_ARB );
    glBindTexture( GL_TEXTURE_2D, g_textureID_0 );

    // STAGE 1
    glActiveTextureARB( GL_TEXTURE1_ARB );
    glBindTexture( GL_TEXTURE_2D, g_textureID_1 );

    // STAGE 1
    glActiveTextureARB( GL_TEXTURE2_ARB );
    glBindTexture( GL_TEXTURE_2D, g_textureID_2 );
#endif

	cgGLBindProgram( g_CGprogram_pixel );
	cgGLEnableProfile( g_CGprofile_pixel );

#ifdef USE_CG_METHOD_OF_PASSING_MULTITEXTURE_ID
	cgGLEnableTextureParameter( g_CGparam_testTexture0 );
    cgGLEnableTextureParameter( g_CGparam_testTexture1 );
    cgGLEnableTextureParameter( g_CGparam_testTexture2 );
#endif

    g_fContributionOfTex1 = 1.0f - (g_fContributionOfTex0 + g_fContributionOfTex2);

    // Do some bounds checking...
    if( g_fContributionOfTex1 < 0.0f )
        g_fContributionOfTex1 = 0.0f;
    if( g_fContributionOfTex1 > 1.0f )
        g_fContributionOfTex1 = 1.0f;

    glColor4f( g_fContributionOfTex0, 
               g_fContributionOfTex1, 
               g_fContributionOfTex2, 1.0f );

    //
	// Render our quad with three sets of texture coordinates...
	//

	glBegin( GL_QUADS );
	{
        glMultiTexCoord2fARB( GL_TEXTURE0_ARB, 0.0f, 0.0f );
		glMultiTexCoord2fARB( GL_TEXTURE1_ARB, 0.0f, 0.0f );
        glMultiTexCoord2fARB( GL_TEXTURE2_ARB, 0.0f, 0.0f );
		glVertex3f( -1.0f, -1.0f, 0.0f );

		glMultiTexCoord2fARB( GL_TEXTURE0_ARB, 0.0f, 1.0f );
		glMultiTexCoord2fARB( GL_TEXTURE1_ARB, 0.0f, 1.0f );
        glMultiTexCoord2fARB( GL_TEXTURE2_ARB, 0.0f, 1.0f );
		glVertex3f( -1.0f, 1.0f, 0.0f );

		glMultiTexCoord2fARB( GL_TEXTURE0_ARB, 1.0f, 1.0f );
		glMultiTexCoord2fARB( GL_TEXTURE1_ARB, 1.0f, 1.0f );
        glMultiTexCoord2fARB( GL_TEXTURE2_ARB, 1.0f, 1.0f );
		glVertex3f( 1.0f,  1.0f, 0.0f );

		glMultiTexCoord2fARB( GL_TEXTURE0_ARB, 1.0f, 0.0f );
		glMultiTexCoord2fARB( GL_TEXTURE1_ARB, 1.0f, 0.0f );
        glMultiTexCoord2fARB( GL_TEXTURE2_ARB, 1.0f, 0.0f );
		glVertex3f( 1.0f, -1.0f, 0.0f );
	}
	glEnd();

#ifdef USE_CG_METHOD_OF_PASSING_MULTITEXTURE_ID
    cgGLDisableTextureParameter( g_CGparam_testTexture0 );
	cgGLDisableTextureParameter( g_CGparam_testTexture1 );
    cgGLDisableTextureParameter( g_CGparam_testTexture2 );
#endif

	cgGLDisableProfile( g_CGprofile_pixel );

#ifdef USE_OPENGL_METHOD_OF_PASSING_MULTITEXTURE_ID
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glDisable(GL_TEXTURE_2D);
    glActiveTextureARB(GL_TEXTURE1_ARB);
    glDisable(GL_TEXTURE_2D);
    glActiveTextureARB(GL_TEXTURE2_ARB);
    glDisable(GL_TEXTURE_2D);
#endif

    //
    // Output some info...
    //

    static char strContributionOfTex0[100];
    static char strContributionOfTex1[100];
    static char strContributionOfTex2[100];

    sprintf( strContributionOfTex0, "Contribution of Tex 0 = %f (Change: F1/F2)", g_fContributionOfTex0 );
    sprintf( strContributionOfTex1, "Contribution of Tex 1 = %f (Inferred by the values of Tex 0 & Tex 2)", g_fContributionOfTex1 );
    sprintf( strContributionOfTex2, "Contribution of Tex 2 = %f (Change: F3/F4)", g_fContributionOfTex2 );

	beginRenderText( g_nWindowWidth, g_nWindowHeight );
	{
        glColor3f( 1.0f, 1.0f, 0.0f );
        renderText( 5, 15, BITMAP_FONT_TYPE_HELVETICA_12, "Contribution of each texture for blending:" );
        glColor3f( 1.0f, 1.0f, 1.0f );
		renderText( 5, 30, BITMAP_FONT_TYPE_HELVETICA_12, strContributionOfTex0 );
		renderText( 5, 45, BITMAP_FONT_TYPE_HELVETICA_12, strContributionOfTex1 );
		renderText( 5, 60, BITMAP_FONT_TYPE_HELVETICA_12, strContributionOfTex2 );
	}
	endRenderText();

	SwapBuffers( g_hDC );
}
