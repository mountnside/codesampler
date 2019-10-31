//-----------------------------------------------------------------------------
//           Name: ogl_planar_shadow.cpp
//         Author: Kevin Harris
//  Last Modified: 02/01/05
//    Description: This sample demonstrates how to create planar shadows under
//                 OpenGL.
//
//                 Planar shadows are created by building a special projection 
//                 matrix which flattens an object's geometry into a plane when 
//                 rendered.
//                 
//                 If the plane, which the geometry is flattened into, matches 
//                 up with another planar surface like a floor or a wall, the 
//                 flattened geometry can be made to resemble a shadow on that 
//                 surface.
//
//   Control Keys: Up    - Light moves up
//                 Down  - Light moves down
//                 Left  - Light moves left
//                 Right - Light moves right
//
//                 Left Mouse Button  - Spin the view
//                 Right Mouse Button - Spin the teapot
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
HWND   g_hWnd = NULL;
HDC    g_hDC  = NULL;
HGLRC  g_hRC  = NULL;

// Spin control for view
float  g_fSpinX_L =   0.0f;
float  g_fSpinY_L = -10.0f;

// Spin control for teapot
float  g_fSpinX_R =   0.0f;
float  g_fSpinY_R =   0.0f;

float g_shadowMatrix[16];
float g_lightPosition[] = { 2.0f, 6.0f, 0.0f, 1.0f }; // World position of light source

struct Vertex
{
    float nx, ny, nz;
    float x, y, z;
};

Vertex g_floorQuad[] =
{
    { 0.0f, 1.0f,  0.0f,  -5.0f, 0.0f, -5.0f },
    { 0.0f, 1.0f,  0.0f,  -5.0f, 0.0f,  5.0f },
    { 0.0f, 1.0f,  0.0f,   5.0f, 0.0f,  5.0f },
    { 0.0f, 1.0f,  0.0f,   5.0f, 0.0f, -5.0f },
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
void buildShadowMatrix(float fMatrix[16],float fLightPos[4],float fPlane[4]);

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
                             "OpenGL - Planar Shadows",
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
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective( 45.0f, 640.0f / 480.0f, 0.1f, 100.0f);

    // Enable a single OpenGL light.
    float lightAmbient[] = {0.2f, 0.2f, 0.2f, 1.0f};
    float lightDiffuse[] = {1.0, 1.0, 1.0, 1.0}; 
    float lightSpecular[] = {1.0, 1.0, 1.0, 1.0};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
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
// Name: buildShadowMatrix()
// Desc: 
//-----------------------------------------------------------------------------
void buildShadowMatrix( float fMatrix[16], float fLightPos[4], float fPlane[4] )
{
    float dotp;

    // Calculate the dot-product between the plane and the light's position
    dotp = fPlane[0] * fLightPos[0] + 
           fPlane[1] * fLightPos[1] + 
           fPlane[1] * fLightPos[2] + 
           fPlane[3] * fLightPos[3];

    // First column
    fMatrix[0]  = dotp - fLightPos[0] * fPlane[0];
    fMatrix[4]  = 0.0f - fLightPos[0] * fPlane[1];
    fMatrix[8]  = 0.0f - fLightPos[0] * fPlane[2];
    fMatrix[12] = 0.0f - fLightPos[0] * fPlane[3];

    // Second column
    fMatrix[1]  = 0.0f - fLightPos[1] * fPlane[0];
    fMatrix[5]  = dotp - fLightPos[1] * fPlane[1];
    fMatrix[9]  = 0.0f - fLightPos[1] * fPlane[2];
    fMatrix[13] = 0.0f - fLightPos[1] * fPlane[3];

    // Third column
    fMatrix[2]  = 0.0f - fLightPos[2] * fPlane[0];
    fMatrix[6]  = 0.0f - fLightPos[2] * fPlane[1];
    fMatrix[10] = dotp - fLightPos[2] * fPlane[2];
    fMatrix[14] = 0.0f - fLightPos[2] * fPlane[3];

    // Fourth column
    fMatrix[3]  = 0.0f - fLightPos[3] * fPlane[0];
    fMatrix[7]  = 0.0f - fLightPos[3] * fPlane[1];
    fMatrix[11] = 0.0f - fLightPos[3] * fPlane[2];
    fMatrix[15] = dotp - fLightPos[3] * fPlane[3];
}

//-----------------------------------------------------------------------------
// Name: findPlane()
// Desc: find the plane equation given 3 points
//-----------------------------------------------------------------------------
void findPlane( GLfloat plane[4], GLfloat v0[3], GLfloat v1[3], GLfloat v2[3] )
{
    GLfloat vec0[3], vec1[3];

    // Need 2 vectors to find cross product
    vec0[0] = v1[0] - v0[0];
    vec0[1] = v1[1] - v0[1];
    vec0[2] = v1[2] - v0[2];

    vec1[0] = v2[0] - v0[0];
    vec1[1] = v2[1] - v0[1];
    vec1[2] = v2[2] - v0[2];

    // Find cross product to get A, B, and C of plane equation
    plane[0] =   vec0[1] * vec1[2] - vec0[2] * vec1[1];
    plane[1] = -(vec0[0] * vec1[2] - vec0[2] * vec1[0]);
    plane[2] =   vec0[0] * vec1[1] - vec0[1] * vec1[0];

    plane[3] = -(plane[0] * v0[0] + plane[1] * v0[1] + plane[2] * v0[2]);
}

//-----------------------------------------------------------------------------
// Name: renderFloor()
// Desc: 
//-----------------------------------------------------------------------------
void renderFloor()
{
    glColor3f( 1.0f, 1.0f, 1.0f );
    glInterleavedArrays( GL_N3F_V3F, 0, g_floorQuad );
    glDrawArrays( GL_QUADS, 0, 4 );
}

//*
//-----------------------------------------------------------------------------
// Name: render()
// Desc: 
//-----------------------------------------------------------------------------
void render( void )
{
    //
    // Define the plane of the planar surface that we want to cast a shadow on...
    //

    GLfloat shadowPlane[4];
    GLfloat v0[3], v1[3], v2[3];

    // To define a plane that matches the floor, we need to 3 vertices from it
    v0[0] = g_floorQuad[0].x;
    v0[1] = g_floorQuad[0].y;
    v0[2] = g_floorQuad[0].z;

    v1[0] = g_floorQuad[1].x;
    v1[1] = g_floorQuad[1].y;
    v1[2] = g_floorQuad[1].z;

    v2[0] = g_floorQuad[2].x;
    v2[1] = g_floorQuad[2].y;
    v2[2] = g_floorQuad[2].z;

    findPlane( shadowPlane, v0, v1, v2 );

    //
    // Build a shadow matrix using the light's current position and the plane
    //

    buildShadowMatrix( g_shadowMatrix, g_lightPosition, shadowPlane );
    
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
    // Render the floor...
    //

    renderFloor();

    //
    // Create a shadow by rendering the teapot using the shadow matrix.
    //

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    glColor3f(0.2f, 0.2f, 0.2f); // Shadow's color
    glPushMatrix();
    {
        glMultMatrixf((GLfloat *)g_shadowMatrix);

        // Teapot's position & orientation (needs to use the same transformations used to render the actual teapot)
        glTranslatef( 0.0f, 2.5f, 0.0f );
        glRotatef( -g_fSpinY_R, 1.0f, 0.0f, 0.0f );
        glRotatef( -g_fSpinX_R, 0.0f, 1.0f, 0.0f );
        renderSolidTeapot( 1.0 );
    }
    glPopMatrix();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

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
    // Render normal teapot
    //

    glPushMatrix();
    {
        // Teapot's position & orientation
        glTranslatef( 0.0f, 2.5f, 0.0f );
        glRotatef( -g_fSpinY_R, 1.0f, 0.0f, 0.0f );
        glRotatef( -g_fSpinX_R, 0.0f, 1.0f, 0.0f );
        renderSolidTeapot( 1.0 );
    }
    glPopMatrix();

    SwapBuffers( g_hDC );
}
//*/

/*
//-----------------------------------------------------------------------------
// Name: render()
// Desc: This version is more complicated as it uses the stencil buffer to 
//       trim the shadow to the floor's edge.
//-----------------------------------------------------------------------------
void render( void )
{
    //
    // Define the plane of the planar surface that we want to cast a shadow on...
    //

    GLfloat shadowPlane[4];
    GLfloat v0[3], v1[3], v2[3];

    // To define a plane that matches the floor, we need to 3 vertices from it
    v0[0] = g_floorQuad[0].x;
    v0[1] = g_floorQuad[0].y;
    v0[2] = g_floorQuad[0].z;

    v1[0] = g_floorQuad[1].x;
    v1[1] = g_floorQuad[1].y;
    v1[2] = g_floorQuad[1].z;

    v2[0] = g_floorQuad[2].x;
    v2[1] = g_floorQuad[2].y;
    v2[2] = g_floorQuad[2].z;

    findPlane( shadowPlane, v0, v1, v2 );

    //
    // Build a shadow matrix using the light's current position and the plane
    //

    buildShadowMatrix( g_shadowMatrix, g_lightPosition, shadowPlane );
    
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

    //
    // Place the view
    //

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glTranslatef( 0.0f, -2.0f, -15.0f );
    glRotatef( -g_fSpinY_L, 1.0f, 0.0f, 0.0f );
    glRotatef( -g_fSpinX_L, 0.0f, 1.0f, 0.0f );

    //
    // Render the floor to the stencil buffer so we can use it later to trim 
    // the shadow at the floor's edge...
    //

    glEnable( GL_STENCIL_TEST );
    glStencilFunc( GL_ALWAYS, 1, 0xFFFFFFFF );         // Write a 1 to the stencil buffer everywhere we are about to draw
    glStencilOp( GL_REPLACE, GL_REPLACE, GL_REPLACE ); // If a 1 is written to the stencil buffer - simply replace the current value stored there with the 1.

    glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE ); // Disable writing to the color buffer
    glDepthMask( GL_FALSE );                               // Disable writing to the depth buffer

    renderFloor();

    glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE ); // Re-enable writing to the color buffer
    glDepthMask( GL_TRUE );                            // Re-enable writing to the depth buffer 
    glDisable( GL_STENCIL_TEST );

    //
    // Render the floor...
    //
    
    renderFloor();

    //
    // Render the shadow...
    //

    // Use our stencil to keep the shadow from running off the floor.
    glEnable( GL_STENCIL_TEST );
    glStencilFunc( GL_EQUAL, 1, 0xFFFFFFFF ); // Only write to areas where the stencil buffer has a 1.
    glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP ); // Don't modify the contents of the stencil buffer

    glDisable( GL_LIGHTING );
    glDisable( GL_DEPTH_TEST );
    glEnable( GL_BLEND );

    glPushMatrix();
    {
        // Load the teapot's shadow matrix
        glMultMatrixf( g_shadowMatrix );

        glColor3f(0.2f, 0.2f, 0.2f); // shadow color
        glPushMatrix();
        {
            // Teapot's position & orientation (needs to use the same transformations used to render the actual teapot)
            glTranslatef( 0.0f, 2.5f, 0.0f );
            glRotatef( -g_fSpinY_R, 1.0f, 0.0f, 0.0f );
            glRotatef( -g_fSpinX_R, 0.0f, 1.0f, 0.0f );
            renderSolidTeapot( 1.0 );
        }
        glPopMatrix();
    }
    glPopMatrix();

    glEnable( GL_DEPTH_TEST );
    glDisable( GL_BLEND );
    glEnable( GL_LIGHTING );
    glDisable( GL_STENCIL_TEST );

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
    // Render the teapot...
    //

    glPushMatrix();
    {
        // Teapot's position & orientation
        glTranslatef( 0.0f, 2.5f, 0.0f );
        glRotatef( -g_fSpinY_R, 1.0f, 0.0f, 0.0f );
        glRotatef( -g_fSpinX_R, 0.0f, 1.0f, 0.0f );
        renderSolidTeapot( 1.0 );
    }
    glPopMatrix();

    SwapBuffers( g_hDC );
}
//*/
