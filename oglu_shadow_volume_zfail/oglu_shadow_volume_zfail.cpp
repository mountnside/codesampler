//------------------------------------------------------------------------------
//
// This user submitted sample is based on an older sample, which is related by 
// topic. Information concerning the new author and the nature of his or her
// modifications are below. For completeness, the original information 
// concerning the older sample is stated in the next comment block. The older 
// information is considered valid unless otherwise stated here. Comments that 
// start with three back-slashes "///" denote note worthy changes from the 
// original sample.
//
//           Name: oglu_shadow_volume_zfail.cpp
//    Modified By: ViLiO (www.rainboxstudios.com)
//  Last Modified: 25th June 2005
//   Modification: Modified Kevin's sample to use the z-fail method, which can 
//                 be used to render correct shadows when the viewing position 
//                 is inside the shadow volume. Z-fail is constantly used and  
//                 the real trick is to know where to use zfail and when to use  
//                 the less cpu intensive z-pass method. That bit you can work  
//                 out for yourselves :P
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//           Name: ogl_shadow_volume.cpp
//         Author: Kevin Harris (kevin@codesampler.com)
//  Last Modified: 02/01/05
//    Description: This sample demonstrates how to create real-time shadows 
//                 under OpenGL using shadow volumes.
//
//                 Please note that this sample is the simplest shadow volume 
//                 sample that I could create. It demonstrates only the basics 
//                 which are required to create a shadow volume and doesn't 
//                 attempt to fix any special cases problems such as what to 
//                 do when the viewing position is inside the shadow volume or 
//                 how to produce shadow volumes for more complicated objects 
//                 which require edge detection.
//
//   Control Keys: Up    - Light moves up
//                 Down  - Light moves down
//                 Left  - Light moves left
//                 Right - Light moves right
//
//                 Left Mouse Button  - Spin the view
//                 Right Mouse Button - Spin the shadow caster (NOT WORKING YET!)
//
//                 F1 - Render shadow volume
//                 F2 - Increase amount of shadow volume extrusion
//                 F3 - Decrease amount of shadow volume extrusion
//------------------------------------------------------------------------------

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

// Spin control for shadow casting quad
float  g_fSpinX_R =   0.0f;
float  g_fSpinY_R =   0.0f;

bool  g_bRenderShadowVolume = false;
float g_fAmountOfExtrusion  = 5.0f;
float g_lightPosition[]     = { 2.0f, 6.0f, 0.0f, 1.0f }; // World position of light source

// GL_C3F
struct Vertex
{
    float x, y, z;
};

Vertex g_shadowCasterVerts[] = 
{
    { -1.0f, 2.5f, -1.0f },{ -1.0f, 2.5f,  1.0f },{ 1.0f, 2.5f,  1.0f },{ 1.0f, 2.5f, -1.0f }
};

float g_shadowCasterNormal[] = { 0.0f, 1.0f, 0.0f };

struct ShadowCaster
{
    Vertex *verts;        // Vertices of the actual shadow casting object
    float  *normal;       // A surface normal for lighting
    int     numVerts;     // Total number of vertices
    int     shadowVolume; // Display list for holding the shadow volume
};

ShadowCaster g_shadowCaster;

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
void extendVertex(float newVert[3], float lightPosit[3], Vertex vert, float ext);
void buildShadowVolume(ShadowCaster *caster, float lightPosit[3], float ext);

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
        "OpenGL - Shadow Volume Using The Z-Fail Technique",
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
                g_bRenderShadowVolume = !g_bRenderShadowVolume;
                break;

            case VK_F2:
                g_fAmountOfExtrusion += 0.1f;
                break;

            case VK_F3:
                g_fAmountOfExtrusion -= 0.1f;
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
// Name: extendVertex()
// Desc: 
//-----------------------------------------------------------------------------
void extendVertex( float newVert[3], float lightPosit[3], Vertex vert, float ext )
{
    float lightDir[3];

    // Create a vector that points from the light's position to the original vertex.
    lightDir[0] = vert.x - lightPosit[0];
    lightDir[1] = vert.y - lightPosit[1];
    lightDir[2] = vert.z - lightPosit[2];

    // Then use that vector to extend the original vertex out to a new position.
    // The distance to extend or extrude the new vector is specified by t.
    newVert[0] = lightPosit[0] + lightDir[0] * ext;
    newVert[1] = lightPosit[1] + lightDir[1] * ext;
    newVert[2] = lightPosit[2] + lightDir[2] * ext;
}

//-----------------------------------------------------------------------------
// Name: buildShadowVolume()
// Desc: 
//-----------------------------------------------------------------------------
void buildShadowVolume( ShadowCaster *caster, float lightPosit[3], float ext )
{
    if( caster->shadowVolume != -1 )
        glDeleteLists( caster->shadowVolume, 0 );

    caster->shadowVolume = glGenLists(1);

    glNewList( caster->shadowVolume, GL_COMPILE );
    {
        glDisable( GL_LIGHTING );

        glBegin( GL_QUADS );
        {
            //
            // For each vertex of the shadow casting object, find the edge 
            // that it helps define and extrude a quad out from that edge.
            //

            for( int i = 0; i < caster->numVerts; ++i )
            {
                // Define the edge we're currently working on extruding...
                int e0 = i;
                int e1 = i+1;

                // If the edge's second vertex is out of array range, 
                // place it back at 0
                if( e1 >= caster->numVerts )
                    e1 = 0;

                /// you may notice that I have flipped the faces when drawing the shadow volumes so they face outwards      
                glVertex3f( caster->verts[e1].x, caster->verts[e1].y, caster->verts[e1].z );
                glVertex3f( caster->verts[e0].x, caster->verts[e0].y, caster->verts[e0].z );

                /// here it gets interesting ...using vertex4f with the w set to 0.0f means the vertex is drawn at infinity :P
                glVertex4f( caster->verts[e0].x-lightPosit[0], caster->verts[e0].y-lightPosit[1], caster->verts[e0].z-lightPosit[2], 0.0f );
                glVertex4f( caster->verts[e1].x-lightPosit[0], caster->verts[e1].y-lightPosit[1], caster->verts[e1].z-lightPosit[2], 0.0f );
            }

            /// the near cap on the volume is just the light occluder ( the shadow caster :P )
            glVertex3f( caster->verts[0].x, caster->verts[0].y, caster->verts[0].z );
            glVertex3f( caster->verts[1].x, caster->verts[1].y, caster->verts[1].z );
            glVertex3f( caster->verts[2].x, caster->verts[2].y, caster->verts[2].z );
            glVertex3f( caster->verts[3].x, caster->verts[3].y, caster->verts[3].z );

            /// When using a point light you have to cap the volume at infinity ( i'm serious :P )
            /// You simply draw the shadow caster again only extended out to infinity and reverse the vertex order (flip the faces)
            /// This makes a completely enclosed volume with all faces of the volume pointing outwards :P
            glVertex4f( caster->verts[3].x-lightPosit[0], caster->verts[3].y-lightPosit[1], caster->verts[3].z-lightPosit[2],0.0f);
            glVertex4f( caster->verts[2].x-lightPosit[0], caster->verts[2].y-lightPosit[1], caster->verts[2].z-lightPosit[2],0.0f);
            glVertex4f( caster->verts[1].x-lightPosit[0], caster->verts[1].y-lightPosit[1], caster->verts[1].z-lightPosit[2],0.0f);
            glVertex4f( caster->verts[0].x-lightPosit[0], caster->verts[0].y-lightPosit[1], caster->verts[0].z-lightPosit[2],0.0f);
        }
        glEnd();

        glEnable( GL_LIGHTING );
    }
    glEndList();
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

    pfd.nSize        = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion     = 1;
    pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType   = PFD_TYPE_RGBA;
    pfd.cColorBits   = 32; /// 32 bit looks nicer :P
    pfd.cDepthBits   = 24; /// bump the depth up a little
    pfd.cAlphaBits   = 8;  /// we may as well have alpha in there too
    pfd.cStencilBits = 8;  /// and of course we cant do stencil shadow volumes without some stencil bits :P

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
    gluPerspective( 45.0f, 640.0f / 480.0f, 1.0f, -1.0f); /// the -1.0f far plane pretty much makes the far plane infinitely far
    /// you can try changing to far plane to 1000.0f or something and you will notice problem start occurring with the far cap on the shadow volume

    // Enable a single OpenGL light.
    float lightAmbient[] = {0.2f, 0.2f, 0.2f, 1.0f};
    float lightDiffuse[] = {1.0, 1.0, 1.0, 1.0}; 
    float lightSpecular[] = {1.0, 1.0, 1.0, 1.0};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);

    //
    // Set up the shadow caster...
    //

    g_shadowCaster.verts        = g_shadowCasterVerts;
    g_shadowCaster.normal       = g_shadowCasterNormal;
    g_shadowCaster.numVerts     = (sizeof(g_shadowCasterVerts) / sizeof(Vertex));
    g_shadowCaster.shadowVolume = -1;
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
void renderScene( void )
{
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

    //
    // Render a teapot so we'll have something else to cast a shadow on  
    // besides the floor.
    //

    glPushMatrix();
    {
        glTranslatef( -2.0f, 0.8f, 0.0f );
        glRotatef( 180.0, 0.0f, 1.0f, 0.0f );
        glColor3f( 1.0f, 1.0f ,1.0f );
        renderSolidTeapot( 1.0);
    }
    glPopMatrix();

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
    // Render the shadow caster (i.e. the quad)
    //

    glPushMatrix();
    {
        // Hmmm... I can't transform the shadow caster unless the 
        // buildShadowVolume function is able to take this into account. 
        // This is because the shadow volume is built in world space.

        //glTranslatef( 0.0f, 2.5f, 0.0f );
        //glRotatef( -g_fSpinY_R, 1.0f, 0.0f, 0.0f );
        //glRotatef( -g_fSpinX_R, 0.0f, 1.0f, 0.0f );

        glBegin( GL_POLYGON );
        {
            glNormal3fv( g_shadowCaster.normal );

            for( int i = 0; i < g_shadowCaster.numVerts; ++i )
            {
                glVertex3f( g_shadowCaster.verts[i].x,
                    g_shadowCaster.verts[i].y,
                    g_shadowCaster.verts[i].z );
            }
        }
        glEnd();
    }
    glPopMatrix();
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: 
//-----------------------------------------------------------------------------
void render( void )
{
    //
    // Using the light's position, extend or extrude each vertex of the shadow
    // caster out by an amount specified by g_fAmountOfExtrusion.
    //

    buildShadowVolume( &g_shadowCaster, g_lightPosition, g_fAmountOfExtrusion );

    //
    // Prepare to render a new scene by clearing out all of the buffers.
    //

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

    // Initialize the depth buffer by rendering the scene into it.
    glDisable(GL_LIGHT0); ///ambient pass ...well ok no light pass so call it ambient = 0.0f :P
    renderScene();
    glEnable(GL_LIGHT0); ///flame on

    //
    // Create the special shadow stencil...
    //

    // Set the appropriate states for creating a stencil for shadowing.
    glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
    /// we wanted the first ambient pass above in the color buffer but we don't want the volumes in there ..removing the above line is fun though :P
    glEnable( GL_CULL_FACE );
    glEnable( GL_STENCIL_TEST );
    glDepthMask( GL_FALSE );

    // Render the shadow volume and increment the stencil every where a front
    // facing polygon is rendered.

    /// Incrementing the stencil buffer when back face depth fails
    glStencilFunc( GL_ALWAYS, 0, ~0 ); /// ~0 is like 0xFFFFFFFF or something :P
    glStencilOp( GL_KEEP, GL_INCR, GL_KEEP ); /// incrementing on the depth fail
    glCullFace( GL_FRONT ); /// cull front facing polys for this pass
    glCallList( g_shadowCaster.shadowVolume );

    // Render the shadow volume and decrement the stencil every where a back
    // facing polygon is rendered.
    glStencilOp( GL_KEEP, GL_DECR, GL_KEEP ); /// decrementing on the depth fail
    glCullFace( GL_BACK ); /// and now culling back facing polys
    glCallList( g_shadowCaster.shadowVolume );

    // When done, set the states back to something more typical.
    glDepthMask( GL_TRUE );
    glDepthFunc( GL_LEQUAL );
    glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
    glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
    glCullFace( GL_BACK );
    glDisable( GL_CULL_FACE );

    //
    // Render the lit part...
    //

    glStencilFunc( GL_EQUAL, 0, ~0 ); /// again with the ~0 :P
    glEnable( GL_LIGHT0 );
    renderScene();

    // When done, set the states back to something more typical.
    glDepthFunc( GL_LESS );
    glDisable( GL_STENCIL_TEST );

    if( g_bRenderShadowVolume )
    {
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        glCallList( g_shadowCaster.shadowVolume );
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    }

    SwapBuffers( g_hDC );
}