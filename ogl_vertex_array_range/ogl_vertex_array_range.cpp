//-----------------------------------------------------------------------------
//           Name: ogl_vertex_array_range.cpp
//         Author: Kevin Harris
//  Last Modified: 02/01/05
//    Description: This sample demonstrates how to use the OpenGL extension 
//                 NV_vertex_array_range to speed up the rendering of geometry 
//                 by loading its vertex data and/or indices data directly into 
//                 AGP or Video memory. Of course, the downside to this is that 
//                 you become responsible for managing AGP and Video Memory 
//                 which can seriously complicate your code.
//
//   Control Keys: F1 - Toggles between using Vertex Array Range buffers and
//                      regular Vertex Arrays.
//
//                 F2 - Toggles the test geometry between a complicated sphere
//                      and a very simple quad. The sphere is heavily  
//                      tessellated and is useful in benchmarking while the 
//                      simple quad demonstrates how to use indices with the 
//                      extension.
//
//                 F3 - Toggles wire-frame mode.
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/timeb.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glaux.h>
#include "bitmap_fonts.h"
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
//PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB = NULL;

// WGL_NV_allocate_memory
PFNWGLALLOCATEMEMORYNVPROC wglAllocateMemoryNV = NULL;
PFNWGLFREEMEMORYNVPROC     wglFreeMemoryNV     = NULL;

// GL_NV_vertex_array_range
PFNGLFLUSHVERTEXARRAYRANGENVPROC glFlushVertexArrayRangeNV = NULL;
PFNGLVERTEXARRAYRANGENVPROC      glVertexArrayRangeNV      = NULL;

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND   g_hWnd          = NULL;
HDC    g_hDC           = NULL;
HGLRC  g_hRC           = NULL;
int    g_nWindowWidth  = 640;
int    g_nWindowHeight = 480;
GLuint g_textureID     = -1;

bool  g_bFirstRendering = true;
timeb g_lastTime;
float g_fTimeSinceLastReport = 0.0;
int   g_nFrameCount = 0;

bool g_bRenderInWireFrame   = false;
bool g_bUseVertexArrayRange = true;
bool g_bRenderQuad          = false;

int      g_nTotalBufferSize     = 0;
GLfloat *g_pAGPMemoryBuffer     = NULL;
int      g_nMemoryOffsetForQuad = 0;

// GL_T2F_N3F_V3F
struct Vertex
{
    GLfloat tu, tv;
    GLfloat nx, ny, nz;
    GLfloat vx, vy, vz;
};

// Textured sphere
GLuint  g_nPrecision = 500;
GLuint  g_nNumSphereVertices;
Vertex *g_pSphereVertices = NULL;

// Textured quad
int g_nNumQuadTriangles = 2;
int g_nNumQuadVertices  = 4;
int g_nNumQuadIndices   = 6;

GLubyte g_pQuadIndices[] = 
{
    0, 1, 2, // Tri 1
    1, 3, 2  // Tri 2
};

Vertex g_pQuadVertices[] = 
{
    { 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,  -1.0f, -1.0f, 0.0f },  
    { 1.0f, 0.0f,  0.0f, 0.0f, 1.0f,   1.0f, -1.0f, 0.0f },  
    { 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  -1.0f,  1.0f, 0.0f },  
    { 1.0f, 1.0f,  0.0f, 0.0f, 1.0f,   1.0f,  1.0f, 0.0f }  
};

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND g_hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void loadTexture(void);
void init(void);
void render(void);
void shutDown(void);
GLvoid createSphereGeometry( float cx, float cy, float cz, float r, int n);
GLvoid setVertData(int index,float tu, float tv, float nx, float ny, float nz, 
                   float vx, float vy, float vz);   

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
                             "OpenGL - Vertex Array Range (NV_vertex_array_range - nVIDIA only) ",
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
LRESULT CALLBACK WindowProc( HWND   g_hWnd, 
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
                    g_bUseVertexArrayRange = !g_bUseVertexArrayRange;
                    break;

                case VK_F2:
                    g_bRenderQuad  = !g_bRenderQuad;
                    break;

				case VK_F3:
					g_bRenderInWireFrame = !g_bRenderInWireFrame;
					if( g_bRenderInWireFrame == true )
						glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
					else
						glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
					break;
            }
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
void loadTexture( void )    
{
    AUX_RGBImageRec *pTextureImage = auxDIBImageLoad( ".\\earth.bmp" );

    if( pTextureImage != NULL )
    {
        glGenTextures( 1, &g_textureID );

        glBindTexture( GL_TEXTURE_2D, g_textureID );

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
    glEnable( GL_TEXTURE_2D );
    glEnable( GL_DEPTH_TEST );

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective( 45.0, (GLdouble)g_nWindowWidth / g_nWindowHeight, 0.1, 100.0 );

    createSphereGeometry( 0.0f, 0.0f, 0.0f, 1.5f, g_nPrecision );

    /*
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
    */

    //
    // WGL_NV_allocate_memory 
    //
    // For some reason, I can't find this using wglGetExtensionsStringARB or 
    // glGetString, but it is available... at least on my machine.
    //

    wglAllocateMemoryNV = (PFNWGLALLOCATEMEMORYNVPROC)wglGetProcAddress("wglAllocateMemoryNV");
    wglFreeMemoryNV     = (PFNWGLFREEMEMORYNVPROC)wglGetProcAddress("wglFreeMemoryNV");

    if( !wglAllocateMemoryNV || !wglFreeMemoryNV  )
    {
        MessageBox(NULL,"One or more WGL_NV_allocate_memory functions were not found",
            "ERROR",MB_OK|MB_ICONEXCLAMATION);
        exit(-1);
    }

    //
    // GL_NV_vertex_array_range
    //

    char *gl_ext = (char*)glGetString( GL_EXTENSIONS );

    if( strstr( gl_ext, "GL_NV_vertex_array_range" ) == NULL )
    {
        MessageBox(NULL,"GL_NV_vertex_array_range extension was not found",
            "ERROR",MB_OK|MB_ICONEXCLAMATION);
        return;
    }
    else
    {
        glFlushVertexArrayRangeNV = (PFNGLFLUSHVERTEXARRAYRANGENVPROC)wglGetProcAddress("glFlushVertexArrayRangeNV");
        glVertexArrayRangeNV      = (PFNGLVERTEXARRAYRANGENVPROC)wglGetProcAddress("glVertexArrayRangeNV");

        if( !glFlushVertexArrayRangeNV || !glVertexArrayRangeNV )
        {
            MessageBox(NULL,"One or more GL_NV_vertex_array_range functions were not found",
                "ERROR",MB_OK|MB_ICONEXCLAMATION);
            return;
        }
    }

    //
    // For best results, only allocate memory once. If multiple buffers are 
    // needed, sum the sizes and allocate one large buffer, then partition it 
    // after the allocation. Switching between multiple (separately allocated) 
    // buffers is expensive and unnecessary.
    //
    // To allocate AGP/Video memory, one must supply the wglAllocateMemoryNV 
    // call with the appropriate usage hints, which are passed in through the 
    // arguments: readfreq, writefreq, and priority:
    //
    // Memory Allocated   readfreq   writefreq   priority
    //
    // AGP Memory         [0, .25)   [0, .25)    (.25, .75]
    // Video Memory       [0, .25)   [0, .25)    (.75, 1]
	//
    // The application rates the read frequency (how often will they read the 
    // memory), the write frequency (how often will they write the memory), 
    // and the priority (how important is this memory relative to other uses 
    // for the memory such as texturing) on a scale of 1.0 to 0.0. Using 
    // these hints and the size of the memory requested, the OpenGL 
    // implementation decides where to allocate the memory.
    //
	// (See the exstenion's online specification for more detials)
	//
    // We try to not directly expose particular types of memory (AGP, local 
    // memory, cached/uncached, etc) so future memory types can be supported 
    // by merely updating the OpenGL implementation.
    //

    // Since we're allocating everything from one large block of AGP/Video 
    // memory we need to calculate the total amount of memory we're going to 
	// need.
    int nAllocationSizeForSphere = g_nNumSphereVertices * sizeof(Vertex);
    int nAllocationSizeForQuad   = g_nNumQuadVertices   * sizeof(Vertex);

    g_nTotalBufferSize = nAllocationSizeForSphere + nAllocationSizeForQuad;

    g_pAGPMemoryBuffer = (GLfloat*)wglAllocateMemoryNV( g_nTotalBufferSize, 0.2f, 0.2f, 0.5f );

    if( !g_pAGPMemoryBuffer )
    {
        MessageBox(NULL,"Call to wglAllocateMemoryNV failed!",
            "ERROR",MB_OK|MB_ICONEXCLAMATION);
    }

    glVertexArrayRangeNV( g_nTotalBufferSize, g_pAGPMemoryBuffer );

    // First, copy the sphere's vertex data into our chunk of high-speed memory

    memcpy( g_pAGPMemoryBuffer, (void*)g_pSphereVertices, nAllocationSizeForSphere );

    // Next, copy the quad's vertex data into our chunk of high-speed memory.
    // Note how we use an offset to load in the quad's vertex data. In a 
	// real-life aplication you would most likely have to code up some sort of 
	// memory manager to keep track of your personel chunk of AGP/Video memory.
	// This is probbaly one of the biggest headaches of using an exsention like
	// this.

    g_nMemoryOffsetForQuad = g_nNumSphereVertices;
    memcpy( g_pAGPMemoryBuffer + g_nMemoryOffsetForQuad, (void*)g_pQuadVertices, nAllocationSizeForQuad );
}

//-----------------------------------------------------------------------------
// Name: setVertData()
// Desc: Helper function for CreateSphereGeometry()
//-----------------------------------------------------------------------------
GLvoid setVertData( int index,
                   float tu, float tv, 
                   float nx, float ny, float nz, 
                   float vx, float vy, float vz )   
{
    (g_pSphereVertices+index)->tu = tu;
    (g_pSphereVertices+index)->tv = tv;
    (g_pSphereVertices+index)->nx = nx;
    (g_pSphereVertices+index)->ny = ny;
    (g_pSphereVertices+index)->nz = nz;
    (g_pSphereVertices+index)->vx = vx;
    (g_pSphereVertices+index)->vy = vy;
    (g_pSphereVertices+index)->vz = vz;
}

//-----------------------------------------------------------------------------
// Name: createSphereGeometry()
// Desc: Creates a sphere as an array of vertex data suitable to be fed into a 
//       OpenGL vertex array. The sphere will be centered at cy, cx, cz with 
//       radius r, and precision p. Based on a function Written by Paul Bourke. 
//       http://astronomy.swin.edu.au/~pbourke/opengl/sphere/
//-----------------------------------------------------------------------------
GLvoid createSphereGeometry( float cx, float cy, float cz, float r, int p )	
{
    const float PI = 3.14159265358979f;
    const float TWOPI = 6.28318530717958f;
    const float PIDIV2 = 1.57079632679489f;

    float theta1 = 0.0;
    float theta2 = 0.0;
    float theta3 = 0.0;

    float ex = 0.0f;
    float ey = 0.0f;
    float ez = 0.0f;

    float px = 0.0f;
    float py = 0.0f;
    float pz = 0.0f;

    float tu  = 0.0f;
    float tv  = 0.0f;

    //-------------------------------------------------------------------------
    // If sphere precision is set to 4, then 20 verts will be needed to 
    // hold the array of GL_TRIANGLE_STRIP(s) and so on...
    //
    // Example:
    //
    // total_verts = (p/2) * ((p+1)*2)
    // total_verts = (4/2) * (  5  *2)
    // total_verts =   2   *  10
    // total_verts =      20
    //-------------------------------------------------------------------------

    g_nNumSphereVertices = (p/2) * ((p+1)*2);

    if( g_pSphereVertices != NULL )
    {
        delete []g_pSphereVertices;
        g_pSphereVertices = NULL;
        g_pSphereVertices = new Vertex[g_nNumSphereVertices];
    }
    else
    {
        g_pSphereVertices = new Vertex[g_nNumSphereVertices];
    }

    // Disallow a negative number for radius.
    if( r < 0 )
        r = -r;

    // Disallow a negative number for precision.
    if( p < 4 ) 
        p = 4;

    int k = -1;

    for( int i = 0; i < p/2; ++i )
    {
        theta1 = i * TWOPI / p - PIDIV2;
        theta2 = (i + 1) * TWOPI / p - PIDIV2;

        for( int j = 0; j <= p; ++j )
        {
            theta3 = j * TWOPI / p;

            ex = cosf(theta2) * cosf(theta3);
            ey = sinf(theta2);
            ez = cosf(theta2) * sinf(theta3);
            px = cx + r * ex;
            py = cy + r * ey;
            pz = cz + r * ez;
            tu  = -(j/(float)p);
            tv  = 2*(i+1)/(float)p;

            ++k;
            setVertData( k, tu, tv, ex, ey, ez, px, py, pz );

            ex = cosf(theta1) * cosf(theta3);
            ey = sinf(theta1);
            ez = cosf(theta1) * sinf(theta3);
            px = cx + r * ex;
            py = cy + r * ey;
            pz = cz + r * ez;
            tu  = -(j/(float)p);
            tv  = 2*i/(float)p;

            ++k;
            setVertData( k, tu, tv, ex, ey, ez, px, py, pz );
        }
    }
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown(void) 
{
    glDeleteTextures( 1, &g_textureID );

    if( g_pAGPMemoryBuffer != NULL )
        wglFreeMemoryNV( g_pAGPMemoryBuffer );

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
    
    static float fYrot = 0.0f;
    fYrot += 0.02f;

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glTranslatef( 0.0f, 0.0f, -5.0f );
    glRotatef( fYrot, 0.0f, 1.0f, 0.0f );
    
    if( g_bUseVertexArrayRange == true )
    {
        if( g_bRenderQuad == true )
        {
            //
            // Render a textured quad using indices and Vertex Array Range
            //

            glEnableClientState( GL_VERTEX_ARRAY_RANGE_NV );
            {
                glBindTexture( GL_TEXTURE_2D, g_textureID );
                glInterleavedArrays( GL_T2F_N3F_V3F, 0, g_pAGPMemoryBuffer + g_nMemoryOffsetForQuad );
                glDrawElements( GL_TRIANGLES, g_nNumQuadIndices, GL_UNSIGNED_BYTE, g_pQuadIndices );
            }
            glDisableClientState( GL_VERTEX_ARRAY_RANGE_NV );
        }
        else
        {
            //
            // Render a textured sphere using Vertex Buffer Range
            //

            glEnableClientState( GL_VERTEX_ARRAY_RANGE_NV );
            {
                glBindTexture( GL_TEXTURE_2D, g_textureID );
                glInterleavedArrays( GL_T2F_N3F_V3F, 0, g_pAGPMemoryBuffer );
                glDrawArrays( GL_QUAD_STRIP, 0, g_nNumSphereVertices );
            }
            glDisableClientState( GL_VERTEX_ARRAY_RANGE_NV );
        }
    }
    else
    {
        if( g_bRenderQuad == true )
        {
            //
            // Render a textured quad using indices and a regular Vertex Array
            //

            glBindTexture( GL_TEXTURE_2D, g_textureID );
            glInterleavedArrays( GL_T2F_N3F_V3F, 0, g_pQuadVertices );
            glDrawElements( GL_TRIANGLES, g_nNumQuadIndices, GL_UNSIGNED_BYTE, g_pQuadIndices );
        }
        else
        {
            //
            // Render a textured sphere using a regular Vertex Array
            //

            glBindTexture( GL_TEXTURE_2D, g_textureID );
            glInterleavedArrays( GL_T2F_N3F_V3F, 0, g_pSphereVertices );
            glDrawArrays( GL_QUAD_STRIP, 0, g_nNumSphereVertices );
        }
    }

    //
    // Report frames-per-second and other settings...
    //

    timeb currentTime;
    float fElapsed = 0.0f;

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
        fElapsed  = (float)(currentTime.time - g_lastTime.time);
        // This gets it down to one ms
        fElapsed += (float)((currentTime.millitm - g_lastTime.millitm) / 1000.0f);
    }

    static char fpsString[50];

    ++g_nFrameCount;

    // Has one second passed?
    if( fElapsed - g_fTimeSinceLastReport > 1.0f )
    {
        sprintf( fpsString, "Frames Per Second = %d", g_nFrameCount );

        g_fTimeSinceLastReport = fElapsed;
        g_nFrameCount = 0;
    }

    static char geometryStorageString[255];
    static char geometryString[255];

    if( g_bUseVertexArrayRange == true )
    {
        if( g_bRenderQuad == true )
            sprintf( geometryStorageString, "Geometry Storage: %s", "Vertex Array Range & Index buffer (Change: F1)" );
        else
            sprintf( geometryStorageString, "Geometry Storage: %s", "Vertex Array Range (Change: F1)" );
    }
    else
    {
        sprintf( geometryStorageString, "Geometry Storage: %s", "Vertex Array (Change: F1)" );
    }

    if( g_bRenderQuad == true )
        sprintf( geometryString, "Geometry: %s", "Textured quad (Change: F2)" );
    else
        sprintf( geometryString, "Geometry: %s", "Textured sphere (Change: F2)" );	

    beginRenderText( g_nWindowWidth, g_nWindowHeight );
    {
        glColor3f( 1.0f, 1.0f, 1.0f );
        renderText( 5, 15, BITMAP_FONT_TYPE_HELVETICA_12, geometryStorageString );
        renderText( 5, 30, BITMAP_FONT_TYPE_HELVETICA_12, geometryString );
        renderText( 5, 45, BITMAP_FONT_TYPE_HELVETICA_12, fpsString );
    }
    endRenderText();

    SwapBuffers( g_hDC );
}
