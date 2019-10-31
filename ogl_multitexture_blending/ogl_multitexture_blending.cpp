//-----------------------------------------------------------------------------
//           Name: ogl_multitexture_blending.cpp
//         Author: Kevin Harris
//  Last Modified: 04/26/05
//    Description: This sample demonstrates how to use the OpenGL extensions 
//                 GL_ARB_multitexture and GL_ARB_texture_env_combine in
//                 conjunction with specially encoded vertex colors to blend 
//                 three textures together.
//
//                 This technique is very popular in terrain rendering engines 
//                 which use it to blend dramatically different textures such  
//                 as rock and grass together with out creating a noticeable 
//                 edge. For example, with three textures consisting of stone,
//                 grass, and sand you can render a mountain that blends in 
//                 patches of grass and sand at its base.
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
// The technique basically consists of the following steps:
//
// Step 1: Take the desired contribution of the three textures and encode them
//         into the vertex's color such that the RGB portion of the color 
//         controls the interpolation between texture stages 0 and 1, and the 
//         color's ALPHA controls the interpolation between texture stages 
//         1 and 2.
// 
// Step 2: Use GL_ARB_multitexture to apply three textures simultaneously to 
//         our geometry.
// 
// Step 3: Set the first texture on texture stage 0.
// 
// Step 4: During texture stage 1, use GL_INTERPOLATE_ARB to linearly 
//         interpolate between the output of stage 0 and the texture of stage 1
//         with GL_SRC_COLOR (i.e. the RGB part of the color).
//         
// Step 4: During texture stage 2, use GL_INTERPOLATE_ARB to linearly 
//         interpolate between the output of stage 1 and the texture of stage 2 
//         with GL_SRC_ALPHA (i.e. the ALPHA part of the color).
//
//   Control Keys: F1   - Increase contribution of texture 0
//                 F2   - Decrease contribution of texture 0
//                 F3   - Increase contribution of texture 2
//                 F4   - Decrease contribution of texture 2
//                 F5   - Toggle wire-frame mode
//                 Up   - View moves forward
//                 Down - View moves backward
//
// Note: I tried to create an intuitive way to set the contribution of each 
//       texture at run-time using the function keys, but this system is still
//       a little confusing since I only allow the contribution of texture 0 
//       and texture 2 to be adjusted. This is due to the fact that the
//       equation for encoding the blending info into the vertex color simply 
//       infers the contribution value of texture 1 based on the values for 
//       textures 0 and 2. Therefore, the contribution value of texture 1 must
//       be indirectly set by adjusting the contributions of textures 0 and 2.
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
// Note: While this technique remains popular as a fall-back for older 
//       hardware, shaders make this task a lot easier and are quickly becoming 
//       the preferred method for terrain texture blending. An example fo how 
//       this technique might work, can be found here:
//
// http://www.codesampler.com/oglsrc.htm#ogl_cg_multitexture_blending
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
#include "resource.h"
#include "bitmap_fonts.h"

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
                             "OpenGL - Multi-Texture Blending",
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

    //
    // Texture Stage 0
    //
    // Simply output texture0 for stage 0.
    //

    glActiveTextureARB( GL_TEXTURE0_ARB );
    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, g_textureID_0 );

    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

    //
    // Texture Stage 1
    //
    // Perform a linear interpolation between the output of stage 0 
    // (i.e texture0) and texture1 and use the RGB portion of the vertex's 
    // color to mix the two. 
    //

    glActiveTextureARB(GL_TEXTURE1_ARB );
    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, g_textureID_1 );

    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB );
    glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_INTERPOLATE_ARB );

    glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PREVIOUS_ARB );
    glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR );

    glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_TEXTURE );
    glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR );

    glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE2_RGB_ARB, GL_PRIMARY_COLOR_ARB );
    glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND2_RGB_ARB, GL_SRC_COLOR );

    //
    // Texture Stage 2
    //
    // Perform a linear interpolation between the output of stage 1 
    // (i.e texture0 mixed with texture1) and texture2 and use the ALPHA 
    // portion of the vertex's color to mix the two. 
    //

    glActiveTextureARB( GL_TEXTURE2_ARB );
    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, g_textureID_2 );

    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB );
    glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_INTERPOLATE_ARB );

    glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PREVIOUS_ARB );
    glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR );

    glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_TEXTURE );
    glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR );

    glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE2_RGB_ARB, GL_PRIMARY_COLOR_ARB );
    glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND2_RGB_ARB, GL_SRC_ALPHA );

    //
    // Based on the contributions of texture 0 and texture 2 what is the
    // contribution of texture 1? We don't really need this for the encoding
    // process below. I'm simply calculating it so I can output its value later.
    //

    g_fContributionOfTex1 = 1.0f - (g_fContributionOfTex0 + g_fContributionOfTex2);

    // Do some bounds checking...
    if( g_fContributionOfTex1 < 0.0f )
        g_fContributionOfTex1 = 0.0f;
    if( g_fContributionOfTex1 > 1.0f )
        g_fContributionOfTex1 = 1.0f;

    //
    // Now, lets encode the blending information into the vertex color.
    // The value set into the RGB part of the color controls the blending
    // between texture 0 and texture 1, and the alpha part of the color
    // controls the blending between textures1 and textures 2.
    //
    // Note: We use the contribution of texture 0 and 2 to infer or deduce 
    // the contribution of texture 1. We can do this because we know that the
    // total contribution of our three textures must add up to 1.0f.
    //

    float rgbValue   = g_fContributionOfTex0 / (1.0f - g_fContributionOfTex2);
    float alphaValue = 1.0f - g_fContributionOfTex2;

    // Do some bounds checking...
    if( rgbValue < 0.0f )
        rgbValue = 0.0f;
    if( rgbValue > 1.0f )
        rgbValue = 1.0f;

    if( alphaValue < 0.0f )
        alphaValue = 0.0f;
    if( alphaValue > 1.0f )
        alphaValue = 1.0f;

    glColor4f( rgbValue, rgbValue, rgbValue, alphaValue );
    
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

    glActiveTextureARB(GL_TEXTURE0_ARB);
    glDisable(GL_TEXTURE_2D);
    glActiveTextureARB(GL_TEXTURE1_ARB);
    glDisable(GL_TEXTURE_2D);
    glActiveTextureARB(GL_TEXTURE2_ARB);
    glDisable(GL_TEXTURE_2D);

    //
    // Output some info...
    //

    static char strContributionOfTex0[100];
    static char strContributionOfTex1[100];
    static char strContributionOfTex2[100];

    sprintf( strContributionOfTex0, "Contribution of Tex 0 = %f (Change: F1/F2)", g_fContributionOfTex0 );
    sprintf( strContributionOfTex1, "Contribution of Tex 1 = %f (Inferred by the values of Tex 0 & Tex 2)", g_fContributionOfTex1 );
    sprintf( strContributionOfTex2, "Contribution of Tex 2 = %f (Change: F3/F4)", g_fContributionOfTex2 );

    static char strRedValue_int[50];
    static char strGreenValue_int[50];
    static char strBlueValue_int[50];
    static char strAlphaValue_int[50];

    sprintf( strRedValue_int, "Red = %f", rgbValue );
    sprintf( strGreenValue_int, "Green = %f", rgbValue );
    sprintf( strBlueValue_int, "Blue = %f", rgbValue );
    sprintf( strAlphaValue_int, "Alpha = %f", alphaValue );

	beginRenderText( g_nWindowWidth, g_nWindowHeight );
	{
        glColor3f( 1.0f, 1.0f, 0.0f );
        renderText( 5, 15, BITMAP_FONT_TYPE_HELVETICA_12, "Contribution of each texture for blending:" );
        glColor3f( 1.0f, 1.0f, 1.0f );
		renderText( 5, 30, BITMAP_FONT_TYPE_HELVETICA_12, strContributionOfTex0 );
		renderText( 5, 45, BITMAP_FONT_TYPE_HELVETICA_12, strContributionOfTex1 );
		renderText( 5, 60, BITMAP_FONT_TYPE_HELVETICA_12, strContributionOfTex2 );

		glColor3f( 1.0f, 1.0f, 0.0f );
        renderText( 5, 355, BITMAP_FONT_TYPE_HELVETICA_12, "RGB values passed for interpolation of texture stage 1" );
        glColor3f( 1.0f, 1.0f, 1.0f );
		renderText( 5, 370, BITMAP_FONT_TYPE_HELVETICA_12, strRedValue_int );
		renderText( 5, 385, BITMAP_FONT_TYPE_HELVETICA_12, strGreenValue_int );
		renderText( 5, 400, BITMAP_FONT_TYPE_HELVETICA_12, strBlueValue_int );

        glColor3f( 1.0f, 1.0f, 0.0f );
        renderText( 5, 415, BITMAP_FONT_TYPE_HELVETICA_12, "ALPHA value passed for interpolation of texture stage 2" );
        glColor3f( 1.0f, 1.0f, 1.0f );
		renderText( 5, 430, BITMAP_FONT_TYPE_HELVETICA_12, strAlphaValue_int );
	}
	endRenderText();

	SwapBuffers( g_hDC );
}
