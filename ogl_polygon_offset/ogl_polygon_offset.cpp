//-----------------------------------------------------------------------------
//           Name: ogl_polygon_offset.cpp
//         Author: Kevin Harris
//  Last Modified: 02/01/05
//    Description: This sample demonstrates how to eliminate z-fighting when
//                 rendering polygons directly on top of other polygons.
//
//   Control Keys: Left Mouse Button - Spin the view
//                 F1 - Increase Offset Factor
//                 F2 - Decrease Offset Factor
//                 F3 - Increase Offset Unit
//                 F4 - Decrease Offset Unit
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glaux.h>
#include "bitmap_fonts.h"
#include "resource.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND   g_hWnd          = NULL;
HDC    g_hDC           = NULL;
HGLRC  g_hRC           = NULL;
int    g_nWindowWidth  = 640;
int    g_nWindowHeight = 480;
GLuint g_textureID     = -1;

float g_OffsetFactor = -1.0f;
float g_OffsetUnit   = -2.0f;

float g_fSpinX = 0.0f;
float g_fSpinY = 0.0f;

struct Vertex
{
    float tu, tv;
    float x, y, z;
};

Vertex g_quadVertices[] =
{
    { 0.0f,0.0f, -1.0f,-1.0f, 0.0f },
    { 1.0f,0.0f,  1.0f,-1.0f, 0.0f },
    { 1.0f,1.0f,  1.0f, 1.0f, 0.0f },
    { 0.0f,1.0f, -1.0f, 1.0f, 0.0f }
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
                             "OpenGL - Polygon Offset",
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
                    g_OffsetFactor += 0.05f;
                    break;

                case VK_F2:
                    g_OffsetFactor -= 0.05f;
                    break;

                case VK_F3:
                    g_OffsetUnit += 0.005f;
                    break;

                case VK_F4:
                    g_OffsetUnit -= 0.005f;
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
void loadTexture(void)	
{
    AUX_RGBImageRec *pTextureImage = auxDIBImageLoad( ".\\checker.bmp" );

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
    glEnable(GL_DEPTH_TEST);

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective( 45.0, (GLdouble)g_nWindowWidth / g_nWindowHeight, 0.1, 100.0 );
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

    //
    // Set up the model-view matrix for spinning the quads about and render the 
    // larger textured quad first.
    //

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glTranslatef( 0.0f, 0.0f, -4.0f );
    glRotatef( -g_fSpinY, 1.0f, 0.0f, 0.0f );
    glRotatef( -g_fSpinX, 0.0f, 1.0f, 0.0f );

    glBindTexture( GL_TEXTURE_2D, g_textureID );
    glInterleavedArrays( GL_T2F_V3F, 0, g_quadVertices );
    glDrawArrays( GL_QUADS, 0, 4 );

    //
    // Now, for the second quad, scale it down a bit but use the same  
    // translation and rotation matrices so we render it right on top of the 
    // first quad. This will, of course, cause some z-fighting issues, which
    // we will fix via glPolygonOffset and GL_POLYGON_OFFSET_FILL.
    //

    // Use polygon offset
    glEnable( GL_POLYGON_OFFSET_FILL ); // This is the one we need...
    //glEnable( GL_POLYGON_OFFSET_LINE );
    //glEnable( GL_POLYGON_OFFSET_POINT );
    glPolygonOffset( g_OffsetFactor, g_OffsetUnit );

    glScalef( 0.5f, 0.5f, 0.0f );

    glDisable( GL_TEXTURE_2D );
    glInterleavedArrays( GL_T2F_V3F, 0, g_quadVertices );
    glDrawArrays( GL_QUADS, 0, 4 );
    glEnable( GL_TEXTURE_2D );

    // Turn off polygon offset
    glPolygonOffset( 0.0f, 0.0f );
    glDisable( GL_POLYGON_OFFSET_FILL );
    //glDisable( GL_POLYGON_OFFSET_LINE );
    //glDisable( GL_POLYGON_OFFSET_POINT );

    //
    // Output the current settings...
    //

    static char strOffsetFactor[255];
    static char strOffsetunit[255];

    sprintf( strOffsetFactor, "Offset Factor = %1.2f    (Change: F1 and F2)", g_OffsetFactor );
    sprintf( strOffsetunit, "Offset Unit = %1.2f    (Change: F3 and F4)", g_OffsetUnit );

    beginRenderText( g_nWindowWidth, g_nWindowHeight );
    {
        glColor3f( 1.0f, 1.0f, 1.0f );
        renderText( 5, 15, BITMAP_FONT_TYPE_HELVETICA_12, strOffsetFactor );
        renderText( 5, 30, BITMAP_FONT_TYPE_HELVETICA_12, strOffsetunit );
    }
    endRenderText();

    SwapBuffers( g_hDC );
}
