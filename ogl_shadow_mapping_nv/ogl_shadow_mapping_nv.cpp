//-----------------------------------------------------------------------------
//           Name: ogl_shadow_mapping_nv.cpp
//         Author: Kevin Harris
//  Last Modified: 06/11/05
//    Description: This sample demonstrates how to perform shadow mapping with 
//                 OpenGL. 
//
//                 The shadow mapping technique basically uses a depth texture 
//                 or "shadow map" to figure out whether or not a pixel's Z 
//                 value places it within a shadowed region or not.
//                 
//                 This involves two render passes. The first pass uses a 
//                 p-buffer to create a depth texture from the light's point 
//                 of view. 
//                 
//                 On the second pass, we render our 3D content and perform a 
//                 depth comparison between each pixel generated and the depth 
//                 texture. If the pixel has a greater Z value than the depth 
//                 texture, the pixel is further away from the light's position 
//                 and we know it lies within a shadow. On the other had, if  
//                 the pixel's Z value is less than the depth texture, we know 
//                 that the current pixel is closer to the light than the 
//                 object or shadow caster used to create the depth texture and 
//                 couldn't possible be in a shadowed region.
//                 
//                 Of course, while the concept sounds easy enough, there is 
//                 one sticking point that can make things less that intuitive: 
//                 The pixels, which are being compared, were rendered from the 
//                 eye's point-of-view and the depth map was created from the 
//                 light's point of view, so for the comparison to be valid we 
//                 need to do determine each pixel's XYZ position relative to 
//                 the light. This sample uses eye-linear texture coordinate 
//                 generation to find each pixel's light position.
//                 
//                 Please note that this sample uses some nVIDIA specific 
//                 features and may not run properly on ATI cards.
//
//   Control Keys: Up    - Light moves up
//                 Down  - Light moves down
//                 Left  - Light moves left
//                 Right - Light moves right 
//
//                 Left Mouse Button  - Spin the view
//                 Right Mouse Button - Spin the teapot
//
//                 F1 - Render depth texture (a.k.a. shadow map)
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "geometry.h"
#include "resource.h"

//-----------------------------------------------------------------------------
// FUNCTION POINTERS FOR OPENGL EXTENSIONS
//-----------------------------------------------------------------------------

// For convenience, this project ships with its own "glext.h" extension header 
// file. If you have trouble running this sample, it may be that this "glext.h" 
// file is defining something that your hardware doesn’t actually support. 
// Try recompiling the sample using your own local, vendor-specific "glext.h" 
// header file. The same applies for "wglext.h".

#include "glext.h"      // Sample's header file
//#include <GL/glext.h> // Your local header file

#include "wglext.h"      // Sample's header file
//#include <GL/wglext.h> // Your local header file

// WGL_ARB_extensions_string
PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB = NULL;

// WGL_ARB_pbuffer
PFNWGLCREATEPBUFFERARBPROC       wglCreatePbufferARB       = NULL;
PFNWGLGETPBUFFERDCARBPROC        wglGetPbufferDCARB        = NULL;
PFNWGLRELEASEPBUFFERDCARBPROC    wglReleasePbufferDCARB    = NULL;
PFNWGLDESTROYPBUFFERARBPROC      wglDestroyPbufferARB      = NULL;

// WGL_ARB_pixel_format
PFNWGLCHOOSEPIXELFORMATARBPROC   wglChoosePixelFormatARB   = NULL;

// WGL_ARB_render_texture
PFNWGLBINDTEXIMAGEARBPROC        wglBindTexImageARB        = NULL;
PFNWGLRELEASETEXIMAGEARBPROC     wglReleaseTexImageARB     = NULL;

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND   g_hWnd = NULL;
HDC	   g_hDC  = NULL;
HGLRC  g_hRC  = NULL;
GLuint g_depthTexture = -1;

float g_fSpinX_L =  0.0f;
float g_fSpinY_L = -10.0f;
float g_fSpinX_R =  0.0f;
float g_fSpinY_R =  0.0f;

float g_lightsLookAtMatrix[16];
float g_lightPosition[] = { 2.0f, 6.5f, 0.0f, 1.0f };

bool g_bRenderDepthTexture = false;

// This little struct will help to organize our p-buffer's data
struct PBUFFER
{
    HPBUFFERARB hPBuffer; // Handle to a p-buffer.
    HDC         hDC;      // Handle to a device context.
    HGLRC       hRC;      // Handle to a GL rendering context.
    int         nWidth;   // Width of the p-buffer
    int         nHeight;  // Height of the p-buffer
};

PBUFFER g_pbuffer;

const int PBUFFER_WIDTH  = 256;
const int PBUFFER_HEIGHT = 256;

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void init(void);
void shutDown(void);
void initExtensions(void);
void initPbuffer(void);
void render(void);
void renderScene(void);
void createDepthTexture(void);
void displayDepthTexture(void);

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
                             "OpenGL - Shadow Mapping",
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
                    
                case VK_F1:
                    g_bRenderDepthTexture = !g_bRenderDepthTexture;
				    break;

                case 38: // Up Arrow Key
					g_lightPosition[1] += 0.1f;
					break;

				case 40: // Down Arrow Key
					g_lightPosition[1] -= 0.1f;
					break;

				case 37: // Left Arrow Key
					g_lightPosition[0] -= 0.1f;
					break;

				case 39: // Right Arrow Key
					g_lightPosition[0] += 0.1f;
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
// Name: initExtensions()
// Desc: 
//-----------------------------------------------------------------------------
void initExtensions( void )
{
    //
    // wglGetExtensionsStringARB
    //

    wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");
    char *wgl_ext = NULL;
    
    if( wglGetExtensionsStringARB )
        wgl_ext = (char*)wglGetExtensionsStringARB( wglGetCurrentDC() );
    else
    {
        MessageBox(NULL,"Unable to get address for wglGetExtensionsStringARB!",
            "ERROR",MB_OK|MB_ICONEXCLAMATION);
        exit(-1);
    }

    //
    // WGL_ARB_pbuffer
    //

    if( strstr( wgl_ext, "WGL_ARB_pbuffer" ) == NULL )
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

        if( !wglCreatePbufferARB || !wglGetPbufferDCARB || 
            !wglReleasePbufferDCARB || !wglDestroyPbufferARB )
        {
            MessageBox(NULL,"One or more WGL_ARB_pbuffer functions were not found",
                "ERROR",MB_OK|MB_ICONEXCLAMATION);
            exit(-1);
        }
    }

    //
    // WGL_ARB_pixel_format
    //

    if( strstr( wgl_ext, "WGL_ARB_pixel_format" ) == NULL )
    {
        MessageBox(NULL,"WGL_ARB_pixel_format extension was not found",
            "ERROR",MB_OK|MB_ICONEXCLAMATION);
        return;
    }
    else
    {
        wglChoosePixelFormatARB  = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");

        if( !wglCreatePbufferARB || !wglGetPbufferDCARB )
        {
            MessageBox(NULL,"One or more WGL_ARB_pixel_format functions were not found",
                "ERROR",MB_OK|MB_ICONEXCLAMATION);
            exit(-1);
        }
    }

    //
    // WGL_ARB_render_texture
    //

    if( strstr( wgl_ext, "WGL_ARB_render_texture" ) == NULL )
    {
        MessageBox(NULL,"WGL_ARB_render_texture extension was not found",
            "ERROR",MB_OK|MB_ICONEXCLAMATION);
        exit(-1);
    }
    else
    {
        wglBindTexImageARB    = (PFNWGLBINDTEXIMAGEARBPROC)wglGetProcAddress("wglBindTexImageARB");
        wglReleaseTexImageARB = (PFNWGLRELEASETEXIMAGEARBPROC)wglGetProcAddress("wglReleaseTexImageARB");

        if( !wglBindTexImageARB || !wglReleaseTexImageARB )
        {
            MessageBox(NULL,"One or more WGL_ARB_render_texture functions were not found",
                "ERROR",MB_OK|MB_ICONEXCLAMATION);
            exit(-1);
        }
    }
}

//-----------------------------------------------------------------------------
// Name: initPbuffer()
// Desc: Create a p-buffer for the off-screen rendering of the depth texture
//-----------------------------------------------------------------------------
void initPbuffer( void )
{
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

WGL_BIND_TO_TEXTURE_DEPTH_NV, TRUE, // Ask for depth texture

        WGL_BIND_TO_TEXTURE_RGBA_ARB, TRUE, // P-buffer will be used as a texture
        WGL_DOUBLE_BUFFER_ARB, FALSE,       // We don't require double buffering
        0                                   // Zero terminates the list
    };

    unsigned int count = 0;
    int pixelFormat;

    if( !wglChoosePixelFormatARB( g_hDC, pf_attr, NULL, 1, &pixelFormat, &count ) )
    {
        MessageBox(NULL,"pbuffer creation error:  wglChoosePixelFormatARB() failed.",
			"ERROR",MB_OK|MB_ICONEXCLAMATION);
        exit(-1);
    }

    if( count <= 0 )
    {
        MessageBox(NULL,"pbuffer creation error:  Couldn't find a suitable pixel format.",
			"ERROR",MB_OK|MB_ICONEXCLAMATION);
        exit(-1);
    }

    //
    // Set some p-buffer attributes so that we can use this p-buffer as a
    // 2D RGBA texture target.
    //

    int pb_attr[] =
    {
        WGL_DEPTH_TEXTURE_FORMAT_NV, WGL_TEXTURE_DEPTH_COMPONENT_NV, // We need to render to a depth texture
        WGL_TEXTURE_FORMAT_ARB, WGL_TEXTURE_RGBA_ARB,                // Our p-buffer will have a texture format of RGBA
        WGL_TEXTURE_TARGET_ARB, WGL_TEXTURE_2D_ARB,                  // Of texture target will be GL_TEXTURE_2D
        0                                                            // Zero terminates the list
    };

    //
    // Create the p-buffer...
    //

    g_pbuffer.hPBuffer = wglCreatePbufferARB( g_hDC, pixelFormat, g_pbuffer.nWidth, g_pbuffer.nHeight, pb_attr );
    g_pbuffer.hDC      = wglGetPbufferDCARB( g_pbuffer.hPBuffer );
    g_pbuffer.hRC      = wglCreateContext( g_pbuffer.hDC );

    if( !g_pbuffer.hPBuffer )
    {
        MessageBox(NULL,"pbuffer creation error: wglCreatePbufferARB() failed!",
			"ERROR",MB_OK|MB_ICONEXCLAMATION);
        exit(-1);
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

	glClearColor( 0.35f, 0.53f, 0.7f, 1.0f );
    glEnable( GL_LIGHTING );
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_DEPTH_TEST );
    
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0f, 640.0f / 480.0f, 0.1f, 100.0f);

    //
	// Enable some dim, grey ambient lighting...
    //

	GLfloat ambient_lightModel[] = { 0.25f, 0.25f, 0.25f, 1.0f };
	glLightModelfv( GL_LIGHT_MODEL_AMBIENT, ambient_lightModel );

    //
	// Set up a point light source...
	//

    glEnable( GL_LIGHT0 );
    GLfloat diffuse_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat linearAttenuation_light[] = { 0.0f };
	glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuse_light );
	glLightfv( GL_LIGHT0, GL_LINEAR_ATTENUATION , linearAttenuation_light );

    //
    // Initialize the p-buffer now that we have a valid context
    // that we can use during the p-buffer creation process.
    //

    initExtensions();

    initPbuffer();

    //
    // Initialize some graphics state for the p-buffer's rendering context.
    //
    
    if( wglMakeCurrent( g_pbuffer.hDC, g_pbuffer.hRC) == FALSE )
    {
		MessageBox(NULL,"Could not make the p-buffer's context current!",
			"ERROR",MB_OK|MB_ICONEXCLAMATION);
        exit(-1);
    }

    glEnable( GL_LIGHTING );
    glEnable( GL_LIGHT0 );
    glEnable( GL_DEPTH_TEST );

    glLightModelfv( GL_LIGHT_MODEL_AMBIENT, ambient_lightModel );

    // Initialize some state for the window's rendering context.
    if( wglMakeCurrent( g_hDC, g_hRC ) == FALSE )
    {
		MessageBox(NULL,"Could not make the window's context current!",
			"ERROR",MB_OK|MB_ICONEXCLAMATION);
        exit(-1);
    }

    //
    // Create the depth texture
    //

    glGenTextures( 1,&g_depthTexture );
    glBindTexture( GL_TEXTURE_2D, g_depthTexture );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_SGIX, GL_TRUE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_OPERATOR_SGIX, GL_TEXTURE_LEQUAL_R_SGIX );
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )	
{
    glDeleteTextures( 1, &g_depthTexture );

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
// Name: displayDepthTexture()
// Desc: For debugging purposes
//-----------------------------------------------------------------------------
void displayDepthTexture( void )
{
    glDisable( GL_LIGHTING );

    glViewport( 0, 0, 640, 480 );

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluOrtho2D( -1.0, 1.0, -1.0, 1.0 );
    
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    // reset our texture matrix
    glMatrixMode( GL_TEXTURE );
    glLoadIdentity();

    // A depth texture can be treated as a luminance texture
    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, g_depthTexture );
    // Disable the shadow hardware
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_SGIX, GL_FALSE );

    if( wglBindTexImageARB( g_pbuffer.hPBuffer, WGL_DEPTH_COMPONENT_NV ) == FALSE )
    {
		MessageBox(NULL,"Could not bind p-buffer to render texture!",
			"ERROR",MB_OK|MB_ICONEXCLAMATION);
		exit(-1);
	}

    glBegin( GL_QUADS );
    {
        glTexCoord2f( 0.0f, 0.0f );
        glVertex3f( -1.0f, -1.0f, 0.0f );
    
        glTexCoord2f( 0.0f, 1.0f );
        glVertex3f( -1.0f, 1.0f, 0.0f );

        glTexCoord2f( 1.0f, 1.0f );
        glVertex3f( 1.0f, 1.0f, 0.0f );

        glTexCoord2f( 1.0f, 0.0f );
        glVertex3f( 1.0f, -1.0f, 0.0f );
    }
    glEnd();

    glEnable( GL_LIGHTING );
    glDisable( GL_TEXTURE_2D );

    // Enable the shadow mapping hardware
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_SGIX, GL_TRUE );

    if( wglReleaseTexImageARB( g_pbuffer.hPBuffer, WGL_DEPTH_COMPONENT_NV ) == FALSE )
    {
		MessageBox(NULL,"Could not release p-buffer from render texture!",
			"ERROR",MB_OK|MB_ICONEXCLAMATION);
		exit(-1);
	}
}

//-----------------------------------------------------------------------------
// Name: createDepthTexture()
// Desc: 
//-----------------------------------------------------------------------------
void createDepthTexture( void )
{
    // Make the pbuffer rendering context current
    if( wglMakeCurrent( g_pbuffer.hDC, g_pbuffer.hRC) == FALSE )
    {
		MessageBox(NULL,"Could not make the p-buffer's context current!",
			"ERROR",MB_OK|MB_ICONEXCLAMATION);
        exit(-1);
    }

    glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );

    glPolygonOffset( 2.0f, 4.0f );
    glEnable( GL_POLYGON_OFFSET_FILL );

    // Place light at the origin
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float originPosition[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glLightfv( GL_LIGHT0, GL_POSITION, originPosition );

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective( 45.0f, 640.0f / 480.0f, 0.1f, 100.0f);
    glViewport( 0, 0, PBUFFER_WIDTH, PBUFFER_HEIGHT );

    glMatrixMode( GL_MODELVIEW );
    glMultMatrixf( g_lightsLookAtMatrix);

    renderScene();

    glDisable( GL_POLYGON_OFFSET_FILL );

    // Make the window rendering context current
    if( wglMakeCurrent( g_hDC, g_hRC ) == FALSE )
    {
		MessageBox(NULL,"Could not make the window's context current!",
			"ERROR",MB_OK|MB_ICONEXCLAMATION);
        exit(-1);
    }
}

//-----------------------------------------------------------------------------
// Name: renderScene()
// Desc: 
//-----------------------------------------------------------------------------
void renderScene( void )
{
    glMatrixMode( GL_MODELVIEW );

    //
    // Render teapot...
    //

    glPushMatrix();
    {
        // Teapot's position & orientation
        glTranslatef( 0.0f, 2.5f, 0.0f );
        glRotatef( -g_fSpinY_R, 1.0f, 0.0f, 0.0f );
        glRotatef( -g_fSpinX_R, 0.0f, 1.0f, 0.0f );

        glColor3f( 1.0f, 1.0f ,1.0f );
        renderSolidTeapot( 1.0);
    }
    glPopMatrix();

    //*
    //
    // Render floor as a single quad...
    //

    glPushMatrix();
    {
        glBegin( GL_QUADS );
        {
            glNormal3f( 0.0f, 1.0f,  0.0f );
            glVertex3f(-5.0f, 0.0f, -5.0f );
            glVertex3f(-5.0f, 0.0f,  5.0f );
            glVertex3f( 5.0f, 0.0f,  5.0f );
            glVertex3f( 5.0f, 0.0f, -5.0f );
        }
        glEnd();
    }
    glPopMatrix();
    //*/

    /*
    //
    // Render floor as a tessellation of many quads...
    //

    float x = 0.0f;
	float z = 0.0f;

	for( int i = 0; i < 5; ++i )
    {
		for( int j = 0; j < 5; ++j )
		{
            glPushMatrix();
            {
			    glTranslatef( x - 4.0f, 0.0f, z - 4.0f );

                glBegin( GL_QUADS );
                {
                    glNormal3f( 0.0f, 1.0f,  0.0f );

                    glVertex3f(-1.0f, 0.0f, -1.0f );
                    glVertex3f( 1.0f, 0.0f, -1.0f );
                    glVertex3f( 1.0f, 0.0f,  1.0f );
                    glVertex3f(-1.0f, 0.0f,  1.0f );
                }
                glEnd();
            }
			glPopMatrix();

			x += 2.0f;
		}
		x  = 0.0f;
		z += 2.0f;
	}
    //*/
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: 
//-----------------------------------------------------------------------------
void render( void )
{
    //
    // Create a look-at matrix for our light and cache it for later use...
    //

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    gluLookAt( g_lightPosition[0], g_lightPosition[1], g_lightPosition[2], // Look from the light's position
               0.0f, 2.5f, 0.0f,   // Towards the teapot's position
               0.0f, 1.0f, 0.0f ); // Up vector

    // Get the model-view matrix
    glGetFloatv( GL_MODELVIEW_MATRIX, g_lightsLookAtMatrix );

    //
    // Create the depth texture...
    //

    createDepthTexture();

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    //
    // Place the view
    //

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glTranslatef( 0.0f, -2.0f, -15.0f );
    glRotatef( -g_fSpinY_L, 1.0f, 0.0f, 0.0f );
    glRotatef( -g_fSpinX_L, 0.0f, 1.0f, 0.0f );

    //
    // Render the light's position as a sphere...
    //

    glDisable( GL_LIGHTING );

    glPushMatrix();
    {
        // Place the light...
        glLightfv( GL_LIGHT0, GL_POSITION, g_lightPosition );

        // Place a sphere to represent the light
        glTranslatef( g_lightPosition[0], g_lightPosition[1], g_lightPosition[2] );

        glColor3f(1.0f, 1.0f, 0.5f);
        renderSolidSphere( 0.1, 8, 8 );
    }
    glPopMatrix();

    glEnable( GL_LIGHTING );

    //
    // Set up OpenGL's state machine for a depth comparison using the 
    // depth texture...
    //

    glEnable( GL_LIGHTING );
    
    float x[] = { 1.0f, 0.0f, 0.0f, 0.0f };
    float y[] = { 0.0f, 1.0f, 0.0f, 0.0f };
    float z[] = { 0.0f, 0.0f, 1.0f, 0.0f };
    float w[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glTexGenfv( GL_S, GL_EYE_PLANE, x );
    glTexGenfv( GL_T, GL_EYE_PLANE, y );
    glTexGenfv( GL_R, GL_EYE_PLANE, z );
    glTexGenfv( GL_Q, GL_EYE_PLANE, w );

    glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR );
    glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR );
    glTexGeni( GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR );
    glTexGeni( GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR );

    glEnable( GL_TEXTURE_GEN_S );
    glEnable( GL_TEXTURE_GEN_T );
    glEnable( GL_TEXTURE_GEN_R );
    glEnable( GL_TEXTURE_GEN_Q );

    // Set up the depth texture projection
    glMatrixMode( GL_TEXTURE );
    glLoadIdentity();
    glTranslatef( 0.5f, 0.5f, 0.5f );                      // Offset
    glScalef( 0.5f, 0.5f, 0.5f );                          // Bias
    gluPerspective( 45.0f, 640.0f / 480.0f, 0.1f, 100.0f); // light frustum
    glMultMatrixf( g_lightsLookAtMatrix );                 // Light matrix

    //glMatrixMode( GL_PROJECTION );
    //glLoadIdentity();
    //gluPerspective( 45.0f, 640.0f / 480.0f, 0.1f, 100.0f);

    //
    // Bind the depth texture so we can use it as the shadow map...
    //

    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, g_depthTexture );
    
    if( wglBindTexImageARB( g_pbuffer.hPBuffer, WGL_DEPTH_COMPONENT_NV ) == FALSE )
    {
		MessageBox(NULL,"Could not bind p-buffer to render texture!",
			"ERROR",MB_OK|MB_ICONEXCLAMATION);
		exit(-1);
	}

    renderScene();

    if( wglReleaseTexImageARB( g_pbuffer.hPBuffer, WGL_DEPTH_COMPONENT_NV ) == FALSE )
    {
		MessageBox(NULL,"Could not release p-buffer from render texture!",
			"ERROR",MB_OK|MB_ICONEXCLAMATION);
		exit(-1);
	}

    //
    // Reset some of the states for the next go-around!
    //

    glDisable( GL_TEXTURE_2D );
    glDisable( GL_TEXTURE_GEN_S );
    glDisable( GL_TEXTURE_GEN_T );
    glDisable( GL_TEXTURE_GEN_R );
    glDisable( GL_TEXTURE_GEN_Q );

    if( g_bRenderDepthTexture == true )
        displayDepthTexture(); // For debugging...

    SwapBuffers( g_hDC );
}
