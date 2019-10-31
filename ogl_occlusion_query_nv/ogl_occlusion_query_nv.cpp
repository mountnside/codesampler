//-----------------------------------------------------------------------------
//           Name: ogl_occlusion_query_nv.cpp
//         Author: Kevin Harris
//  Last Modified: 02/01/05
//    Description: This sample demonstrates how to use nVIDIA's new OpenGL 
//                 extension, NV_occlusion_query.
//
//   Control Keys: Left Mouse Button - Spin the view
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "geometry.h"
#include "bitmap_fonts.h"
#include "resource.h"

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

PFNGLGENOCCLUSIONQUERIESNVPROC    glGenOcclusionQueriesNV    = NULL;
PFNGLDELETEOCCLUSIONQUERIESNVPROC glDeleteOcclusionQueriesNV = NULL;
PFNGLGETOCCLUSIONQUERYUIVNVPROC   glGetOcclusionQueryuivNV   = NULL;
PFNGLBEGINOCCLUSIONQUERYNVPROC    glBeginOcclusionQueryNV    = NULL;
PFNGLENDOCCLUSIONQUERYNVPROC      glEndOcclusionQueryNV      = NULL;

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND   g_hWnd          = NULL;
HDC    g_hDC           = NULL;
HGLRC  g_hRC           = NULL;
int    g_nWindowWidth  = 640;
int    g_nWindowHeight = 480;
GLuint g_planeQuery    = -1;
GLuint g_sphereQuery   = -1;

float g_fSpinX =  0.0f;
float g_fSpinY = 90.0f;

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void loadTexture(void);
void init(void);
void renderScene_toInitDepthBuffer(void);
void renderScene_toQuery(void);
void render(void);
void shutDown(void);

//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: The application's entry point
//-----------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE hInstance,
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
    winClass.hIcon         = LoadIcon(hInstance, (LPCTSTR)IDI_OPENGL_ICON);
    winClass.hIconSm       = LoadIcon(hInstance, (LPCTSTR)IDI_OPENGL_ICON);
    winClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    winClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    winClass.lpszMenuName  = NULL;
    winClass.cbClsExtra    = 0;
    winClass.cbWndExtra    = 0;
    
    if( !RegisterClassEx(&winClass) )
        return E_FAIL;

    g_hWnd = CreateWindowEx( NULL, "MY_WINDOWS_CLASS", 
                             "OpenGL - NV_occlusion_query Demo",
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
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective( 45.0, (GLdouble)g_nWindowWidth / g_nWindowHeight, 0.1, 100.0);

    //
	// If the required extensions are present, get the addresses of their 
	// functions that we wish to use...
	//

	char *ext = (char*)glGetString( GL_EXTENSIONS );

	if( strstr( ext, "GL_NV_occlusion_query" ) == NULL )
	{
		MessageBox(NULL,"GL_NV_occlusion_query extension was not found",
			"ERROR",MB_OK|MB_ICONEXCLAMATION);
		return;
	}
	else
	{
        glGenOcclusionQueriesNV    = (PFNGLGENOCCLUSIONQUERIESNVPROC)wglGetProcAddress("glGenOcclusionQueriesNV");
        glDeleteOcclusionQueriesNV = (PFNGLDELETEOCCLUSIONQUERIESNVPROC)wglGetProcAddress("glDeleteOcclusionQueriesNV");
        glGetOcclusionQueryuivNV   = (PFNGLGETOCCLUSIONQUERYUIVNVPROC)wglGetProcAddress("glGetOcclusionQueryuivNV");
        glBeginOcclusionQueryNV    = (PFNGLBEGINOCCLUSIONQUERYNVPROC)wglGetProcAddress("glBeginOcclusionQueryNV");
        glEndOcclusionQueryNV      = (PFNGLENDOCCLUSIONQUERYNVPROC)wglGetProcAddress("glEndOcclusionQueryNV");

		if( !glGenOcclusionQueriesNV  || !glDeleteOcclusionQueriesNV || 
            !glGetOcclusionQueryuivNV || !glBeginOcclusionQueryNV    || 
            !glEndOcclusionQueryNV )
		{
			MessageBox(NULL,"One or more GL_NV_occlusion_query functions were not found",
				"ERROR",MB_OK|MB_ICONEXCLAMATION);
			return;
		}
	}

    //
    // Create query objects for our sphere and plane
    //

    glGenOcclusionQueriesNV( 1, &g_sphereQuery );
    glGenOcclusionQueriesNV( 1, &g_planeQuery );
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )   
{
    //
    // Make sure to clean up after our queries...
    //

    glDeleteOcclusionQueriesNV( 1, &g_sphereQuery );
    glDeleteOcclusionQueriesNV( 1, &g_planeQuery );

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
// Name: renderScene_toInitDepthBuffer()
// Desc: 
//-----------------------------------------------------------------------------
void renderScene_toInitDepthBuffer( void )
{
    //
    // Render the plane first...
    //

    glPushMatrix();
    {
        glTranslatef( 0.0f, -0.025f, 0.0f);
        glScalef( 1.0f, 0.05f, 1.0f );
        glColor3f( 1.0f, 1.0f, 0.0f );
        renderSolidCube( 0.435f );
    }
    glPopMatrix();

    //
    // Render the sphere second...
    //

    glPushMatrix();
    {
        glTranslatef( 0.0f, 0.25f, 0.0f );
        glColor3f( 1.0f, 0.0f, 0.0f );
        renderSolidSphere( 0.25f, 20, 20 );
    }
    glPopMatrix();
}

//-----------------------------------------------------------------------------
// Name: renderScene_toQuery()
// Desc: 
//-----------------------------------------------------------------------------
void renderScene_toQuery( void )
{
    //
    // Render the plane first and wrap it with an occlusion query
    //

    glPushMatrix();
    {
        glTranslatef( 0.0f, -0.025f, 0.0f);
        glScalef( 1.0f, 0.05f, 1.0f );

        glBeginOcclusionQueryNV( g_planeQuery );
        {
            glColor3f( 1.0f, 1.0f, 0.0f );
            renderSolidCube( 0.435f );
        }
        glEndOcclusionQueryNV();
    }
    glPopMatrix();

    //
    // Render the sphere second and wrap it with an occlusion query
    //

    glPushMatrix();
    {
        glTranslatef( 0.0f, 0.25f, 0.0f );

        glBeginOcclusionQueryNV( g_sphereQuery );
        {
            glColor3f( 1.0f, 0.0f, 0.0f );
            renderSolidSphere( 0.25f, 20, 20 );
        }
        glEndOcclusionQueryNV();
    }
    glPopMatrix();
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
    glTranslatef( 0.0f, 0.0f, -1.5f );
    glRotatef( -g_fSpinY, 1.0f, 0.0f, 0.0f );
    glRotatef( -g_fSpinX, 0.0f, 1.0f, 0.0f );

    //
    // The first time we render the scene is to initialize the depth buffer. 
    // If we don't do this, an object, which is rendered first, may generate a 
    // pixel count which is greater than 0 even when that object is later 
    // occluded completely by another object, which is closer to the view point.
    //
    // You can actually skip this step if you know for certain that you'll
    // be rendering and querying your scene's objects in back-to-front order.
    //

    renderScene_toInitDepthBuffer();

    //
    // The second time is for getting accurate visible fragment counts
    //

    renderScene_toQuery();

    //
    // Now, we collect the fragment counts from our two 3D objects to see  
    // whether or not either of them would have contributed anything to the  
    // frame buffer if they were rendered.
    //

    GLuint planeFragmentCount;
    GLuint sphereFragmentCount;
    char planeString[50];
    char sphereString[50];

    glGetOcclusionQueryuivNV( g_planeQuery, GL_PIXEL_COUNT_NV, &planeFragmentCount );
    glGetOcclusionQueryuivNV( g_sphereQuery, GL_PIXEL_COUNT_NV, &sphereFragmentCount );

	sprintf( planeString, "Plane Fragments    = %d", planeFragmentCount );
    sprintf( sphereString, "Sphere Fragments = %d", sphereFragmentCount );

    beginRenderText( g_nWindowWidth, g_nWindowHeight );
    {
        glColor3f( 1.0f, 1.0f, 1.0f );
        renderText( 5, 15, BITMAP_FONT_TYPE_HELVETICA_12, planeString );
        renderText( 5, 30, BITMAP_FONT_TYPE_HELVETICA_12, sphereString );
    }
    endRenderText();

    SwapBuffers( g_hDC );
}
