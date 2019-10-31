//-----------------------------------------------------------------------------
//           Name: ogl_material.cpp
//         Author: Kevin Harris
//  Last Modified: 04/28/05
//    Description: This sample demonstrates how to use materials with lighting 
//                 to produce different surface effects.
//
//   Control Keys: Left Mouse Button - Spin the view
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "geometry.h"
#include "resource.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND  g_hWnd = NULL;
HDC	  g_hDC  = NULL;
HGLRC g_hRC  = NULL;

GLfloat g_redClayMtrl_diffuse[4];

GLfloat g_greenPlasticMtrl_diffuse[4];
GLfloat g_greenPlasticMtrl_ambient[4];
GLfloat g_greenPlasticMtrl_specular[4];
GLfloat g_greenPlasticMtrl_shininess;

GLfloat g_silverMetalMtrl_diffuse[4];
GLfloat g_silverMetalMtrl_ambient[4];
GLfloat g_silverMetalMtrl_emission[4];
GLfloat g_silverMetalMtrl_specular[4];
GLfloat g_silverMetalMtrl_shininess;

GLfloat g_default_diffuse[]  = { 0.8f, 0.8f, 0.8f, 1.0f };
GLfloat g_default_ambient[]  = { 0.2f, 0.2f, 0.2f, 1.0f };
GLfloat g_default_emission[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat g_default_specular[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat g_default_shininess  = 0.0f;

float g_fSpinZ = 0.0f;
float g_fSpinY = 0.0f;

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
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
                             "OpenGL - Materials",
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
                g_fSpinZ -= (ptCurrentMousePosit.x - ptLastMousePosit.x);
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
			gluPerspective( 45.0, (GLdouble)nWidth / (GLdouble)nHeight, 0.1, 1000.0);
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

	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_LIGHTING );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0f, 640.0f / 480.0f, 0.1f, 100.0f);

	//-------------------------------------------------------------------------
	// Setup material - Used to create a reddish clay teapot
	//-------------------------------------------------------------------------

	// A matte clay look is easy. All we really have to do is set the color to 
	// look like clay pottery.
	g_redClayMtrl_diffuse[0] = 1.0f;
	g_redClayMtrl_diffuse[1] = 0.5f;
	g_redClayMtrl_diffuse[2] = 0.2f;
	g_redClayMtrl_diffuse[3] = 1.0f;

	//-------------------------------------------------------------------------
	// Setup material - Used to create a green plastic teapot
	//-------------------------------------------------------------------------

	// Set the material's main color to green.
	g_greenPlasticMtrl_diffuse[0] = 0.0f;
	g_greenPlasticMtrl_diffuse[1] = 1.0f;
	g_greenPlasticMtrl_diffuse[2] = 0.0f;
	g_greenPlasticMtrl_diffuse[3] = 1.0f;

	// Lets favor the ambient's green over the other colors. Why? I don't know.
	// It just looks better to me. Using materials is some what more artistic
	// than scientific, so just play around till you get what you want.
	g_greenPlasticMtrl_ambient[0] = 0.5f;
	g_greenPlasticMtrl_ambient[1] = 1.0f;
	g_greenPlasticMtrl_ambient[2] = 0.5f;
	g_greenPlasticMtrl_ambient[3] = 1.0f;

	// Plastic can be shiny, but we don't want it too shiny are it will look 
	// more like glass or metal. We'll have the material reflect back more 
	// green than red and blue so the highlights won't be pure white.
	g_greenPlasticMtrl_specular[0] = 0.5f;
	g_greenPlasticMtrl_specular[1] = 1.0f;
	g_greenPlasticMtrl_specular[2] = 0.5f;
	g_greenPlasticMtrl_specular[3] = 1.0f;

	// It seems backwards, but increasing the shininess value reduces the 
	// highlight's size
	g_greenPlasticMtrl_shininess = 40.0f;

	//-------------------------------------------------------------------------
	// Setup material - Used to create a silver metallic teapot
	//-------------------------------------------------------------------------

	// Set the material's main color to a silver-like gray color.
	g_silverMetalMtrl_diffuse[0] = 0.5f;
	g_silverMetalMtrl_diffuse[1] = 0.5f;
	g_silverMetalMtrl_diffuse[2] = 0.5f;
	g_silverMetalMtrl_diffuse[3] = 1.0f;

	// A silver metal would be very shiny, so we'll reflect back all ambient.
	g_silverMetalMtrl_ambient[0] = 1.0f;
	g_silverMetalMtrl_ambient[1] = 1.0f;
	g_silverMetalMtrl_ambient[2] = 1.0f;
	g_silverMetalMtrl_ambient[3] = 1.0f;

	// We can make it seem extra shiny by having it actually emit some light 
	// of its own... but not too much are we'll wash the color out.
	g_silverMetalMtrl_emission[0] = 0.1f;
	g_silverMetalMtrl_emission[1] = 0.1f;
	g_silverMetalMtrl_emission[2] = 0.1f;
	g_silverMetalMtrl_emission[3] = 1.0f;

	// Polished silver can reflect back pure white highlights, so set the 
	// specular to pure white.
	g_silverMetalMtrl_specular[0] = 1.0f;
	g_silverMetalMtrl_specular[1] = 1.0f;
	g_silverMetalMtrl_specular[2] = 1.0f;
	g_silverMetalMtrl_specular[3] = 1.0f;

	// Set the Power value to a small number to make the highlight's size bigger.
	g_silverMetalMtrl_shininess = 5.0f;

	//
	// Setup a simple directional light and some ambient...
	//

	glEnable( GL_LIGHT0 );

	GLfloat position_light0[] = { -1.0f, 0.0f, 1.0f, 0.0f };
	GLfloat diffuse_light0[]  = {  1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat specular_light0[] = {  1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv( GL_LIGHT0, GL_POSITION, position_light0 );
	glLightfv( GL_LIGHT0, GL_DIFFUSE,  diffuse_light0 );
	glLightfv( GL_LIGHT0, GL_SPECULAR, specular_light0 );

	// Be careful when setting up ambient lighting. You can very easily wash 
	// out the material's specular highlights.

	GLfloat ambient_lightModel[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	glLightModelfv( GL_LIGHT_MODEL_AMBIENT, ambient_lightModel );
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )	
{
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
    glTranslatef( 0.0f, 0.0f, -6.5f );

    // -------------------------------------------------------------------------

	//
    // When using a material, its possible to have a color set by glColor3f 
    // override one or more material colors by enabling GL_COLOR_MATERIAL.
    // Once that's done, you can and ID the material color or colors to 
    // override using glColorMaterial.
	//

    // Uncomment this to use the teapot's color, which is blue, as a 
    // replacement for one or more material colors.
	//glEnable( GL_COLOR_MATERIAL );

    // Uncomment one or more of these to ID the material color or colors you 
    // would like to override?
	//glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT );
    //glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );
	//glColorMaterial( GL_FRONT_AND_BACK, GL_DIFFUSE );
	//glColorMaterial( GL_FRONT_AND_BACK, GL_SPECULAR );

    // -------------------------------------------------------------------------

	//
	// Render the first teapot using a red clay material
	//

	glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, g_redClayMtrl_diffuse );

	glPushMatrix();
	{
		glTranslatef( 0.0f, 1.2f, 0.0f );
		glRotatef( -g_fSpinY, 1.0f, 0.0f, 0.0f );
		glRotatef( -g_fSpinZ, 0.0f, 0.0f, 1.0f );

		glColor3f( 0.0f, 0.0f, 1.0f ); // This color won't get used unless we enable GL_COLOR_MATERIAL
		renderSolidTeapot( 1.0f );
	}
	glPopMatrix();

	//
	// Render the second teapot using a green plastic material
	//

	glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, g_greenPlasticMtrl_diffuse );
	glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT, g_greenPlasticMtrl_ambient );
	glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, g_greenPlasticMtrl_specular );
	glMaterialf( GL_FRONT_AND_BACK, GL_SHININESS, g_greenPlasticMtrl_shininess );

	glPushMatrix();
	{
		glTranslatef( 1.7f, -0.8f, 0.0f );
		glRotatef( -g_fSpinY, 1.0f, 0.0f, 0.0f );
		glRotatef( -g_fSpinZ, 0.0f, 0.0f, 1.0f );

		glColor3f( 0.0f, 0.0f, 1.0f ); // This color won't get used unless we enable GL_COLOR_MATERIAL
		renderSolidTeapot( 1.0f );
	}
	glPopMatrix();

	//
	// Render the third teapot using a silver metallic material
	//

	glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, g_silverMetalMtrl_diffuse );
	glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT, g_silverMetalMtrl_ambient );
	glMaterialfv( GL_FRONT_AND_BACK, GL_EMISSION, g_silverMetalMtrl_emission );
	glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, g_silverMetalMtrl_specular );
	glMaterialf( GL_FRONT_AND_BACK, GL_SHININESS, g_silverMetalMtrl_shininess );

	glPushMatrix();
	{
		glTranslatef( -1.7f, -0.8f, 0.0f );
		glRotatef( -g_fSpinY, 1.0f, 0.0f, 0.0f );
		glRotatef( -g_fSpinZ, 0.0f, 0.0f, 1.0f );

		glColor3f( 0.0f, 0.0f, 1.0f ); // This color won't get used unless we enable GL_COLOR_MATERIAL
		renderSolidTeapot( 1.0f );
	}
	glPopMatrix();

	//
	// Set all the material properties back to their default values...
	//

	glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, g_default_diffuse );
	glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT, g_default_ambient );
	glMaterialfv( GL_FRONT_AND_BACK, GL_EMISSION, g_default_emission );
	glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, g_default_specular );
	glMaterialf( GL_FRONT_AND_BACK, GL_SHININESS, g_default_shininess );

	SwapBuffers( g_hDC );
}
