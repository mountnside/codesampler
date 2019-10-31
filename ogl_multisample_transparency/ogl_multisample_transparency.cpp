//------------------------------------------------------------------------------
//           Name: ogl_multisample_transparency.cpp
//         Author: Kevin Harris
//  Last Modified: 12/13/07
//    Description: This sample demonstrates how to use multi-sample transparency 
//                 to skip the typically necessary step of sorting all 
//                 transparent geometry back to front. See the related sample
//                 "ogl_alpha_blending_texture" for the more typical setup which
//                 does require sorting.
//
//   Control Keys: b - Toggle blending via multi-sample transparency
//                 Up Arrow - Move the test cube closer
//                 Down Arrow - Move the test cube away
//
// NOTE: This sample will not work correctly unless the driver has been set to
//       use Antialiasing or Multisample Transparency. I can't tell you how to 
//       do this since every driver control panel does it a little differently.
//       For my particular nVIDIA card, I need to set the Antialias level to 4X 
//       to get the sample to work correctly.
//------------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdlib.h>
#include "glew.h"
#include "wglew.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include "tga.h"
#include "resource.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND   g_hWnd      = NULL;
HDC	   g_hDC       = NULL;
HGLRC  g_hRC       = NULL;
GLuint g_textureID = -1;

bool g_bBlending = true;

float g_fDistance = -4.5f;
float g_fSpinX    = 0.0f;
float g_fSpinY    = 0.0f;

struct Vertex
{
    float tu, tv;
    float x, y, z;
};

Vertex g_cubeVertices[] =
{
    { 0.0f,0.0f, -1.0f,-1.0f, 1.0f },
    { 1.0f,0.0f,  1.0f,-1.0f, 1.0f },
    { 1.0f,1.0f,  1.0f, 1.0f, 1.0f },
    { 0.0f,1.0f, -1.0f, 1.0f, 1.0f },
   
    { 1.0f,0.0f, -1.0f,-1.0f,-1.0f },
    { 1.0f,1.0f, -1.0f, 1.0f,-1.0f },
    { 0.0f,1.0f,  1.0f, 1.0f,-1.0f },
    { 0.0f,0.0f,  1.0f,-1.0f,-1.0f },
   
    { 0.0f,1.0f, -1.0f, 1.0f,-1.0f },
    { 0.0f,0.0f, -1.0f, 1.0f, 1.0f },
    { 1.0f,0.0f,  1.0f, 1.0f, 1.0f },
    { 1.0f,1.0f,  1.0f, 1.0f,-1.0f },
   
    { 1.0f,1.0f, -1.0f,-1.0f,-1.0f },
    { 0.0f,1.0f,  1.0f,-1.0f,-1.0f },
    { 0.0f,0.0f,  1.0f,-1.0f, 1.0f },
    { 1.0f,0.0f, -1.0f,-1.0f, 1.0f },
   
    { 1.0f,0.0f,  1.0f,-1.0f,-1.0f },
    { 1.0f,1.0f,  1.0f, 1.0f,-1.0f },
    { 0.0f,1.0f,  1.0f, 1.0f, 1.0f },
    { 0.0f,0.0f,  1.0f,-1.0f, 1.0f },
   
    { 0.0f,0.0f, -1.0f,-1.0f,-1.0f },
    { 1.0f,0.0f, -1.0f,-1.0f, 1.0f },
    { 1.0f,1.0f, -1.0f, 1.0f, 1.0f },
    { 0.0f,1.0f, -1.0f, 1.0f,-1.0f }
};

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
                             "OpenGL - Multi-sample Transparency Blending",
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
        case WM_CHAR:
		{
			switch( wParam )
			{
                case 'b':
                case 'B':
                    g_bBlending = !g_bBlending;
                    break;
            }
        }
        break;

        case WM_KEYDOWN:
		{
			switch( wParam )
			{
				case VK_ESCAPE:
					PostQuitMessage(0);
					break;

                case 38: // Up Arrow Key
                    g_fDistance -= 0.1f;
                    break;

                case 40: // Down Arrow Key
                    g_fDistance += 0.1f;
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
		break;

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
    tgaImageFile tgaImage;
    tgaImage.load( "radiation_box.tga" );

    glGenTextures( 1, &g_textureID );

    glBindTexture( GL_TEXTURE_2D, g_textureID );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    glTexImage2D( GL_TEXTURE_2D, 0, tgaImage.m_texFormat, 
                  tgaImage.m_nImageWidth, tgaImage.m_nImageHeight, 
                  0, tgaImage.m_texFormat, GL_UNSIGNED_BYTE, 
                  tgaImage.m_nImageData );
}

//-----------------------------------------------------------------------------
// Name: init()
// Desc: 
//-----------------------------------------------------------------------------
void init( void )
{
    //
    // We need to create a dummy window to init GLEW...
    //

    HGLRC hRC = NULL;
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
	hRC = wglCreateContext( g_hDC );
	wglMakeCurrent( g_hDC, hRC );

    // Init GLEW and verify that we got the extensions we need...
    GLenum err = glewInit();

    if( err != GLEW_OK )
    {
        static char str[255];
        sprintf( str, "Error: %s\n", glewGetErrorString(err) );
        MessageBox( NULL, str, "ERROR", MB_OK|MB_ICONEXCLAMATION );
    }

    if( hRC != NULL )
	{
		wglMakeCurrent( NULL, NULL );
		wglDeleteContext( hRC );
		hRC = NULL;
	}

    //
	// GLEW is not setup. Use wglChoosePixelFormatARB to find a valid 
    // multisample configuration.
	//

	int pf_attr[] =
	{
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,    // Our context will be used with OpenGL
		WGL_RED_BITS_ARB, 8,                // At least 8 bits for RED channel
		WGL_GREEN_BITS_ARB, 8,              // At least 8 bits for GREEN channel
		WGL_BLUE_BITS_ARB, 8,               // At least 8 bits for BLUE channel
		WGL_ALPHA_BITS_ARB, 8,              // At least 8 bits for ALPHA channel
		WGL_DEPTH_BITS_ARB, 24,             // At least 16 bits for depth buffer
		WGL_DOUBLE_BUFFER_ARB, GL_TRUE,     // We don require double buffering
        WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,    // Turn Multi-sampling on
		WGL_SAMPLES_ARB, 4,                 // Check For 4x Multi-sampling
		0                                   // Zero terminates the list
	};

	unsigned int count = 0;
    int nPixelFormat;
	wglChoosePixelFormatARB( g_hDC,(const int*)pf_attr, NULL, 1, &nPixelFormat, &count);

	if( count == 0 )
	{
		MessageBox(NULL,"Could not find an acceptable pixel format!",
			"ERROR",MB_OK|MB_ICONEXCLAMATION);
		exit(-1);
	}

    g_hDC = GetDC( g_hWnd );
	SetPixelFormat( g_hDC, nPixelFormat, NULL );
	g_hRC = wglCreateContext( g_hDC );
	wglMakeCurrent( g_hDC, g_hRC );

	loadTexture();

	glClearColor( 0.35f, 0.53f, 0.7f, 1.0f );
	glEnable( GL_TEXTURE_2D );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0f, 640.0f / 480.0f, 0.1f, 100.0f);
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
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glTranslatef( 0.0f, 0.0f, g_fDistance );
    glRotatef( -g_fSpinY, 1.0f, 0.0f, 0.0f );
    glRotatef( -g_fSpinX, 0.0f, 1.0f, 0.0f );

    if( g_bBlending == true )
	{
        glEnable( GL_DEPTH_TEST );
        glEnable( GL_MULTISAMPLE_ARB );
        glEnable( GL_SAMPLE_ALPHA_TO_COVERAGE_ARB );

        glBindTexture( GL_TEXTURE_2D, g_textureID );
        glInterleavedArrays( GL_T2F_V3F, 0, g_cubeVertices );
        glDrawArrays( GL_QUADS, 0, 24 );
    }
    else
    {
        glDisable( GL_MULTISAMPLE_ARB );
        glDisable( GL_SAMPLE_ALPHA_TO_COVERAGE_ARB );

        glBindTexture( GL_TEXTURE_2D, g_textureID );
        glInterleavedArrays( GL_T2F_V3F, 0, g_cubeVertices );
        glDrawArrays( GL_QUADS, 0, 24 );
    }

	SwapBuffers( g_hDC );
}
