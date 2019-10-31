//-----------------------------------------------------------------------------
//           Name: ogl_offscreen_rendering_3.cpp
//         Author: Kevin Harris 
//  Last Modified: 07/06/05
//    Description: This sample demonstrates how to create dynamic textures 
//                 through off-screen rendering. The off-screen rendering step 
//                 is accomplished using a p-buffer, which is created using 
//                 OpenGL's WGL_ARB_pbuffer, and WGL_ARB_pixel_format 
//                 extensions. The step of dynamic texture creation from a 
//                 p-buffer requires the WGL_ARB_render_texture extension.
//
//                 As a demonstration, a spinning textured cube is rendered 
//                 to a p-buffer, which is in turn, used to create a dynamic 
//                 texture. The dynamic texture is then used to texture a 
//                 second spinning cube, which will be rendered to the 
//                 application's window.
//
//   Control Keys: Left Mouse Button  - Spin the large, black cube.
//                 Right Mouse Button - Spin the textured cube being rendered 
//                                      into the p-buffer.
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
// Note: The EXT_framebuffer_object extension has become an excellent 
//       replacement for the WGL_ARB_pbuffer and WGL_ARB_render_texture combo 
//       which is normally used to create dynamic textures. An example of this 
//       newer technique can be found here.
//
//       http://www.codesampler.com/oglsrc/oglsrc_14.htm#ogl_frame_buffer_object
//------------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdlib.h>
#include <mmsystem.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glaux.h>
#include "resource.h"

//-----------------------------------------------------------------------------
// FUNCTION POINTERS FOR OPENGL EXTENSIONS
//-----------------------------------------------------------------------------

// For convenience, this project ships with its own "wglext.h" extension header 
// file. If you have trouble running this sample, it may be that this "wglext.h" 
// file is defining something that your hardware doesn’t actually support. 
// Try recompiling the sample using your own local, vendor-specific "wglext.h" 
// header file.

#include "wglext.h"      // Sample's header file
//#include <GL/wglext.h> // Your local header file

// WGL_ARB_extensions_string
PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB = NULL;

// WGL_ARB_pbuffer
PFNWGLCREATEPBUFFERARBPROC    wglCreatePbufferARB    = NULL;
PFNWGLGETPBUFFERDCARBPROC     wglGetPbufferDCARB     = NULL;
PFNWGLRELEASEPBUFFERDCARBPROC wglReleasePbufferDCARB = NULL;
PFNWGLDESTROYPBUFFERARBPROC   wglDestroyPbufferARB   = NULL;
PFNWGLQUERYPBUFFERARBPROC     wglQueryPbufferARB     = NULL;

// WGL_ARB_pixel_format
PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = NULL;

// WGL_ARB_render_texture
PFNWGLBINDTEXIMAGEARBPROC     wglBindTexImageARB     = NULL;
PFNWGLRELEASETEXIMAGEARBPROC  wglReleaseTexImageARB  = NULL;
PFNWGLSETPBUFFERATTRIBARBPROC wglSetPbufferAttribARB = NULL;

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND   g_hWnd             = NULL;
HDC	   g_hDC              = NULL;
HGLRC  g_hRC              = NULL;
GLuint g_testTextureID    = -1;
GLuint g_dynamicTextureID = -1;

float  g_fSpinX_L = 0.0f;
float  g_fSpinY_L = 0.0f;
float  g_fSpinX_R = 0.0f;
float  g_fSpinY_R = 0.0f;

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

// This little struct will help to organize our p-buffer's data
typedef struct {

    HPBUFFERARB hPBuffer;
    HDC         hDC;
    HGLRC       hRC;
    int         nWidth;
    int         nHeight;

} PBUFFER;

PBUFFER g_pbuffer;

const int PBUFFER_WIDTH  = 256;
const int PBUFFER_HEIGHT = 256;

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
		                     "OpenGL - Off-Screen Rendering Using P-Buffers",
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
	static POINT ptLastMousePosit_L;
	static POINT ptCurrentMousePosit_L;
	static bool  bMousing_L;
	
	static POINT ptLastMousePosit_R;
	static POINT ptCurrentMousePosit_R;
	static bool  bMousing_R;

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
			ptLastMousePosit_L.x = ptCurrentMousePosit_L.x = LOWORD (lParam);
            ptLastMousePosit_L.y = ptCurrentMousePosit_L.y = HIWORD (lParam);
			bMousing_L = true;
		}
		break;

		case WM_LBUTTONUP:
		{
			bMousing_L = false;
		}
		break;

		case WM_RBUTTONDOWN:
		{
			ptLastMousePosit_R.x = ptCurrentMousePosit_R.x = LOWORD (lParam);
            ptLastMousePosit_R.y = ptCurrentMousePosit_R.y = HIWORD (lParam);
			bMousing_R = true;
		}
		break;

		case WM_RBUTTONUP:
		{
			bMousing_R = false;
		}
		break;

		case WM_MOUSEMOVE:
		{
			ptCurrentMousePosit_L.x = LOWORD (lParam);
			ptCurrentMousePosit_L.y = HIWORD (lParam);
			ptCurrentMousePosit_R.x = LOWORD (lParam);
			ptCurrentMousePosit_R.y = HIWORD (lParam);

			if( bMousing_L )
			{
				g_fSpinX_L -= (ptCurrentMousePosit_L.x - ptLastMousePosit_L.x);
				g_fSpinY_L -= (ptCurrentMousePosit_L.y - ptLastMousePosit_L.y);
			}
			
			if( bMousing_R )
			{
				g_fSpinX_R -= (ptCurrentMousePosit_R.x - ptLastMousePosit_R.x);
				g_fSpinY_R -= (ptCurrentMousePosit_R.y - ptLastMousePosit_R.y);
			}

			ptLastMousePosit_L.x = ptCurrentMousePosit_L.x;
            ptLastMousePosit_L.y = ptCurrentMousePosit_L.y;
			ptLastMousePosit_R.x = ptCurrentMousePosit_R.x;
            ptLastMousePosit_R.y = ptCurrentMousePosit_R.y;
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
	AUX_RGBImageRec *pTextureImage = auxDIBImageLoad( ".\\test.bmp" );

    if( pTextureImage != NULL )
	{
		glGenTextures( 1, &g_testTextureID );

		glBindTexture(GL_TEXTURE_2D, g_testTextureID);

		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

		glTexImage2D( GL_TEXTURE_2D, 0, 3,pTextureImage->sizeX,pTextureImage->sizeY, 0,
			GL_RGB, GL_UNSIGNED_BYTE,pTextureImage->data );
	}

	if( pTextureImage != NULL )
	{
		if( pTextureImage->data != NULL )
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
	SetPixelFormat( g_hDC, PixelFormat, &pfd);
	g_hRC = wglCreateContext( g_hDC );
	wglMakeCurrent( g_hDC, g_hRC );

	glClearColor( 0.0f, 0.0f, 1.0f, 1.0f );
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0f, 640.0f / 480.0f, 0.1f, 100.0f);

	//
	// This is our dynamic texture, which will be loaded with new pixel data
	// after we're finshed rendering to the p-buffer.
	//

	glGenTextures( 1, &g_dynamicTextureID );
	glBindTexture( GL_TEXTURE_2D, g_dynamicTextureID );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, PBUFFER_WIDTH, PBUFFER_HEIGHT, 0, GL_RGB, GL_FLOAT, 0 );
	glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR );

	//
	// If the required extensions are present, get the addresses for the
	// functions that we wish to use...
	//

	wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");
	char *ext = NULL;
	
	if( wglGetExtensionsStringARB )
		ext = (char*)wglGetExtensionsStringARB( wglGetCurrentDC() );
	else
	{
		MessageBox(NULL,"Unable to get address for wglGetExtensionsStringARB!",
			"ERROR",MB_OK|MB_ICONEXCLAMATION);
		exit(-1);
	}

	//
	// WGL_ARB_pbuffer
	//

	if( strstr( ext, "WGL_ARB_pbuffer" ) == NULL )
	{
		MessageBox(NULL,"WGL_ARB_pbuffer extension was not found",
			"ERROR",MB_OK|MB_ICONEXCLAMATION);
		exit(-1);
	}
	else
	{
		wglCreatePbufferARB    = (PFNWGLCREATEPBUFFERARBPROC)wglGetProcAddress("wglCreatePbufferARB");
		wglGetPbufferDCARB     = (PFNWGLGETPBUFFERDCARBPROC)wglGetProcAddress("wglGetPbufferDCARB");
		wglReleasePbufferDCARB = (PFNWGLRELEASEPBUFFERDCARBPROC)wglGetProcAddress("wglReleasePbufferDCARB");
		wglDestroyPbufferARB   = (PFNWGLDESTROYPBUFFERARBPROC)wglGetProcAddress("wglDestroyPbufferARB");
		wglQueryPbufferARB     = (PFNWGLQUERYPBUFFERARBPROC)wglGetProcAddress("wglQueryPbufferARB");

		if( !wglCreatePbufferARB || !wglGetPbufferDCARB || !wglReleasePbufferDCARB ||
			!wglDestroyPbufferARB || !wglQueryPbufferARB )
		{
			MessageBox(NULL,"One or more WGL_ARB_pbuffer functions were not found",
				"ERROR",MB_OK|MB_ICONEXCLAMATION);
			exit(-1);
		}
	}

	//
	// WGL_ARB_pixel_format
	//

	if( strstr( ext, "WGL_ARB_pixel_format" ) == NULL )
	{
		MessageBox(NULL,"WGL_ARB_pixel_format extension was not found",
			"ERROR",MB_OK|MB_ICONEXCLAMATION);
		return;
	}
	else
	{
		wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");

		if( !wglChoosePixelFormatARB )
		{
			MessageBox(NULL,"One or more WGL_ARB_pixel_format functions were not found",
				"ERROR",MB_OK|MB_ICONEXCLAMATION);
			exit(-1);
		}
	}

	//
	// WGL_ARB_render_texture
	//

	if( strstr( ext, "WGL_ARB_render_texture" ) == NULL )
	{
		MessageBox(NULL,"WGL_ARB_render_texture extension was not found",
			"ERROR",MB_OK|MB_ICONEXCLAMATION);
		exit(-1);
	}
	else
	{
		wglBindTexImageARB     = (PFNWGLBINDTEXIMAGEARBPROC)wglGetProcAddress("wglBindTexImageARB");
		wglReleaseTexImageARB  = (PFNWGLRELEASETEXIMAGEARBPROC)wglGetProcAddress("wglReleaseTexImageARB");
		wglSetPbufferAttribARB = (PFNWGLSETPBUFFERATTRIBARBPROC)wglGetProcAddress("wglSetPbufferAttribARB");

		if( !wglBindTexImageARB || !wglReleaseTexImageARB || !wglSetPbufferAttribARB )
		{
			MessageBox(NULL,"One or more WGL_ARB_render_texture functions were not found",
				"ERROR",MB_OK|MB_ICONEXCLAMATION);
			exit(-1);
		}
	}

	//-------------------------------------------------------------------------
	// Create a p-buffer for off-screen rendering.
	//-------------------------------------------------------------------------

	g_pbuffer.hPBuffer = NULL;
	g_pbuffer.nWidth   = PBUFFER_WIDTH;
	g_pbuffer.nHeight  = PBUFFER_HEIGHT;

	//
	// Define the minimum pixel format requirements we will need for our 
	// p-buffer. A p-buffer is just like a frame buffer, it can have a depth 
	// buffer associated with it and it can be double buffered.
	//

	int pf_attr[] =
	{
		WGL_SUPPORT_OPENGL_ARB, TRUE,       // P-buffer will be used with OpenGL
		WGL_DRAW_TO_PBUFFER_ARB, TRUE,      // Enable render to p-buffer
		WGL_BIND_TO_TEXTURE_RGBA_ARB, TRUE, // P-buffer will be used as a texture
		WGL_RED_BITS_ARB, 8,                // At least 8 bits for RED channel
		WGL_GREEN_BITS_ARB, 8,              // At least 8 bits for GREEN channel
		WGL_BLUE_BITS_ARB, 8,               // At least 8 bits for BLUE channel
		WGL_ALPHA_BITS_ARB, 8,              // At least 8 bits for ALPHA channel
		WGL_DEPTH_BITS_ARB, 16,             // At least 16 bits for depth buffer
		WGL_DOUBLE_BUFFER_ARB, FALSE,       // We don't require double buffering
		0                                   // Zero terminates the list
	};

	unsigned int count = 0;
	int pixelFormat;
	wglChoosePixelFormatARB( g_hDC,(const int*)pf_attr, NULL, 1, &pixelFormat, &count);

	if( count == 0 )
	{
		MessageBox(NULL,"Could not find an acceptable pixel format!",
			"ERROR",MB_OK|MB_ICONEXCLAMATION);
		exit(-1);
	}

	//
	// Set some p-buffer attributes so that we can use this p-buffer as a
	// 2D RGBA texture target.
	//

	int pb_attr[] =
	{
		WGL_TEXTURE_FORMAT_ARB, WGL_TEXTURE_RGBA_ARB, // Our p-buffer will have a texture format of RGBA
		WGL_TEXTURE_TARGET_ARB, WGL_TEXTURE_2D_ARB,   // Of texture target will be GL_TEXTURE_2D
		0                                             // Zero terminates the list
	};

	//
	// Create the p-buffer...
	//

	g_pbuffer.hPBuffer = wglCreatePbufferARB( g_hDC, pixelFormat, g_pbuffer.nWidth, g_pbuffer.nHeight, pb_attr );
	g_pbuffer.hDC      = wglGetPbufferDCARB( g_pbuffer.hPBuffer );
	g_pbuffer.hRC      = wglCreateContext( g_pbuffer.hDC );

	if( !g_pbuffer.hPBuffer )
	{
		MessageBox(NULL,"Could not create the p-buffer",
			"ERROR",MB_OK|MB_ICONEXCLAMATION);
		exit(-1);
	}

	int h;
	int w;
	wglQueryPbufferARB( g_pbuffer.hPBuffer, WGL_PBUFFER_WIDTH_ARB, &h );
	wglQueryPbufferARB( g_pbuffer.hPBuffer, WGL_PBUFFER_WIDTH_ARB, &w );

	if( h != g_pbuffer.nHeight || w != g_pbuffer.nWidth )
	{
		MessageBox(NULL,"The width and height of the created p-buffer don't match the requirements!",
			"ERROR",MB_OK|MB_ICONEXCLAMATION);
		exit(-1);
	}

	//
	// We were successful in creating a p-buffer. We can now make its context 
	// current and set it up just like we would a regular context 
	// attached to a window.
	//

	if( !wglMakeCurrent( g_pbuffer.hDC, g_pbuffer.hRC ) ) 
	{
		MessageBox(NULL,"Could not make the p-buffer's context current!",
			"ERROR",MB_OK|MB_ICONEXCLAMATION);
		exit(-1);
	}

	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0f, PBUFFER_WIDTH / PBUFFER_HEIGHT, 0.1f, 10.0f );

	loadTexture();
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )
{
    glDeleteTextures( 1, &g_testTextureID );
    glDeleteTextures( 1, &g_dynamicTextureID );

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

	//
	// Don't forget to clean up after our p-buffer...
	//

	if( g_pbuffer.hRC != NULL )
	{
		wglMakeCurrent( g_pbuffer.hDC, g_pbuffer.hRC );
		wglDeleteContext( g_pbuffer.hRC );
		wglReleasePbufferDCARB( g_pbuffer.hPBuffer, g_pbuffer.hDC );
		wglDestroyPbufferARB( g_pbuffer.hPBuffer );
		g_pbuffer.hRC = NULL;
	}

	if( g_pbuffer.hDC != NULL )
	{
		ReleaseDC( g_hWnd, g_pbuffer.hDC );
		g_pbuffer.hDC = NULL;
	}
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: 
//-----------------------------------------------------------------------------
void render( void )
{
	//-------------------------------------------------------------------------
	// Make sure we haven't lost our p-buffer due to a display mode change.
    // If we haven't, make the p-buffer's context current, so we can do an 
    // off-screen render to it...
	//-------------------------------------------------------------------------

    int flag = 0;
    wglQueryPbufferARB( g_pbuffer.hPBuffer, WGL_PBUFFER_LOST_ARB, &flag );

    if( flag != 0 )
    {
        MessageBox(NULL,"The p-buffer was lost!",
            "ERROR",MB_OK|MB_ICONEXCLAMATION);
        exit(-1);
    }

	if( !wglMakeCurrent( g_pbuffer.hDC, g_pbuffer.hRC ) )
	{
		MessageBox(NULL,"Could not make the p-buffer's context current!",
			"ERROR",MB_OK|MB_ICONEXCLAMATION);
        exit(-1);
    }

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	//
	// Let the user spin the cube about with the right mouse button, so our 
	// dynamic texture will show motion.
	//

	glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
	glTranslatef( 0.0f, 0.0f, -5.0f );
    glRotatef( -g_fSpinY_R, 1.0f, 0.0f, 0.0f );
    glRotatef( -g_fSpinX_R, 0.0f, 1.0f, 0.0f );

	//
	// Now the render the cube to the p-buffer just like you we would have 
	// done with the regular window.
	//

    glBindTexture( GL_TEXTURE_2D, g_testTextureID );
    glInterleavedArrays( GL_T2F_V3F, 0, g_cubeVertices );
    glDrawArrays( GL_QUADS, 0, 24 );

	//-------------------------------------------------------------------------
	// Now, make the window's context current for regular rendering...
	//-------------------------------------------------------------------------

	if( !wglMakeCurrent( g_hDC, g_hRC ) )
	{
		MessageBox(NULL,"Could not make the window's context current!",
			"ERROR",MB_OK|MB_ICONEXCLAMATION);
        exit(-1);
    }

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	//
	// Let the user spin the cube about with the left mouse button.
	//

	glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
	glTranslatef( 0.0f, 0.0f, -5.0f );
    glRotatef( -g_fSpinY_L, 1.0f, 0.0f, 0.0f );
    glRotatef( -g_fSpinX_L, 0.0f, 1.0f, 0.0f );

    //
    // Finally, we'll use the dynamic texture like a regular static texture.
    //

	//
	// Bind the dynamic texture like you normally would, then bind the
	// p-buffer to it.
	//

    glBindTexture( GL_TEXTURE_2D, g_dynamicTextureID );

	if( !wglBindTexImageARB( g_pbuffer.hPBuffer, WGL_FRONT_LEFT_ARB ) )
	{
		MessageBox(NULL,"Could not bind p-buffer to render texture!",
			"ERROR",MB_OK|MB_ICONEXCLAMATION);
		exit(-1);
	}

    glInterleavedArrays( GL_T2F_V3F, 0, g_cubeVertices );
    glDrawArrays( GL_QUADS, 0, 24 );

    //
    // Before we forget, we need to make sure that the p-buffer has been 
    // released from the dynamic "render-to" texture.
    //

    if( !wglReleaseTexImageARB( g_pbuffer.hPBuffer, WGL_FRONT_LEFT_ARB ) )
    {
        MessageBox(NULL,"Could not release p-buffer from render texture!",
            "ERROR",MB_OK|MB_ICONEXCLAMATION);
        exit(-1);
    }

	SwapBuffers( g_hDC );
}
