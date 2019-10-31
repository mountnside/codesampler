//-----------------------------------------------------------------------------
//           Name: ogl_frustum_culling.cpp
//         Author: Kevin Harris
//  Last Modified: 04/28/05
//    Description: This sample demonstrates how to perform view frustum 
//                 culling, which is a very clever way of using bounding 
//                 volumes to skip the rending of objects which are outside 
//                 the current view. It's a fairly straight forward concept: 
//                 extract the clip planes that define the view's frustum from 
//                 the scene's model-view-projection matrix and use them to see 
//                 whether the bound spheres that represent our 3D objects 
//                 are in or out of the frustum. If the object is outside the 
//                 frustum, we do nothing and skip it. If it proves to be 
//                 inside or intersecting the frustum, we render it as usual.
//
//                 Note: This sample contains some code from another sample 
//                 by Mark Morley, which can be found here:
//
//                 http://www.markmorley.com/opengl/frustumculling.html
//
//   Control Keys: F1 - Toggle frustum culling
//
//                 Up         - View moves forward
//                 Down       - View moves backward
//                 Left       - View strafes left
//                 Right      - View strafes Right
//                 Left Mouse - Perform looking
//                 Mouse      - Look about the scene
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <sys/timeb.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "geometry.h"
#include "bitmap_fonts.h"
#include "resource.h"
#include "matrix4x4f.h"
#include "vector3f.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND  g_hWnd          = NULL;
HDC   g_hDC           = NULL;
HGLRC g_hRC           = NULL;
int   g_nWindowWidth  = 640;
int   g_nWindowHeight = 480;
int   g_displayList   = -1;

bool  g_bFirstRendering = true;
timeb g_lastTime;
float g_fTimeSinceLastReport = 0.0f;
int   g_nFrameCount = 0;
float g_fElpasedTime = 0.0f;

POINT  g_ptLastMousePosit;
POINT  g_ptCurrentMousePosit;
bool   g_bMousing = false;
float  g_fMoveSpeed = 25.0f;

vector3f g_vEye(0.0f, 0.0f, 0.0f);   // Eye Position
vector3f g_vLook(0.0f, 0.0f, -1.0f); // Look Vector
vector3f g_vUp(0.0f, 1.0f, 0.0f);    // Up Vector
vector3f g_vRight(1.0f, 0.0f, 0.0f); // Right Vector

bool g_bDoFrustumCulling = true;
float g_frustumPlanes[6][4];

const int NUM_OBJECTS = 1000;

struct ObjectData
{
    float x, y, z;
    float r, g, b;

    float fBoundingSphereRadius;
};

ObjectData g_objects[NUM_OBJECTS];

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void init(void);
void render(void);
void shutDown(void);
void getRealTimeUserInput(void);
void updateViewMatrix(void);
void calculateFrustumPlanes(void);

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
                             "OpenGL - Frustum Culling",
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
                    g_bDoFrustumCulling = !g_bDoFrustumCulling;
                    break;
            }
        }
        break;

        case WM_LBUTTONDOWN:
		{
			g_bMousing = true;
		}
		break;

		case WM_LBUTTONUP:
		{
			g_bMousing = false;
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
// Name: getRealTimeUserInput()
// Desc: 
//-----------------------------------------------------------------------------
void getRealTimeUserInput( void )
{
    //
    // Get mouse input...
    //

    POINT mousePosit;
    GetCursorPos( &mousePosit );
    ScreenToClient( g_hWnd, &mousePosit );

    g_ptCurrentMousePosit.x = mousePosit.x;
    g_ptCurrentMousePosit.y = mousePosit.y;

    matrix4x4f matRotation;

    if( g_bMousing )
    {
        int nXDiff = (g_ptCurrentMousePosit.x - g_ptLastMousePosit.x);
        int nYDiff = (g_ptCurrentMousePosit.y - g_ptLastMousePosit.y);

        if( nYDiff != 0 )
        {
            matRotation.rotate( -(float)nYDiff / 3.0f, g_vRight );
            matRotation.transformVector( &g_vLook );
            matRotation.transformVector( &g_vUp );
        }

        if( nXDiff != 0 )
        {
            matRotation.rotate( -(float)nXDiff / 3.0f, vector3f(0.0f, 1.0f, 0.0f) );
            matRotation.transformVector( &g_vLook );
            matRotation.transformVector( &g_vUp );
        }
    }

    g_ptLastMousePosit.x = g_ptCurrentMousePosit.x;
    g_ptLastMousePosit.y = g_ptCurrentMousePosit.y;

    //
    // Get keyboard input...
    //

    unsigned char keys[256];
    GetKeyboardState( keys );

    vector3f tmpLook  = g_vLook;
    vector3f tmpRight = g_vRight;

    // Up Arrow Key - View moves forward
    if( keys[VK_UP] & 0x80 )
        g_vEye -= tmpLook*-g_fMoveSpeed*g_fElpasedTime;

    // Down Arrow Key - View moves backward
    if( keys[VK_DOWN] & 0x80 )
        g_vEye += (tmpLook*-g_fMoveSpeed)*g_fElpasedTime;

    // Left Arrow Key - View side-steps or strafes to the left
    if( keys[VK_LEFT] & 0x80 )
        g_vEye -= (tmpRight*g_fMoveSpeed)*g_fElpasedTime;

    // Right Arrow Key - View side-steps or strafes to the right
    if( keys[VK_RIGHT] & 0x80 )
        g_vEye += (tmpRight*g_fMoveSpeed)*g_fElpasedTime;

    // Home Key - View elevates up
    if( keys[VK_HOME] & 0x80 )
        g_vEye.y += g_fMoveSpeed*g_fElpasedTime; 

    // End Key - View elevates down
    if( keys[VK_END] & 0x80 )
        g_vEye.y -= g_fMoveSpeed*g_fElpasedTime;
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

	// For lighting, pull our material colors from the vertex color...
	glEnable( GL_COLOR_MATERIAL );
	glColorMaterial( GL_FRONT, GL_AMBIENT_AND_DIFFUSE );

    glEnable(GL_LIGHT0);
    GLfloat ambient_light0[]= { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat diffuse_light0[]= { 1.0f, 1.0f, 1.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient_light0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse_light0);
    
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective( 45.0, (GLdouble)g_nWindowWidth / g_nWindowHeight, 0.1, 100.0 );
 
    //
    // Create a display list to hold our sphere and initialize an array of 
    // test objects.
    //

    g_displayList = glGenLists(1);

    glNewList( g_displayList, GL_COMPILE );
    {
        renderSolidSphere( 1.0, 32, 32 );
    }
    glEndList();

    for( int i = 0; i < NUM_OBJECTS; ++i )
    {
        // For each object, generate a random position...
        g_objects[i].x = -100.0f + rand() % 200;
        g_objects[i].y =  -10.0f + rand() % 20;
        g_objects[i].z = -100.0f + rand() % 200;

        // For each object, generate a random color...
        g_objects[i].r = 0.01f * (rand() % 100);
        g_objects[i].g = 0.01f * (rand() % 100);
        g_objects[i].b = 0.01f * (rand() % 100);

        //
        // Because we're actually rendering spheres as our test object, using
        // a bounding sphere as our bounding volume during frustum culling is
        // ideal. Of course, don't confuse the two. The spheres we render are
        // defined by actual triangles while the bounding sphere's are purely 
        // mathematical and are nothing more than a world position and a 
        // radius. They exist only for testing purposes.
        //
        // We use 1.0f for the radius since that's what we used when we created 
        // the display list above.
        //

        g_objects[i].fBoundingSphereRadius = 1.0f;
    }

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
// Name : updateViewMatrix() 
// Desc : Builds a view matrix suitable for OpenGL.
//
// Here's what the final view matrix should look like:
//
//  |  rx   ry   rz  -(r.e) |
//  |  ux   uy   uz  -(u.e) |
//  | -lx  -ly  -lz   (l.e) |
//  |   0    0    0     1   |
//
// Where r = Right vector
//       u = Up vector
//       l = Look vector
//       e = Eye position in world space
//       . = Dot-product operation
//
//-----------------------------------------------------------------------------
void updateViewMatrix( void )
{
    matrix4x4f view;
    view.identity();

    g_vLook.normalize();
 
    g_vRight = crossProduct(g_vLook, g_vUp);
    g_vRight.normalize();

    g_vUp = crossProduct(g_vRight, g_vLook);
    g_vUp.normalize();

    view.m[0] =  g_vRight.x;
    view.m[1] =  g_vUp.x;
    view.m[2] = -g_vLook.x;
    view.m[3] =  0.0f;

    view.m[4] =  g_vRight.y;
    view.m[5] =  g_vUp.y;
    view.m[6] = -g_vLook.y;
    view.m[7] =  0.0f;

    view.m[8]  =  g_vRight.z;
    view.m[9]  =  g_vUp.z;
    view.m[10] = -g_vLook.z;
    view.m[11] =  0.0f;

    view.m[12] = -dotProduct(g_vRight, g_vEye);
    view.m[13] = -dotProduct(g_vUp, g_vEye);
    view.m[14] =  dotProduct(g_vLook, g_vEye);
    view.m[15] =  1.0f;

    glMultMatrixf( view.m );
}

//-----------------------------------------------------------------------------
// Name: calculateFrustumPlanes()
// Desc: 
//-----------------------------------------------------------------------------
void calculateFrustumPlanes( void )                                 
{
    float p[16];   // projection matrix
    float mv[16];  // model-view matrix
    float mvp[16]; // model-view-projection matrix
    float t;

    glGetFloatv( GL_PROJECTION_MATRIX, p );
    glGetFloatv( GL_MODELVIEW_MATRIX, mv );

    //
    // Concatenate the projection matrix and the model-view matrix to produce 
    // a combined model-view-projection matrix.
    //
    
    mvp[ 0] = mv[ 0] * p[ 0] + mv[ 1] * p[ 4] + mv[ 2] * p[ 8] + mv[ 3] * p[12];
    mvp[ 1] = mv[ 0] * p[ 1] + mv[ 1] * p[ 5] + mv[ 2] * p[ 9] + mv[ 3] * p[13];
    mvp[ 2] = mv[ 0] * p[ 2] + mv[ 1] * p[ 6] + mv[ 2] * p[10] + mv[ 3] * p[14];
    mvp[ 3] = mv[ 0] * p[ 3] + mv[ 1] * p[ 7] + mv[ 2] * p[11] + mv[ 3] * p[15];

    mvp[ 4] = mv[ 4] * p[ 0] + mv[ 5] * p[ 4] + mv[ 6] * p[ 8] + mv[ 7] * p[12];
    mvp[ 5] = mv[ 4] * p[ 1] + mv[ 5] * p[ 5] + mv[ 6] * p[ 9] + mv[ 7] * p[13];
    mvp[ 6] = mv[ 4] * p[ 2] + mv[ 5] * p[ 6] + mv[ 6] * p[10] + mv[ 7] * p[14];
    mvp[ 7] = mv[ 4] * p[ 3] + mv[ 5] * p[ 7] + mv[ 6] * p[11] + mv[ 7] * p[15];

    mvp[ 8] = mv[ 8] * p[ 0] + mv[ 9] * p[ 4] + mv[10] * p[ 8] + mv[11] * p[12];
    mvp[ 9] = mv[ 8] * p[ 1] + mv[ 9] * p[ 5] + mv[10] * p[ 9] + mv[11] * p[13];
    mvp[10] = mv[ 8] * p[ 2] + mv[ 9] * p[ 6] + mv[10] * p[10] + mv[11] * p[14];
    mvp[11] = mv[ 8] * p[ 3] + mv[ 9] * p[ 7] + mv[10] * p[11] + mv[11] * p[15];

    mvp[12] = mv[12] * p[ 0] + mv[13] * p[ 4] + mv[14] * p[ 8] + mv[15] * p[12];
    mvp[13] = mv[12] * p[ 1] + mv[13] * p[ 5] + mv[14] * p[ 9] + mv[15] * p[13];
    mvp[14] = mv[12] * p[ 2] + mv[13] * p[ 6] + mv[14] * p[10] + mv[15] * p[14];
    mvp[15] = mv[12] * p[ 3] + mv[13] * p[ 7] + mv[14] * p[11] + mv[15] * p[15];

    //
    // Extract the frustum's right clipping plane and normalize it.
    //

    g_frustumPlanes[0][0] = mvp[ 3] - mvp[ 0];
    g_frustumPlanes[0][1] = mvp[ 7] - mvp[ 4];
    g_frustumPlanes[0][2] = mvp[11] - mvp[ 8];
    g_frustumPlanes[0][3] = mvp[15] - mvp[12];

    t = (float) sqrt( g_frustumPlanes[0][0] * g_frustumPlanes[0][0] + 
                      g_frustumPlanes[0][1] * g_frustumPlanes[0][1] + 
                      g_frustumPlanes[0][2] * g_frustumPlanes[0][2] );

    g_frustumPlanes[0][0] /= t;
    g_frustumPlanes[0][1] /= t;
    g_frustumPlanes[0][2] /= t;
    g_frustumPlanes[0][3] /= t;

    //
    // Extract the frustum's left clipping plane and normalize it.
    //

    g_frustumPlanes[1][0] = mvp[ 3] + mvp[ 0];
    g_frustumPlanes[1][1] = mvp[ 7] + mvp[ 4];
    g_frustumPlanes[1][2] = mvp[11] + mvp[ 8];
    g_frustumPlanes[1][3] = mvp[15] + mvp[12];

    t = (float) sqrt( g_frustumPlanes[1][0] * g_frustumPlanes[1][0] + 
                      g_frustumPlanes[1][1] * g_frustumPlanes[1][1] + 
                      g_frustumPlanes[1][2] * g_frustumPlanes[1][2] );

    g_frustumPlanes[1][0] /= t;
    g_frustumPlanes[1][1] /= t;
    g_frustumPlanes[1][2] /= t;
    g_frustumPlanes[1][3] /= t;

    //
    // Extract the frustum's bottom clipping plane and normalize it.
    //

    g_frustumPlanes[2][0] = mvp[ 3] + mvp[ 1];
    g_frustumPlanes[2][1] = mvp[ 7] + mvp[ 5];
    g_frustumPlanes[2][2] = mvp[11] + mvp[ 9];
    g_frustumPlanes[2][3] = mvp[15] + mvp[13];

    t = (float) sqrt( g_frustumPlanes[2][0] * g_frustumPlanes[2][0] + 
                      g_frustumPlanes[2][1] * g_frustumPlanes[2][1] + 
                      g_frustumPlanes[2][2] * g_frustumPlanes[2][2] );

    g_frustumPlanes[2][0] /= t;
    g_frustumPlanes[2][1] /= t;
    g_frustumPlanes[2][2] /= t;
    g_frustumPlanes[2][3] /= t;

    //
    // Extract the frustum's top clipping plane and normalize it.
    //

    g_frustumPlanes[3][0] = mvp[ 3] - mvp[ 1];
    g_frustumPlanes[3][1] = mvp[ 7] - mvp[ 5];
    g_frustumPlanes[3][2] = mvp[11] - mvp[ 9];
    g_frustumPlanes[3][3] = mvp[15] - mvp[13];

    t = (float) sqrt( g_frustumPlanes[3][0] * g_frustumPlanes[3][0] + 
                      g_frustumPlanes[3][1] * g_frustumPlanes[3][1] + 
                      g_frustumPlanes[3][2] * g_frustumPlanes[3][2] );

    g_frustumPlanes[3][0] /= t;
    g_frustumPlanes[3][1] /= t;
    g_frustumPlanes[3][2] /= t;
    g_frustumPlanes[3][3] /= t;

    //
    // Extract the frustum's far clipping plane and normalize it.
    //

    g_frustumPlanes[4][0] = mvp[ 3] - mvp[ 2];
    g_frustumPlanes[4][1] = mvp[ 7] - mvp[ 6];
    g_frustumPlanes[4][2] = mvp[11] - mvp[10];
    g_frustumPlanes[4][3] = mvp[15] - mvp[14];

    t = (float) sqrt( g_frustumPlanes[4][0] * g_frustumPlanes[4][0] +  
                      g_frustumPlanes[4][1] * g_frustumPlanes[4][1] + 
                      g_frustumPlanes[4][2] * g_frustumPlanes[4][2] );

    g_frustumPlanes[4][0] /= t;
    g_frustumPlanes[4][1] /= t;
    g_frustumPlanes[4][2] /= t;
    g_frustumPlanes[4][3] /= t;

    //
    // Extract the frustum's near clipping plane and normalize it.
    //

    g_frustumPlanes[5][0] = mvp[ 3] + mvp[ 2];
    g_frustumPlanes[5][1] = mvp[ 7] + mvp[ 6];
    g_frustumPlanes[5][2] = mvp[11] + mvp[10];
    g_frustumPlanes[5][3] = mvp[15] + mvp[14];

    t = (float) sqrt( g_frustumPlanes[5][0] * g_frustumPlanes[5][0] + 
                      g_frustumPlanes[5][1] * g_frustumPlanes[5][1] + 
                      g_frustumPlanes[5][2] * g_frustumPlanes[5][2] );

    g_frustumPlanes[5][0] /= t;
    g_frustumPlanes[5][1] /= t;
    g_frustumPlanes[5][2] /= t;
    g_frustumPlanes[5][3] /= t;
}

//-----------------------------------------------------------------------------
// Name: isBoundingSphereInFrustum()
// Desc: 
//-----------------------------------------------------------------------------
bool isBoundingSphereInFrustum( float x, float y, float z, float fRadius )
{
    for( int i = 0; i < 6; ++i )
    {
        if( g_frustumPlanes[i][0] * x +
            g_frustumPlanes[i][1] * y +
            g_frustumPlanes[i][2] * z +
            g_frustumPlanes[i][3] <= -fRadius )
            return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: 
//-----------------------------------------------------------------------------
void render( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    getRealTimeUserInput();

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    updateViewMatrix();

    calculateFrustumPlanes();

    //
    // Place our light source at the origin...
    //

    GLfloat position_light0[]= { 0.0f, 0.0f, 0.0f, 1.0f };
    glLightfv( GL_LIGHT0, GL_POSITION, position_light0 );

    //
    // Either render all the objects or use frustum culling to reject objects
    // which are outside the view's frustum.
    //

    int nObjectsCulled = 0;
    
    if( g_bDoFrustumCulling == true )
    {
        //
        // Use frustum culling to render only the spheres that are actually 
        // inside the viewing frustum.
        //

        for( int i = 0; i < NUM_OBJECTS; ++i )
        {
            if( isBoundingSphereInFrustum( g_objects[i].x,
                                           g_objects[i].y,
                                           g_objects[i].z,
                                           g_objects[i].fBoundingSphereRadius ) )
            {
                glPushMatrix();
                {
                    glTranslatef( g_objects[i].x, g_objects[i].y, g_objects[i].z );
                    glColor3f( g_objects[i].r, g_objects[i].g, g_objects[i].b );
                    glCallList( g_displayList );
                }
                glPopMatrix();
            }
            else
                ++nObjectsCulled;
        }
    }
    else
    {
        //
        // If we don't do frustum culling, we'll just have to render everything
        // and let OpenGL clip it away automatically. This, of course, is 
        // wasteful.
        //

        for( int i = 0; i < NUM_OBJECTS; ++i )
        {
            glPushMatrix();
            {
                glTranslatef( g_objects[i].x, g_objects[i].y, g_objects[i].z );
                glColor3f( g_objects[i].r, g_objects[i].g, g_objects[i].b );
                glCallList( g_displayList );
            }
            glPopMatrix();
        }
    }

    //
    // Report frames per second and the number of objects culled...
    //

    timeb currentTime;

    if( g_bFirstRendering == true )
    {
        ftime( &g_lastTime );
        currentTime = g_lastTime;
        g_bFirstRendering = false;
    }
    else
    {
        ftime( &currentTime );

        // This is accurate to one second
        g_fElpasedTime  = (float)(currentTime.time - g_lastTime.time);
        // This gets it down to one ms
        g_fElpasedTime += (float)((currentTime.millitm - g_lastTime.millitm) / 1000.0f);
    }

    static char fpsString[50];

    ++g_nFrameCount;

    // Has one second passed?
    if( g_fElpasedTime - g_fTimeSinceLastReport > 1.0f )
    {
        sprintf( fpsString, "Frames Per Second = %d", g_nFrameCount );

        g_fTimeSinceLastReport = g_fElpasedTime;
        g_nFrameCount = 0;
    }

    static char objectString[50];
    static char infoString[] = "Press F1 to toggle culling.";

    sprintf( objectString, "Objects Culled Away = %d (out of %d)", nObjectsCulled, NUM_OBJECTS );

    beginRenderText( g_nWindowWidth, g_nWindowHeight );
    {
        glColor3f( 1.0f, 1.0f, 1.0f );
        renderText( 5, 15, BITMAP_FONT_TYPE_HELVETICA_12, infoString );
        renderText( 5, 30, BITMAP_FONT_TYPE_HELVETICA_12, objectString );
        renderText( 5, 45, BITMAP_FONT_TYPE_HELVETICA_12, fpsString );
    }
    endRenderText();

    SwapBuffers( g_hDC );
}
