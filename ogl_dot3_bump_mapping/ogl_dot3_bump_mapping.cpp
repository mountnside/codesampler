//-----------------------------------------------------------------------------
//           Name: ogl_dot3_bump_mapping.cpp
//         Author: Kevin Harris
//  Last Modified: 04/21/05
//    Description: This sample demonstrates how to perform Dot3 per-pixel bump 
//                 mapping using a normal map and the GL_DOT3_RGB_EXT 
//                 texture-blending operation. This technique is sometimes 
//                 referred to as per-pixel lighting or per-pixel attenuation, 
//                 but Dot3 per-pixel bump-mapping is what most programmers 
//                 know it as.
//
//                 This sample also demonstrates how to create tangent, 
//                 binormal, and normal vectors, for each vertex of our test
//                 geometry (a simple cube). These vectors are used during 
//                 rendering to define an inverse tangent matrix for each
//                 vertex. This has to be done because a normal-map stores its 
//                 normals in tangent-space. Therefore, we need an inverse 
//                 tangent matrix so we can transform our scene's light vector 
//                 from object-space into tangent-space. Once transformed, we 
//                 then encode this new light vector as a RGB color and pass 
//                 it into the texture blending stage as the vertex's Diffuse 
//                 color.
//
// Additional Notes:
//
//                 The phrase "Dot3" comes form the mathematical operation that 
//                 combines a light vector with a surface normal.
//
//                 The phrase "Per-pixel" comes from the fact that for every 
//                 pixel in the base texture map, we store a unique surface 
//                 normal to light it. These surface normals are passed into the
//                 texture blending stage via a normal-map. A normal-map is a 
//                 texture where normals (x,y,z) have been encoded and stored 
//                 as (r,g,b).
//
//   Control Keys: d/D - Toggle Dot3 bump mapping
//                 l/L - Toggle the usage of regular lighting in addition
//                       to the per-pixel lighting effect of bump mapping.
//                 m/M - Toggle motion of point light
//                 Up Arrow - Move the test cube closer
//                 Down Arrow - Move the test cube away
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <mmsystem.h>
// Do this to access M_PI, which is not officially part of the C/C++ standard.
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glaux.h>
#include "geometry.h"
#include "matrix4x4f.h"
#include "vector3f.h"
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

PFNGLACTIVETEXTUREPROC       glActiveTexture       = NULL;
PFNGLMULTITEXCOORD2FPROC     glMultiTexCoord2f     = NULL;
PFNGLCLIENTACTIVETEXTUREPROC glClientActiveTexture = NULL;

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HDC   g_hDC  = NULL;
HGLRC g_hRC  = NULL;
HWND  g_hWnd = NULL;

GLuint g_textureID = -1;
GLuint g_normalmapTextureID = -1;
bool g_bDoDot3BumpMapping = true;
bool g_bMoveLightAbout = true;
bool g_bToggleRegularLighting = false;
GLfloat g_lightPosition[] = { 0.0f, 0.0f, 0.0f, 1.0f }; // Point Light's position

float g_fDistance = -5.0f;
float g_fSpinX = 0.0f;
float g_fSpinY = 0.0f;

float  g_fElpasedTime;
double g_dCurTime;
double g_dLastTime;

struct Vertex
{
    float tu, tv;
    float r, g, b, a;
    float nx, ny, nz;
    float x, y, z;
};

const int NUM_VERTICES = 24;

Vertex g_cubeVertices[NUM_VERTICES] =
{
//    tu   tv     r    g    b    a      nx    ny    nz       x     y     z 
    // Front face
    { 0.0f,0.0f,  1.0f,1.0f,1.0f,1.0f,  0.0f, 0.0f, 1.0f,  -1.0f,-1.0f, 1.0f },
    { 1.0f,0.0f,  1.0f,1.0f,1.0f,1.0f,  0.0f, 0.0f, 1.0f,   1.0f,-1.0f, 1.0f },
    { 1.0f,1.0f,  1.0f,1.0f,1.0f,1.0f,  0.0f, 0.0f, 1.0f,   1.0f, 1.0f, 1.0f },
    { 0.0f,1.0f,  1.0f,1.0f,1.0f,1.0f,  0.0f, 0.0f, 1.0f,  -1.0f, 1.0f, 1.0f },
    // Back face
    { 1.0f,0.0f,  1.0f,1.0f,1.0f,1.0f,  0.0f, 0.0f,-1.0f,  -1.0f,-1.0f,-1.0f },
    { 1.0f,1.0f,  1.0f,1.0f,1.0f,1.0f,  0.0f, 0.0f,-1.0f,  -1.0f, 1.0f,-1.0f },
    { 0.0f,1.0f,  1.0f,1.0f,1.0f,1.0f,  0.0f, 0.0f,-1.0f,   1.0f, 1.0f,-1.0f },
    { 0.0f,0.0f,  1.0f,1.0f,1.0f,1.0f,  0.0f, 0.0f,-1.0f,   1.0f,-1.0f,-1.0f },
    // Top face
    { 0.0f,1.0f,  1.0f,1.0f,1.0f,1.0f,  0.0f, 1.0f, 0.0f,  -1.0f, 1.0f,-1.0f },
    { 0.0f,0.0f,  1.0f,1.0f,1.0f,1.0f,  0.0f, 1.0f, 0.0f,  -1.0f, 1.0f, 1.0f },
    { 1.0f,0.0f,  1.0f,1.0f,1.0f,1.0f,  0.0f, 1.0f, 0.0f,   1.0f, 1.0f, 1.0f },
    { 1.0f,1.0f,  1.0f,1.0f,1.0f,1.0f,  0.0f, 1.0f, 0.0f,   1.0f, 1.0f,-1.0f },
    // Bottom face
    { 1.0f,1.0f,  1.0f,1.0f,1.0f,1.0f,  0.0f,-1.0f, 0.0f,  -1.0f,-1.0f,-1.0f },
    { 0.0f,1.0f,  1.0f,1.0f,1.0f,1.0f,  0.0f,-1.0f, 0.0f,   1.0f,-1.0f,-1.0f },
    { 0.0f,0.0f,  1.0f,1.0f,1.0f,1.0f,  0.0f,-1.0f, 0.0f,   1.0f,-1.0f, 1.0f },
    { 1.0f,0.0f,  1.0f,1.0f,1.0f,1.0f,  0.0f,-1.0f, 0.0f,  -1.0f,-1.0f, 1.0f },
    // Right face
    { 1.0f,0.0f,  1.0f,1.0f,1.0f,1.0f,  1.0f, 0.0f, 0.0f,   1.0f,-1.0f,-1.0f },
    { 1.0f,1.0f,  1.0f,1.0f,1.0f,1.0f,  1.0f, 0.0f, 0.0f,   1.0f, 1.0f,-1.0f },
    { 0.0f,1.0f,  1.0f,1.0f,1.0f,1.0f,  1.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f },
    { 0.0f,0.0f,  1.0f,1.0f,1.0f,1.0f,  1.0f, 0.0f, 0.0f,   1.0f,-1.0f, 1.0f },
    // Left face
    { 0.0f,0.0f,  1.0f,1.0f,1.0f,1.0f, -1.0f, 0.0f, 0.0f,  -1.0f,-1.0f,-1.0f },
    { 1.0f,0.0f,  1.0f,1.0f,1.0f,1.0f, -1.0f, 0.0f, 0.0f,  -1.0f,-1.0f, 1.0f },
    { 1.0f,1.0f,  1.0f,1.0f,1.0f,1.0f, -1.0f, 0.0f, 0.0f,  -1.0f, 1.0f, 1.0f },
    { 0.0f,1.0f,  1.0f,1.0f,1.0f,1.0f, -1.0f, 0.0f, 0.0f,  -1.0f, 1.0f,-1.0f }
};

// For each vertex defined above, we'll need to create a Tangent, BiNormal, and
// Normal vector, which together define a tangent matrix for Dot3 bump mapping.
vector3f g_vTangents[NUM_VERTICES];
vector3f g_vBiNormals[NUM_VERTICES];
vector3f g_vNormals[NUM_VERTICES];

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
vector3f scaleAndBiasVectorAsRGBColor(vector3f* vVector);
void computeTangentsMatricesForEachVertex(void);
void createTangentSpaceVectors( vector3f *v1, vector3f *v2, vector3f *v3,
                                float v1u, float v1v, float v2u, float v2v, float v3u, float v3v, 
                                vector3f *vTangent, vector3f *vBiNormal, vector3f *vNormal );

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

    g_hWnd = CreateWindowEx( NULL,"MY_WINDOWS_CLASS",
                            "OpenGL - Dot3 Per-Pixel Bump Mapping",
                            WS_OVERLAPPEDWINDOW,
                            0,0, 640,480, NULL, NULL, hInstance, NULL );

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
        {
            g_dCurTime     = timeGetTime();
            g_fElpasedTime = (float)((g_dCurTime - g_dLastTime) * 0.001);
            g_dLastTime    = g_dCurTime;

            render();
        }
    }

    shutDown();

    UnregisterClass( "MY_WINDOWS_CLASS", hInstance );

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
        case WM_CHAR:
        {
            switch( wParam )
            {
                case 'd':
                case 'D':
                    g_bDoDot3BumpMapping = !g_bDoDot3BumpMapping;
                    break;

                case 'l':
                case 'L':
                    g_bToggleRegularLighting = !g_bToggleRegularLighting;
                    break;

                case 'm':
                case 'M':
                    g_bMoveLightAbout = !g_bMoveLightAbout;
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
                    g_fDistance += 0.1f;
                    break;

                case 40: // Down Arrow Key
                    g_fDistance -= 0.1f;
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
    //
    // Load the normal map...
    //

    AUX_RGBImageRec *pTextureImage = auxDIBImageLoad( ".\\stone_wall_normal_map.bmp" );
    //AUX_RGBImageRec *pTextureImage = auxDIBImageLoad( ".\\test_normal_map.bmp" );

    if( pTextureImage != NULL )
    {
        glGenTextures( 1, &g_normalmapTextureID );

        glBindTexture( GL_TEXTURE_2D, g_normalmapTextureID );

        glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR );

        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER_ARB);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER_ARB);

        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, pTextureImage->sizeX, pTextureImage->sizeY, 0,
                GL_RGB, GL_UNSIGNED_BYTE, pTextureImage->data );
    }

    if( pTextureImage )
    {
        if( pTextureImage->data )
            free( pTextureImage->data );

        free( pTextureImage );
    }

    //
    // Load the base texture
    //

    pTextureImage = auxDIBImageLoad( ".\\stone_wall.bmp" );
    //pTextureImage = auxDIBImageLoad( ".\\checker_with_numbers.bmp" );

    if( pTextureImage != NULL )
    {
        glGenTextures( 1, &g_textureID );

        glBindTexture( GL_TEXTURE_2D, g_textureID );

        glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR );

        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER_ARB);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER_ARB);

        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, pTextureImage->sizeX, pTextureImage->sizeY, 0,
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
    SetPixelFormat( g_hDC, PixelFormat, &pfd);
    g_hRC = wglCreateContext( g_hDC );
    wglMakeCurrent( g_hDC, g_hRC );

    glClearColor( 0.35f, 0.53f, 0.7f, 1.0f );
    glEnable( GL_TEXTURE_2D );
    glEnable( GL_DEPTH_TEST );

    //glShadeModel( GL_FLAT );
    glShadeModel( GL_SMOOTH ); // Supposedly this is important to set when doing Dot3, but I notice no difference.

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective( 45.0f, 640.0f / 480.0f, 0.1f, 100.0f);

    // Set up the view matrix
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

	// For lighting, pull our material colors from the vertex color...
    glColorMaterial( GL_FRONT, GL_AMBIENT_AND_DIFFUSE );
    glEnable( GL_COLOR_MATERIAL );
  
    // Set light 0 to be a pure white point light
    GLfloat diffuse_light0[]  = { 1.0f, 1.0f,  1.0f, 1.0f };
    glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuse_light0 );
    glLightfv( GL_LIGHT0, GL_POSITION, g_lightPosition );
    glEnable( GL_LIGHTING );
    glEnable( GL_LIGHT0 );

    // Enable some dim, gray ambient lighting so objects that are not lit 
    // by the other lights are not completely black.
    GLfloat ambient_lightModel[] = { 0.2f, 0.2f, 0.2f, 0.2f };
    glLightModelfv( GL_LIGHT_MODEL_AMBIENT, ambient_lightModel );

    loadTexture();

    //
    // If the required extension is present, get the addresses of its 
    // functions that we wish to use...
    //

    //
    // GL_ARB_multitexture
    //

    char *ext = (char*)glGetString( GL_EXTENSIONS );

    if( strstr( ext, "GL_ARB_multitexture" ) == NULL )
    {
        MessageBox(NULL,"GL_ARB_multitexture extension was not found",
            "ERROR",MB_OK|MB_ICONEXCLAMATION);
        return;
    }
    else
    {
        glActiveTexture       = (PFNGLCLIENTACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");
        glMultiTexCoord2f     = (PFNGLMULTITEXCOORD2FPROC)wglGetProcAddress("glMultiTexCoord2f");
        glClientActiveTexture = (PFNGLACTIVETEXTUREARBPROC)wglGetProcAddress("glClientActiveTexture");

        if( !glActiveTexture || !glMultiTexCoord2f || !glClientActiveTexture )
        {
            MessageBox(NULL,"One or more GL_ARB_multitexture functions were not found",
                "ERROR",MB_OK|MB_ICONEXCLAMATION);
            return;
        }
    }

    //
    // GL_EXT_texture_env_combine
    //

    if( strstr( ext, "GL_EXT_texture_env_combine" ) == NULL )
    {
        MessageBox(NULL,"GL_EXT_texture_env_combine extension was not found",
            "ERROR",MB_OK|MB_ICONEXCLAMATION);
        return;
    }

    //
    // For each vertex, create a tangent vector, binormal, and normal
    //

    // Initialize the inverse tangent matrix for each vertex to the identity 
    // matrix before we get started.
    for( int i = 0; i < NUM_VERTICES; ++i )
    {
        g_vTangents[i]  = vector3f( 1.0f, 0.0f, 0.0f );
        g_vBiNormals[i] = vector3f( 0.0f, 1.0f, 0.0f );
        g_vNormals[i]   = vector3f( 0.0f, 0.0f, 1.0f );
    }

    computeTangentsMatricesForEachVertex();

}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown(void) 
{
    glDeleteTextures( 1, &g_textureID );
    glDeleteTextures( 1, &g_normalmapTextureID );

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
// Name: computeTangentsMatricesForEachVertex()
// Desc: 
//-----------------------------------------------------------------------------
void computeTangentsMatricesForEachVertex(void)
{
    vector3f v1;
    vector3f v2;
    vector3f v3;
    vector3f vTangent;
    vector3f vBiNormal;
    vector3f vNormal;

    //
    // For each cube face defined in the vertex array, compute a tangent matrix 
    // for each of the four vertices that define it.
    //

    for( int i = 0; i < NUM_VERTICES; i += 4 ) // Use += 4 to process 1 face at a time
    {
        //
        // Vertex 0 of current cube face...
        //
        //  v2
        //    3----2
        //    |    |
        //    |    |
        //    0----1
        //  v1      v3
        //

        v1 = vector3f(g_cubeVertices[i].x,g_cubeVertices[i].y,g_cubeVertices[i].z);
        v2 = vector3f(g_cubeVertices[i+3].x,g_cubeVertices[i+3].y,g_cubeVertices[i+3].z);
        v3 = vector3f(g_cubeVertices[i+1].x,g_cubeVertices[i+1].y,g_cubeVertices[i+1].z);

        createTangentSpaceVectors( &v1,&v2,&v3,
                                   g_cubeVertices[i].tu, g_cubeVertices[i].tv,
                                   g_cubeVertices[i+3].tu, g_cubeVertices[i+3].tv,
                                   g_cubeVertices[i+1].tu, g_cubeVertices[i+1].tv,
                                   &vTangent, &vBiNormal, &vNormal );
        g_vTangents[i]  = vTangent;
        g_vBiNormals[i] = vBiNormal;
        g_vNormals[i]   = vNormal;

        //
        // Vertex 1 of current cube face...
        //
        //          v3
        //    3----2
        //    |    |
        //    |    |
        //    0----1
        //  v2      v1
        //

        v1 = vector3f(g_cubeVertices[i+1].x,g_cubeVertices[i+1].y,g_cubeVertices[i+1].z);
        v2 = vector3f(g_cubeVertices[i].x,g_cubeVertices[i].y,g_cubeVertices[i].z);
        v3 = vector3f(g_cubeVertices[i+2].x,g_cubeVertices[i+2].y,g_cubeVertices[i+2].z);

        createTangentSpaceVectors( &v1,&v2,&v3,
                                   g_cubeVertices[i+1].tu, g_cubeVertices[i+1].tv,
                                   g_cubeVertices[i].tu, g_cubeVertices[i].tv,
                                   g_cubeVertices[i+2].tu, g_cubeVertices[i+2].tv,
                                   &vTangent, &vBiNormal, &vNormal );

        g_vTangents[i+1]  = vTangent;
        g_vBiNormals[i+1] = vBiNormal;
        g_vNormals[i+1]   = vNormal;

        //
        // Vertex 2 of current cube face...
        //
        //  v3      v1
        //    3----2
        //    |    |
        //    |    |
        //    0----1
        //          v2
        //

        v1 = vector3f(g_cubeVertices[i+2].x,g_cubeVertices[i+2].y,g_cubeVertices[i+2].z);
        v2 = vector3f(g_cubeVertices[i+1].x,g_cubeVertices[i+1].y,g_cubeVertices[i+1].z);
        v3 = vector3f(g_cubeVertices[i+3].x,g_cubeVertices[i+3].y,g_cubeVertices[i+3].z);

        createTangentSpaceVectors( &v1,&v2,&v3,
                                   g_cubeVertices[i+2].tu, g_cubeVertices[i+2].tv,
                                   g_cubeVertices[i+1].tu, g_cubeVertices[i+1].tv,
                                   g_cubeVertices[i+3].tu, g_cubeVertices[i+3].tv,
                                   &vTangent, &vBiNormal, &vNormal );

        g_vTangents[i+2]  = vTangent;
        g_vBiNormals[i+2] = vBiNormal;
        g_vNormals[i+2]   = vNormal;

        //
        // Vertex 3 of current cube face...
        //
        //  v1      v2
        //    3----2
        //    |    |
        //    |    |
        //    0----1
        //  v3
        //

        v1 = vector3f(g_cubeVertices[i+3].x,g_cubeVertices[i+3].y,g_cubeVertices[i+3].z);
        v2 = vector3f(g_cubeVertices[i+2].x,g_cubeVertices[i+2].y,g_cubeVertices[i+2].z);
        v3 = vector3f(g_cubeVertices[i].x,g_cubeVertices[i].y,g_cubeVertices[i].z);

        createTangentSpaceVectors( &v1,&v2,&v3,
                                   g_cubeVertices[i+3].tu, g_cubeVertices[i+3].tv,
                                   g_cubeVertices[i+2].tu, g_cubeVertices[i+2].tv,
                                   g_cubeVertices[i].tu, g_cubeVertices[i].tv,
                                   &vTangent, &vBiNormal, &vNormal );

        g_vTangents[i+3]  = vTangent;
        g_vBiNormals[i+3] = vBiNormal;
        g_vNormals[i+3]   = vNormal;
    }
}

//-----------------------------------------------------------------------------
// Name: createTangentSpaceVectors()
// Desc: Given a vertex (v1) and two other vertices (v2 & v3) which define a 
//       triangle, this function will return Tangent, BiNormal, and Normal, 
//       vectors which can be used to define the tangent matrix for the first 
//       vertex's position (v1).
//
// Args: v1        - vertex 1
//       v2        - vertex 2
//       v3        - vertex 3
//       v1u, v1v  - texture-coordinates of vertex 1
//       v2u, v2v  - texture-coordinates of vertex 2
//       v3u, v3v  - texture-coordinates of vertex 3
//       vTangent  - When the function returns, this will be set as the tangent vector
//       vBiNormal - When the function returns, this will be set as the binormal vector
//       vNormal   - When the function returns, this will be set as the normal vector
//
// Note: This function is based on an article by By Jakob Gath and Sbren Dreijer.
// http://www.blacksmith-studios.dk/projects/downloads/tangent_matrix_derivation.php
//------------------------------------------------------------------------------

void createTangentSpaceVectors( vector3f *v1,
                                vector3f *v2,
                                vector3f *v3,
                                float v1u, float v1v,
                                float v2u, float v2v,
                                float v3u, float v3v,
                                vector3f *vTangent,
                                vector3f *vBiNormal,
                                vector3f *vNormal )
{
    // Create edge vectors from vertex 1 to vectors 2 and 3.
    vector3f vDirVec_v2_to_v1 = *v2 - *v1;
    vector3f vDirVec_v3_to_v1 = *v3 - *v1;

    // Create edge vectors from the texture coordinates of vertex 1 to vector 2.
    float vDirVec_v2u_to_v1u = v2u - v1u;
    float vDirVec_v2v_to_v1v = v2v - v1v;

    // Create edge vectors from the texture coordinates of vertex 1 to vector 3.
    float vDirVec_v3u_to_v1u = v3u - v1u;
    float vDirVec_v3v_to_v1v = v3v - v1v;

    float fDenominator = vDirVec_v2u_to_v1u * vDirVec_v3v_to_v1v - 
                         vDirVec_v3u_to_v1u * vDirVec_v2v_to_v1v;

    if( fDenominator < 0.0001f && fDenominator > -0.0001f )
    {
        // We're too close to zero and we're at risk of a divide-by-zero! 
        // Set the tangent matrix to the identity matrix and do nothing.
        *vTangent  = vector3f( 1.0f, 0.0f, 0.0f );
        *vBiNormal = vector3f( 0.0f, 1.0f, 0.0f );
        *vNormal   = vector3f( 0.0f, 0.0f, 1.0f );
    }
    else
    {
        // Calculate and cache the reciprocal value
        float fScale1 = 1.0f / fDenominator;

        vector3f T;
        vector3f B;
        vector3f N;

        T = vector3f((vDirVec_v3v_to_v1v * vDirVec_v2_to_v1.x - vDirVec_v2v_to_v1v * vDirVec_v3_to_v1.x) * fScale1,
                     (vDirVec_v3v_to_v1v * vDirVec_v2_to_v1.y - vDirVec_v2v_to_v1v * vDirVec_v3_to_v1.y) * fScale1,
                     (vDirVec_v3v_to_v1v * vDirVec_v2_to_v1.z - vDirVec_v2v_to_v1v * vDirVec_v3_to_v1.z) * fScale1);

        B = vector3f((-vDirVec_v3u_to_v1u * vDirVec_v2_to_v1.x + vDirVec_v2u_to_v1u * vDirVec_v3_to_v1.x) * fScale1,
                     (-vDirVec_v3u_to_v1u * vDirVec_v2_to_v1.y + vDirVec_v2u_to_v1u * vDirVec_v3_to_v1.y) * fScale1,
                     (-vDirVec_v3u_to_v1u * vDirVec_v2_to_v1.z + vDirVec_v2u_to_v1u * vDirVec_v3_to_v1.z) * fScale1);

        // The normal N is calculated as the cross product between T and B
        N = crossProduct( T, B );

        // Calculate and cache the reciprocal value
        float fScale2 = 1.0f / ((T.x * B.y * N.z - T.z * B.y * N.x) + 
                                (B.x * N.y * T.z - B.z * N.y * T.x) + 
                                (N.x * T.y * B.z - N.z * T.y * B.x));

        //
        // Use the temporary T (Tangent), (B) Binormal, and N (Normal) vectors 
        // to calculate the inverse of the tangent matrix that they represent.
        // The inverse of the tangent matrix is what we want since we need that
        // to transform the light's vector into tangent-space.
        //

        (*vTangent).x =   crossProduct( B, N ).x * fScale2;
        (*vTangent).y = -(crossProduct( N, T ).x * fScale2);
        (*vTangent).z =   crossProduct( T, B ).x * fScale2;
        (*vTangent).normalize();

        (*vBiNormal).x = -(crossProduct( B, N ).y * fScale2);
        (*vBiNormal).y =   crossProduct( N, T ).y * fScale2;
        (*vBiNormal).z = -(crossProduct( T, B ).y * fScale2);
        (*vBiNormal).normalize();

        (*vNormal).x =   crossProduct( B, N ).z * fScale2;
        (*vNormal).y = -(crossProduct( N, T ).z * fScale2);
        (*vNormal).z =   crossProduct( T, B ).z * fScale2;
        (*vNormal).normalize();

        //
        // NOTE: Since the texture-space of Direct3D and OpenGL are laid-out 
        //       differently, a single normal map can't look right in both 
        //       unless you make some adjustments somewhere.
        //
        //       You can adjust or fix this problem in three ways:
        //
        //       1. Create two normal maps: one for OpenGL and one for Direct3D.
        //       2. Flip the normal map image over as you load it into a texture 
        //          object.
        //       3. Flip the binormal over when computing the tangent-space
        //          matrix.
        //
        // Since the normal map used by this sample was created for Direct3D,
        // I've decided to simply flip the binormal.
        //
        *vBiNormal = *vBiNormal * -1.0f;
    }
}

//-----------------------------------------------------------------------------
// Name: scaleAndBiasVectorAsRGBColor()
// Desc: 
//-----------------------------------------------------------------------------
vector3f scaleAndBiasVectorAsRGBColor( vector3f* vVector )
{
    vector3f vScaledAndBiasedVector;

    vScaledAndBiasedVector.x = ((*vVector).x * 0.5f) + 0.5f;
    vScaledAndBiasedVector.y = ((*vVector).y * 0.5f) + 0.5f;
    vScaledAndBiasedVector.z = ((*vVector).z * 0.5f) + 0.5f;

    return vScaledAndBiasedVector;
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

    // Translate our view back a bit
    glTranslatef( 0.0f, 0.0f, g_fDistance );

    if( g_bMoveLightAbout == true )
    {
        //
        // Spin our point light around the cube...
        //

        static float fAngle = 0;
        fAngle += 60 * g_fElpasedTime;
        // Wrap it around, if it gets too big
        while(fAngle > 360.0f) fAngle -= 360.0f;
        while(fAngle < 0.0f)   fAngle += 360.0f;
        float x = sinf( DEGTORAD(fAngle) );
        float y = cosf( DEGTORAD(fAngle) );

        // The call to glLightfv is just like calling glTranslatef on our
        // light's position.
        g_lightPosition[0] = 1.2f * x;
        g_lightPosition[1] = 1.2f * y;
        g_lightPosition[2] = 2.0f;
        glLightfv( GL_LIGHT0, GL_POSITION, g_lightPosition );
    }

    // Spin the cube via mouse input.
    glRotatef( -g_fSpinY, 1.0f, 0.0f, 0.0f );
    glRotatef( -g_fSpinX, 0.0f, 1.0f, 0.0f );

    //
    // Set texture units to some known state...
    //

    glActiveTexture(GL_TEXTURE0);
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glDisable(GL_TEXTURE_3D_EXT);
    glDisable(GL_TEXTURE_CUBE_MAP_ARB);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_GEN_R);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glActiveTexture(GL_TEXTURE1);
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glDisable(GL_TEXTURE_3D_EXT);
    glDisable(GL_TEXTURE_CUBE_MAP_ARB);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_GEN_R);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    if( g_bToggleRegularLighting == true )
        glEnable( GL_LIGHTING );
    else
        glDisable( GL_LIGHTING );

    if( g_bDoDot3BumpMapping == true )
    {        
        //
        // Render a Dot3 bump mapped cube...
        //

        //
        // STAGE 0
        //
        // Use GL_DOT3_RGB_EXT to find the dot-product of (N.L), where N is 
        // stored in the normal map, and L is passed in as the PRIMARY_COLOR
        // using the standard glColor3f call.
        //

        glActiveTexture(GL_TEXTURE0);
        glBindTexture (GL_TEXTURE_2D, g_normalmapTextureID);
        glEnable(GL_TEXTURE_2D);

        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);      // Perform a Dot3 operation...
        glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_DOT3_RGB_EXT); 

        glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_TEXTURE);           // between the N (of N.L) which is stored in a normal map texture...
        glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);

        glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_PRIMARY_COLOR_EXT); // with the L (of N.L) which is stored in the vertex's diffuse color.
        glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_COLOR);

        //
        // STAGE 1
        //
        // Modulate the base texture by N.L calculated in STAGE 0.
        //

        glActiveTexture(GL_TEXTURE1);
        glBindTexture (GL_TEXTURE_2D, g_textureID);
        glEnable(GL_TEXTURE_2D);

        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);  // Modulate...
        glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE);

        glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_PREVIOUS_EXT);
        glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);    // the color argument passed down from the previous stage (stage 0) with...
        
        glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_TEXTURE);
        glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_COLOR);    // the texture for this stage with.

        //
        // Transform the light's position from eye-space to object-space
        //

        vector3f vLightPosES;    // Light position (in eye-space)
        vector3f vLightPosOS;    // Light position (in object-space)
        vector3f vVertToLightOS; // L vector of N.L (in object-space)
        vector3f vVertToLightTS; // L vector of N.L (in tangent-space)

        // Get the light's current position, which is in eye-space.
        float fLightsPosition[4];
        glGetLightfv( GL_LIGHT0, GL_POSITION, fLightsPosition );
        vLightPosES.x = fLightsPosition[0];
        vLightPosES.y = fLightsPosition[1];
        vLightPosES.z = fLightsPosition[2];

        // Transform the light's position from eye-space into object-space
        matrix4x4f modelViewMatrix;
        matrix4x4f modelViewMatrixInverse;
        glGetFloatv(GL_MODELVIEW_MATRIX, &modelViewMatrix.m[0] );
        modelViewMatrixInverse = matrix4x4f::invertMatrix( &modelViewMatrix );
        vLightPosOS = vLightPosES;
        modelViewMatrixInverse.transformPoint( &vLightPosOS );

        //
        // Now, render our textured test cube, which consists of 6 quads...
        //

        vector3f vCurrentVertex;

        glBegin( GL_QUADS );
        {
            for( int i = 0; i < NUM_VERTICES; ++i )
            {
                vCurrentVertex.x = g_cubeVertices[i].x;
                vCurrentVertex.y = g_cubeVertices[i].y;
                vCurrentVertex.z = g_cubeVertices[i].z;

                glMultiTexCoord2f( GL_TEXTURE0, g_cubeVertices[i].tu, g_cubeVertices[i].tv );
                glMultiTexCoord2f( GL_TEXTURE1, g_cubeVertices[i].tu, g_cubeVertices[i].tv );

                //
                // For each vertex, rotate L (of N.L) into tangent-space and 
                // pass it into OpenGL's texture blending system by packing it 
                // into the glVertex3f for that vertex.
                //

                vVertToLightOS = vLightPosOS - vCurrentVertex;
                vVertToLightOS.normalize();

                //
                // Build up an inverse tangent-space matrix using the Tangent, 
                // Binormal, and Normal calculated for the current vertex and 
                // then use it to transform our L vector (of N.L), which is in 
                // object-space, into tangent-space.
                //
                // A tangent matrix is of the form:
                //
                // |Tx Bx Nx 0|
                // |Ty By Ny 0|
                // |Tz Bz Nz 0|
                // |0  0  0  1|
                //
                // Note: Our vectors have already been inverted, so there is no 
                // need to invert our tangent matrix once we build it up.
                //

                //                         Tangent          Binormal           Normal
                matrix4x4f invTangentMatrix( g_vTangents[i].x, g_vBiNormals[i].x, g_vNormals[i].x, 0.0f,
                                             g_vTangents[i].y, g_vBiNormals[i].y, g_vNormals[i].y, 0.0f,
                                             g_vTangents[i].z, g_vBiNormals[i].z, g_vNormals[i].z, 0.0f,
                                             0.0f,             0.0f,              0.0f,            1.0f );

                vVertToLightTS = vVertToLightOS;
                invTangentMatrix.transformVector( &vVertToLightTS );

                //
                // Last but not least, we must scale and bias our L vector 
                // before passing it as a vertex color since colors have a 
                // clamped range of [0,1]. If we don't do this any negative 
                // components of our L vector will get clamped and the vector 
                // will be wrong. Of course, the hardware assumes that we are 
                // going to do this, so it will simply decode the original 
                // vector back out by reversing the scale and bias we've 
                // performed here.
                //

                vector3f vVertToLightTS_scaledAndBiased = scaleAndBiasVectorAsRGBColor( &vVertToLightTS );

                glColor3f( vVertToLightTS_scaledAndBiased.x,
                           vVertToLightTS_scaledAndBiased.y,
                           vVertToLightTS_scaledAndBiased.z );

                glNormal3f( g_cubeVertices[i].nx, g_cubeVertices[i].ny, g_cubeVertices[i].nz );
                glVertex3f( vCurrentVertex.x, vCurrentVertex.y, vCurrentVertex.z );
            }
        }
        glEnd();
    }
    else
    {        
        //
        // Render a regular textured cube with no Dot3 bump mapping...
        //

        //
        // STAGE 0
        //

        glActiveTexture(GL_TEXTURE0);
        glBindTexture (GL_TEXTURE_2D, g_textureID);
        glEnable(GL_TEXTURE_2D);

        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_REPLACE);   

        glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_TEXTURE);
        glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);

        //
        // STAGE 1
        //

        glActiveTexture(GL_TEXTURE1);
        glDisable(GL_TEXTURE_2D);

        glBegin( GL_QUADS );
        {
            for( int i = 0; i < NUM_VERTICES; ++i )
            {
                glMultiTexCoord2f( GL_TEXTURE0, g_cubeVertices[i].tu, g_cubeVertices[i].tv );
                glMultiTexCoord2f( GL_TEXTURE1, g_cubeVertices[i].tu, g_cubeVertices[i].tv );
                glColor3f( g_cubeVertices[i].r, g_cubeVertices[i].g, g_cubeVertices[i].b );
                glNormal3f( g_cubeVertices[i].nx, g_cubeVertices[i].ny, g_cubeVertices[i].nz );
                glVertex3f( g_cubeVertices[i].x, g_cubeVertices[i].y, g_cubeVertices[i].z );
            }
        }
        glEnd();
    }

    //
    // Reset texture unit state
    //

    glActiveTexture(GL_TEXTURE0);
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glDisable(GL_TEXTURE_3D_EXT);
    glDisable(GL_TEXTURE_CUBE_MAP_ARB);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_GEN_R);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glActiveTexture(GL_TEXTURE1);
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glDisable(GL_TEXTURE_3D_EXT);
    glDisable(GL_TEXTURE_CUBE_MAP_ARB);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_GEN_R);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    //
    // Render a small white sphere to mark the point light's position...
    //

    glDisable( GL_LIGHTING );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    glTranslatef( 0.0f, 0.0f, g_fDistance );
    glTranslatef( g_lightPosition[0], g_lightPosition[1], g_lightPosition[2] );

    glColor3f( 1.0f, 1.0f, 1.0f );
    renderSolidSphere( 0.05f, 8, 8 );

    glEnable( GL_LIGHTING );

    SwapBuffers( g_hDC );
}