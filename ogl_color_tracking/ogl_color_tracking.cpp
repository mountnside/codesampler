//-----------------------------------------------------------------------------
//           Name: ogl_color_tracking.cpp
//         Author: Kevin Harris
//  Last Modified: 04/28/05
//    Description: This sample demonstrates color-tracking and two-sided 
//                 lighting in OpenGL.
//
//                 Color tracking allows us to substitute the color of our 
//                 vertices for one or more of the material colors used by 
//                 OpenGL's lighting equation. This feature is typically not 
//                 used much anymore as since modelers today use textures to 
//                 color their geometry - not vertex colors. Of course, this 
//                 technique is alive and kicking in a billion lines of legacy 
//                 code so it's good to understand this technique just in case 
//                 you run across it.
//
//                 Two-sided lighting basically means that we want OpenGL to 
//                 light both sides of our geometry instead of just the front 
//                 faces. Again, this feature is typically not used much 
//                 anymore since it's very inefficient to light both sides of 
//                 every triangle but there are some cases where this is 
//                 helpful to know.
//                 
//   Control Keys: F1 - Toggle between a material color or color tracking the
//                      vertices
//                 F2 - Toggle two-sided lighting
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdlib.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glaux.h>
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

bool g_bColorTracking = false;
bool g_bTwoSidedLighting = true;

struct Vertex
{
	//GL_T2F_C4F_N3F_V3F
	float tu, tv;
	float r, g, b, a;
	float nx, ny, nz;
	float x, y, z;
};

// For testing purposes create a simple quad whose vertices are colored pure green
Vertex g_quadVertices[] =
{
	// tu,  tv     r    g    b    a      nx   ny  nz     x     y     z 
	{ 0.0f,0.0f,  0.0f,1.0f,0.0f,1.0f,  0.0f,0.0f,1.0, -1.0f,-1.0f, 0.0f },
	{ 1.0f,0.0f,  0.0f,1.0f,0.0f,1.0f,  0.0f,0.0f,1.0,  1.0f,-1.0f, 0.0f },
	{ 1.0f,1.0f,  0.0f,1.0f,0.0f,1.0f,  0.0f,0.0f,1.0,  1.0f, 1.0f, 0.0f },
	{ 0.0f,1.0f,  0.0f,1.0f,0.0f,1.0f,  0.0f,0.0f,1.0, -1.0f, 1.0f, 0.0f },
};

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE g_hInstance,HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND g_hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
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

	g_hWnd = CreateWindowEx( NULL,"MY_WINDOWS_CLASS",
						    "OpenGL - Color Tracking And Two-Sided Lighting",
							WS_OVERLAPPEDWINDOW,0,0, 640,480, NULL, NULL, g_hInstance, NULL );

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
                    g_bColorTracking = !g_bColorTracking;
                    break;

                case VK_F2:
                    g_bTwoSidedLighting = !g_bTwoSidedLighting;
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

    loadTexture();

	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0f, 640.0f / 480.0f, 0.1f, 100.0f );

    //
    // Set up some lighting...
    //

    glEnable( GL_LIGHTING );
    glEnable( GL_LIGHT0 );

    // Set light 0 to be a pure white directional light
    GLfloat diffuse_light0[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat specular_light0[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat position_light0[] = { 0.0f, 0.0f, 1.0f, 0.0f };
    glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuse_light0 );
    glLightfv( GL_LIGHT0, GL_SPECULAR, specular_light0 );
    glLightfv( GL_LIGHT0, GL_POSITION, position_light0 );

    // Enable some dim, grey ambient lighting so objects that are not lit 
    // by the other lights are not completely black.
	GLfloat ambient_lightModel[] = { 0.2f, 0.2f, 0.2f, 0.2f };
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

    //
    // Enable or disable color tracking...
    //

    if( g_bColorTracking == true )
    {
        // Instead of having a material provide the ambient and diffuse colors 
        // for the lighting equation, we'll use color-tracking to use the 
        // vertex color. Our test quad has green vertices so if we set color
        // tracking while rendering it, the quad will be considered green for 
        // lighting.

        glEnable( GL_COLOR_MATERIAL );

        // Now, tell OpenGL which material properties will be set to the vertex
        // color. 
        //
        // Note: If you don't use a vertex array to store your geometry data, 
        // a call to glColor will do the same thing.

        glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT );
        glColorMaterial( GL_FRONT_AND_BACK, GL_DIFFUSE );
    }
    else
    {
        // Stop using the quad's vertex colors in the lighting equation and use
        // the colors assigned via a material instead.

        glDisable( GL_COLOR_MATERIAL );

        // Set up a red material for the front side of the quad
        GLfloat ambient_mtrl_front[] = { 1.0f, 0.0f, 0.0f, 1.0f };
        GLfloat diffuse_mtrl_front[] = { 1.0f, 0.0f, 0.0f, 1.0f };
        glMaterialfv( GL_FRONT, GL_AMBIENT, ambient_mtrl_front ); 
        glMaterialfv( GL_FRONT, GL_DIFFUSE, diffuse_mtrl_front );

        // Set up a blue material for the backside of the quad
        GLfloat ambient_mtrl_back[] = { 0.0f, 0.0f, 1.0f, 1.0f };
        GLfloat diffuse_mtrl_back[] = { 0.0f, 0.0f, 1.0f, 1.0f };
        glMaterialfv( GL_BACK, GL_AMBIENT, ambient_mtrl_back ); 
        glMaterialfv( GL_BACK, GL_DIFFUSE, diffuse_mtrl_back );
    }

    //
    // Enable or disable two-sided lighting...
    //

    if( g_bTwoSidedLighting == true )
        glLightModeli( GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE );
    else
        glLightModeli( GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE );

	glBindTexture( GL_TEXTURE_2D, g_textureID );
    glInterleavedArrays( GL_T2F_C4F_N3F_V3F, 0, g_quadVertices );
    glDrawArrays( GL_QUADS, 0, 4 );

	SwapBuffers( g_hDC );
}
