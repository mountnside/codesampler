//-----------------------------------------------------------------------------
//           Name: ogl_solidnode_bsp.cpp
//         Author: Kevin Harris
//  Last Modified: 02/22/05
//    Description: A simple example of a "Solid Node Tree" style BSP compiler.
//
//           Note: This sample demonstrates how to code a BSP tree compiler 
//                 to sort polys into a back-to-front order. This technique 
//                 allows polys to be rendered in the proper order regardless 
//                 of the viewer's position, so polys further away don't end up 
//                 being rendered on top of a polys that are closer to the 
//                 viewer, which always results in strange graphical artifacts. 
//                 Of course, most people now use hardware accelerated 
//                 Z-Buffers that simply check the distance value of each 
//                 pixel before it gets added to the frame buffer and refuse 
//                 to overwrite any pixel that is closer to the viewer than 
//                 the new pixel value. This allows programmers to completely 
//                 skip the sorting process. I'm only revisiting this old 
//                 technique because this task is as simple as a BSP tree can 
//                 get while still doing something related to 3D graphics.
//
//       Controls: Up Arrow    = Moves view forward
//                 Down Arrow  = Moves view backwards
//                 Left Arrow  = View strafes to the left
//                 Right Arrow = View strafes to the right
//                 Home Key    = View gains altitude (disabled)
//                 End Key     = View loses altitude (disabled)
//                 Left Mouse  = Perform mouse-looking
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <mmsystem.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glaux.h>
#include "resource.h"
#include "matrix4x4f.h"
#include "vector3f.h"

//-----------------------------------------------------------------------------
// SYMBOLIC CONSTANTS
//-----------------------------------------------------------------------------
enum PointPosition
{
    POINTPOSITION_FRONT,   // Point is positioned in front of plane
    POINTPOSITION_BACK,    // Point is positioned behind plane
    POINTPOSITION_ONPLANE, // Point is positioned on plane
    POINTPOSITION_SPANNING // Point is spanning plane
};

//-----------------------------------------------------------------------------
// STRUCTS
//-----------------------------------------------------------------------------
struct VERTEX
{
    float tu;
    float tv;
    float r; 
    float g;
    float b; 
    float a;
    float x;
    float y;
    float z;
};

struct POLYGON // Polygon Structure
{
    VERTEX    VertexList[10];    // Actual Vertex Data
    vector3f  Normal;            // Polygon Normal
    int       nNumberOfVertices; // Number of Vertices
    int       nNumberOfIndices;  // Number of Indices
    WORD      Indices[30];       // Actual Index Data
    POLYGON  *Next;              // Linked List Next Poly
};

struct NODE // Node structure
{
    POLYGON *Splitter; // Splitter poly for this node
    NODE    *Front;    // Front Node Pointer
    NODE    *Back;     // Back Node Pointer
    bool     bIsLeaf;  // Is this a leaf node
    bool     bIsSolid; // Is this leaf node solid.
};

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND   g_hWnd = NULL;
HDC    g_hDC  = NULL;
HGLRC  g_hRC  = NULL;

GLuint g_wallTextureID = -1;

POINT  g_ptLastMousePosit;
POINT  g_ptCurrentMousePosit;
bool   g_bMousing = false;
float  g_fMoveSpeed = 25.0f;
float  g_fElpasedTime;
double g_dCurTime;
double g_dLastTime;

vector3f g_vLook(0.0f, 0.0f, 1.0f);         // Camera's look vector
vector3f g_vUp(0.0f, 1.0f, 0.0f);           // Camera's up vector
vector3f g_vRight(1.0f, 0.0f, 0.0f);        // Camera's right vector
vector3f g_vCameraPos(0.0f, 1.5f, 5.0f);    // Camera's current position
vector3f g_vOldCameraPos(0.0f, 0.0f, 0.0f); // Camera's old or cached Position 

POLYGON *g_polygonList;     // Top of polygon linked list
NODE    *g_BSPTreeRootNode; // BSP tree root node

//-----------------------------------------------------------------------------
// Notes concerning the global BSPMAP[]:
//
// The values contained in the array called BSPMAP will be used later to 
// create all of the polygons which we'll be using to test our Solid Node 
// BSP compiler. The layout of the array is intended to be viewed as a crude
// game level in which the viewer is looking down from above. Each number in 
// the array maps to the type of wall section that is going to be used 
// at that grid location on the maze floor.
//
// 0 =  Open space with no geometry
//      
//      |
// 1 =  |  OR ---- A normal wall section
//      |
//
//        /
// 2 =   /   A diagonal wall section
//      /
//
//      \ 
// 3 =   \   Same as 2 but oriented differently
//        \
//
//-----------------------------------------------------------------------------

//*
BYTE BSPMAP[] ={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,
                1,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1,
                1,0,0,3,0,0,0,1,0,0,0,0,0,0,0,0,2,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,
                1,1,1,1,1,1,1,1,1,1,1,2,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,1,1,1,1,1,1,0,0,0,0,0,1,1,1,0,0,1,1,1,
                1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,
                1,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,
                1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
//*/

/*
// This is highly chaotic maze with numreous twists and turns
BYTE BSPMAP[] ={0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                0,0,2,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,
                0,2,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,
                1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,1,
                0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,
                0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,
                0,1,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,3,1,
                0,2,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,
                1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,
                0,1,0,0,0,0,1,2,0,0,0,1,0,0,0,1,0,0,0,1,
                0,1,0,0,0,1,2,0,0,0,0,1,1,0,0,0,0,0,0,1,
                0,1,0,0,0,1,0,0,0,0,0,3,1,0,0,0,0,0,0,1,
                0,1,0,1,1,2,0,0,0,0,0,0,1,0,0,0,0,0,0,1,
                1,2,0,0,0,0,0,0,1,0,0,0,1,1,1,1,0,0,0,1,
                1,0,0,0,1,1,0,0,0,0,0,0,0,0,0,1,0,0,0,1,
                1,0,0,1,2,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,
                1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,
                1,1,1,1,1,0,1,0,0,0,0,0,1,1,1,1,1,1,1,1,
                0,0,0,1,1,0,1,0,0,0,0,1,1,1,1,1,1,1,1,1,
                0,0,2,0,0,0,1,0,0,1,0,3,0,0,0,0,0,0,0,0,
                0,2,0,0,0,0,1,0,0,1,0,0,3,0,0,0,0,0,0,0,
                1,0,0,0,0,0,1,0,0,1,0,0,0,1,1,1,1,1,0,1,
                0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1,
                0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,1,1,
                0,1,1,0,0,0,1,1,0,1,0,0,0,0,0,0,0,0,3,1,
                0,2,0,0,0,0,0,1,0,1,1,1,0,0,0,1,0,0,0,1,
                1,0,0,0,0,0,0,1,0,0,3,1,0,0,0,1,0,0,0,1,
                0,1,0,0,0,0,1,2,0,0,0,1,0,0,0,1,0,0,0,1,
                0,1,0,0,0,1,2,0,0,0,0,1,1,0,0,0,0,0,0,1,
                0,1,0,0,0,1,0,0,0,0,0,3,1,0,0,0,0,0,0,1,
                0,1,0,1,1,2,0,0,0,0,0,0,1,0,0,0,0,0,0,1,
                1,2,0,0,0,0,0,0,1,0,0,0,1,1,1,1,0,0,0,1,
                1,0,0,0,1,1,0,0,0,0,0,0,0,0,0,1,0,0,0,1,
                1,0,0,1,2,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,
                1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
//*/

/*
// This is blank maze with only a perimeter wall
BYTE BSPMAP[] ={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
//*/
            
//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void init(void);
void loadTexture(void);
void render(void);
void shutDown(void);
void getRealTimeUserInput(void);
void updateViewMatrix(void);
int classifyPoly(POLYGON *plane, POLYGON *Poly);
int classifyPoint(vector3f *pos, POLYGON *Plane);
void initLevelPolys(void);
void splitPolygon(POLYGON *Poly,POLYGON *Plane, POLYGON *FrontSplit, POLYGON *BackSplit);
void buildBspTree(NODE *CurrentNode, POLYGON *PolyList);
void walkBspTree(NODE *Node, vector3f *pos);
void deleteBSPTree(NODE *Node);
bool lineOfSight(vector3f *Start, vector3f *End, NODE *Node);
bool getIntersect(vector3f *linestart, vector3f *lineend, vector3f *vertex,
                  vector3f *normal, vector3f *intersection, float *percentage);
POLYGON *addPolygon(POLYGON *Parent, VERTEX *Vertices, WORD NumberOfVerts);
POLYGON *selectBestSplitter(POLYGON *PolyList);
VERTEX setVertex(float x, float y, float z,float r, float g, float b, float a,
                 float tu, float tv);

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
                             "OpenGL - Solid Node BSP Compiler",
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
        {
            g_dCurTime     = timeGetTime();
            g_fElpasedTime = (float)((g_dCurTime - g_dLastTime) * 0.001);
            g_dLastTime    = g_dCurTime;

            render();
        }
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
    vector3f tmpLook  = g_vLook;
    vector3f tmpRight = g_vRight;
    matrix4x4f mtxRot;

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
    AUX_RGBImageRec *pTextureImage = auxDIBImageLoad( ".\\wall.bmp" );

    if( pTextureImage != NULL )
    {
        glGenTextures( 1, &g_wallTextureID );

        glBindTexture(GL_TEXTURE_2D, g_wallTextureID);

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

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective( 45.0f, 640.0f / 480.0f, 0.1f, 100.0f);

    loadTexture();

    glClearColor( 0.350f, 0.530f, 0.701f, 1.0f );
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    initLevelPolys();

    // Once initLevelPolys() creates the required poly's for the scene 
    // geometry, we can compile the BSP tree by calling buildBspTree()...
    g_BSPTreeRootNode = new NODE;
    buildBspTree( g_BSPTreeRootNode, g_polygonList );
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )   
{
    glDeleteTextures( 1, &g_wallTextureID );

    deleteBSPTree( g_BSPTreeRootNode );
    
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
// Name : deleteBSPTree() (Recursive)
// Desc : Cleans up the BSP Tree after we have finished with it
//----------------------------------------------------------------------------- 
void deleteBSPTree( NODE *Node )
{
    if( Node->Back ) 
        deleteBSPTree(Node->Back);

    if( Node->Front ) 
        deleteBSPTree(Node->Front);

    if( Node->Splitter ) 
        delete Node->Splitter;

    delete Node;
}

//-----------------------------------------------------------------------------
// Name : setVertex 
// Desc : Given all the various vertex components, returns a valid VERTEX
//-----------------------------------------------------------------------------
VERTEX setVertex( float x,  float y, float z,
                  float r,  float g, float b, float a,
                  float tu, float tv )
{
    VERTEX vertex;

    vertex.tu = tu;
    vertex.tv = tv;
    vertex.r  = r;
    vertex.g  = g;
    vertex.b  = b;
    vertex.a  = a;
    vertex.x  = x;
    vertex.y  = y;
    vertex.z  = z;

    return vertex;
}

//-----------------------------------------------------------------------------
// Name : buildBspTree  (Recursive)
// Desc : Performs the actual BSP Compile when fed a polygon linked list and
//        a valid node (first time this is called this will be the parent of
//        the polygon linked list, and an already created root node.
//-----------------------------------------------------------------------------
void buildBspTree( NODE *CurrentNode,POLYGON *PolyList )
{
    POLYGON *polyTest    = NULL;
    POLYGON *FrontList   = NULL;
    POLYGON *BackList    = NULL;
    POLYGON *NextPolygon = NULL;
    POLYGON *FrontSplit  = NULL;
    POLYGON *BackSplit   = NULL;

    // Select the best splitter for this set of polygons
    CurrentNode->Splitter = selectBestSplitter(PolyList);
    
    polyTest = PolyList;

    // Begin the loop until we reach the end of the linked list.
    while( polyTest != NULL ) 
    {
        // Remember to store because polytest->Next could be altered
        NextPolygon = polyTest->Next;

        if( polyTest != CurrentNode->Splitter ) 
        {
            switch( classifyPoly( CurrentNode->Splitter, polyTest ) ) 
            {
                case POINTPOSITION_FRONT:
                    polyTest->Next = FrontList;
                    FrontList      = polyTest;      
                    break;

                case POINTPOSITION_BACK:
                    polyTest->Next = BackList;
                    BackList       = polyTest;  
                    break;

                case POINTPOSITION_SPANNING:
                    // Allocate two new polys for this fragment
                    FrontSplit = new POLYGON;
                    BackSplit  = new POLYGON;
                    ZeroMemory(FrontSplit,sizeof(POLYGON));
                    ZeroMemory(BackSplit,sizeof(POLYGON));
                    // Split the poly into two fragments
                    splitPolygon( polyTest, CurrentNode->Splitter, FrontSplit, BackSplit);
                    // Delete the original poly
                    delete polyTest;
                    // Reshuffle linked list
                    FrontSplit->Next = FrontList;
                    FrontList        = FrontSplit;
                    BackSplit->Next  = BackList;
                    BackList         = BackSplit;
                    break;

                default:
                    break;
            }
        }

        // Move onto the next polygon
        polyTest = NextPolygon;
    }

    // If there is nothing left in the front list then add an empty node here, 
    // otherwise carry on building the tree
    if( FrontList == NULL ) 
    {
        NODE *leafnode      = new NODE;
        ZeroMemory(leafnode,sizeof(leafnode));
        leafnode->bIsLeaf   = true;
        leafnode->bIsSolid  = false;    
        leafnode->Front     = NULL;
        leafnode->Back      = NULL;
        leafnode->Splitter  = NULL;
        CurrentNode->Front  = leafnode;
    } 
    else 
    {
        NODE *newnode       = new NODE;
        ZeroMemory(newnode,sizeof(newnode));
        newnode->bIsLeaf    = false;
        CurrentNode->Front  = newnode;
        buildBspTree(newnode,FrontList);
    } // End if frontlist is empty

    // If there is nothing left in the back list then add a solid node here, 
    // otherwise carry on building the tree
    if( BackList == NULL ) 
    {
        NODE *leafnode      = new NODE;
        ZeroMemory(leafnode,sizeof(leafnode));
        leafnode->bIsLeaf   = true;
        leafnode->bIsSolid  = true;
        leafnode->Front     = NULL;
        leafnode->Back      = NULL;
        leafnode->Splitter  = NULL;
        CurrentNode->Back   = leafnode;;
    }
    else 
    {
        NODE *newnode     = new NODE;
        ZeroMemory(newnode,sizeof(newnode));
        newnode->bIsLeaf  = false;
        CurrentNode->Back = newnode;
        buildBspTree(newnode,BackList);
    }
}

//-----------------------------------------------------------------------------
// Name : classifyPoly()
// Desc : Classifies a polygon against the plane passed
//-----------------------------------------------------------------------------
int classifyPoly( POLYGON *Plane, POLYGON *Poly )
{
    vector3f vec1( Plane->VertexList[0].x, Plane->VertexList[0].y, Plane->VertexList[0].z );
    int Infront = 0;
    int Behind  = 0;
    int OnPlane = 0;
    float result;
    
    // Loop round each of the vertices
    for( int i = 0; i < Poly->nNumberOfVertices; ++i ) 
    {
        vector3f vec2( Poly->VertexList[i].x, Poly->VertexList[i].y, Poly->VertexList[i].z );
        vector3f Direction = vec1 - vec2;
        result = dotProduct( Direction, Plane->Normal );

        // Tally up the position of each vertex
        if( result > 0.001f ) 
            ++Behind;
        else if( result < -0.001f ) 
            ++Infront; 
        else 
        {
            ++OnPlane;
            ++Infront;
            ++Behind;
        }
    }

    if( OnPlane == Poly->nNumberOfVertices )
        return POINTPOSITION_FRONT;

    if( Behind  == Poly->nNumberOfVertices )
        return POINTPOSITION_BACK;

    if( Infront == Poly->nNumberOfVertices )
        return POINTPOSITION_FRONT;

    return POINTPOSITION_SPANNING;
}

//-----------------------------------------------------------------------------
// Name : classifyPoint()
// Desc : Classifies a point against the plane passed
//-----------------------------------------------------------------------------
int classifyPoint( vector3f *pos, POLYGON *Plane )
{
    vector3f vec1( Plane->VertexList[0].x, Plane->VertexList[0].y, Plane->VertexList[0].z );
    vector3f Direction = vec1 - *pos;
    float    result = dotProduct( Direction, Plane->Normal );

    if( result < -0.001f ) 
        return POINTPOSITION_FRONT;

    if( result >  0.001f ) 
        return POINTPOSITION_BACK;

    return POINTPOSITION_ONPLANE;
}

//-----------------------------------------------------------------------------
// Name : walkBspTree() (Recursive)
// Desc : Performs a BSP Rendering Traversal
//-----------------------------------------------------------------------------
void walkBspTree( NODE *Node, vector3f *pos )
{
    if( Node->bIsLeaf == true )
        return;

    int result = classifyPoint( pos, Node->Splitter );
    
    if( result == POINTPOSITION_FRONT )
    {
        if( Node->Back  != NULL )
            walkBspTree( Node->Back, pos );

        glBegin( GL_TRIANGLES );
        {
            int nNumTris = Node->Splitter->nNumberOfIndices / 3;
            int v0;
            int v1;
            int v2;

            for( int i = 0; i < nNumTris; ++i )
            {
                v0 = Node->Splitter->Indices[i*3+0];
                v1 = Node->Splitter->Indices[i*3+1];
                v2 = Node->Splitter->Indices[i*3+2];

                glColor4f( Node->Splitter->VertexList[v0].r,
                           Node->Splitter->VertexList[v0].g,
                           Node->Splitter->VertexList[v0].b, 
                           Node->Splitter->VertexList[v0].a );
                
                glTexCoord2f( Node->Splitter->VertexList[v0].tu,
                              Node->Splitter->VertexList[v0].tv );

                glVertex3f( Node->Splitter->VertexList[v0].x,
                            Node->Splitter->VertexList[v0].y,
                            Node->Splitter->VertexList[v0].z );

                //---------------------------------------------

                glColor4f( Node->Splitter->VertexList[v1].r,
                           Node->Splitter->VertexList[v1].g,
                           Node->Splitter->VertexList[v1].b, 
                           Node->Splitter->VertexList[v1].a );

                glTexCoord2f( Node->Splitter->VertexList[v1].tu,
                              Node->Splitter->VertexList[v1].tv );

                glVertex3f( Node->Splitter->VertexList[v1].x,
                            Node->Splitter->VertexList[v1].y,
                            Node->Splitter->VertexList[v1].z );

                //----------------------------------------------

                glColor4f( Node->Splitter->VertexList[v2].r,
                           Node->Splitter->VertexList[v2].g,
                           Node->Splitter->VertexList[v2].b, 
                           Node->Splitter->VertexList[v2].a );
                
                glTexCoord2f( Node->Splitter->VertexList[v2].tu,
                              Node->Splitter->VertexList[v2].tv );

                glVertex3f( Node->Splitter->VertexList[v2].x,
                            Node->Splitter->VertexList[v2].y,
                            Node->Splitter->VertexList[v2].z );
            }
        }
        glEnd();

        if( Node->Front != NULL )
            walkBspTree( Node->Front, pos );

        return;
    } // End if camera is in front

    // This happens if we are at back or on plane
    if( Node->Front != NULL ) 
        walkBspTree( Node->Front, pos );

    if( Node->Back != NULL ) 
        walkBspTree( Node->Back,  pos );

    return;
}

//-----------------------------------------------------------------------------
// Name : initLevelPolys()
// Desc : Builds polygons in accordance to what's stored in the BSPMAP[] array.
//-----------------------------------------------------------------------------
void initLevelPolys( void )
{
    VERTEX   VERTLIST[4][4];
    POLYGON *child = NULL;
    int      direction[4];

    g_polygonList = NULL;

    for( int y = 0; y < 40; ++y )
    {
        for( int x = 0; x < 20; ++x )
        {
            ZeroMemory(direction, sizeof(int) * 4);
            
            int offset = (y * 20) + x;

            if( BSPMAP[offset] != 0 ) 
            {
                if( BSPMAP[offset] == 2 ) 
                {   
                    VERTLIST[0][0] = setVertex(x-10.5f,3.0f,(20.0f-y)-0.5f, 1.0f,1.0f,1.0f,1.0f, 1.0f,1.0f);
                    VERTLIST[0][1] = setVertex(x- 9.5f,3.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 0.0f,1.0f);
                    VERTLIST[0][2] = setVertex(x- 9.5f,0.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 0.0f,0.0f);
                    VERTLIST[0][3] = setVertex(x-10.5f,0.0f,(20.0f-y)-0.5f, 1.0f,1.0f,1.0f,1.0f, 1.0f,0.0f);
                    direction[0]   = 1;
                    
                    if( x > 0 ) 
                    {
                        if( BSPMAP[offset-1] == 0) 
                        {
                            VERTLIST[1][0] = setVertex(x-10.5f,3.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 1.0f,1.0f);
                            VERTLIST[1][1] = setVertex(x-10.5f,3.0f,(20.0f-y)-0.5f, 1.0f,1.0f,1.0f,1.0f, 0.0f,1.0f);
                            VERTLIST[1][2] = setVertex(x-10.5f,0.0f,(20.0f-y)-0.5f, 1.0f,1.0f,1.0f,1.0f, 0.0f,0.0f);
                            VERTLIST[1][3] = setVertex(x-10.5f,0.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 1.0f,0.0f);
                            direction[1]   = 1;
                        }
                    }

                    if( y > 0 ) 
                    {
                        if( BSPMAP[offset-20] == 0 ) 
                        {
                            VERTLIST[2][0] = setVertex(x- 9.5f,3.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 1.0f,1.0f);
                            VERTLIST[2][1] = setVertex(x-10.5f,3.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 0.0f,1.0f);
                            VERTLIST[2][2] = setVertex(x-10.5f,0.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 0.0f,0.0f);
                            VERTLIST[2][3] = setVertex(x- 9.5f,0.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 1.0f,0.0f);
                            direction[2]   = 1;
                        }
                    }
                }
                
                if( BSPMAP[offset] == 3 )
                {   
                    VERTLIST[0][0] = setVertex(x-10.5f,3.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 1.0f,1.0f);
                    VERTLIST[0][1] = setVertex(x- 9.5f,3.0f,(20.0f-y)-0.5f, 1.0f,1.0f,1.0f,1.0f, 0.0f,1.0f);
                    VERTLIST[0][2] = setVertex(x- 9.5f,0.0f,(20.0f-y)-0.5f, 1.0f,1.0f,1.0f,1.0f, 0.0f,0.0f);
                    VERTLIST[0][3] = setVertex(x-10.5f,0.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 1.0f,0.0f);
                    direction[0]   = 1;
    
                    if(x < 19)
                    {
                        if( BSPMAP[offset+1] == 0)
                        {
                            VERTLIST[1][0] = setVertex(x-9.5f,3.0f,(20.0f-y)-0.5f, 1.0f,1.0f,1.0f,1.0f, 1.0f,1.0f);
                            VERTLIST[1][1] = setVertex(x-9.5f,3.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 0.0f,1.0f);
                            VERTLIST[1][2] = setVertex(x-9.5f,0.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 0.0f,0.0f);
                            VERTLIST[1][3] = setVertex(x-9.5f,0.0f,(20.0f-y)-0.5f, 1.0f,1.0f,1.0f,1.0f, 1.0f,0.0f);
                            direction[1]   = 1;
                        }
                    }

                    if( y > 0 )
                    {
                        if( BSPMAP[offset-20] == 0 )
                        {
                            VERTLIST[2][0] = setVertex(x- 9.5f,3.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 1.0f,1.0f);
                            VERTLIST[2][1] = setVertex(x-10.5f,3.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 0.0f,1.0f);
                            VERTLIST[2][2] = setVertex(x-10.5f,0.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 0.0f,0.0f);
                            VERTLIST[2][3] = setVertex(x- 9.5f,0.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 1.0f,0.0f);
                            direction[2]   = 1;
                        }
                    }
                }               
                
                if( BSPMAP[offset] == 1 )
                {
                    if( x > 0 )
                    {
                        if( BSPMAP[offset-1] == 0 )
                        {
                            VERTLIST[0][0] = setVertex(x-10.5f,3.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 1.0f,1.0f);
                            VERTLIST[0][1] = setVertex(x-10.5f,3.0f,(20.0f-y)-0.5f, 1.0f,1.0f,1.0f,1.0f, 0.0f,1.0f);
                            VERTLIST[0][2] = setVertex(x-10.5f,0.0f,(20.0f-y)-0.5f, 1.0f,1.0f,1.0f,1.0f, 0.0f,0.0f);
                            VERTLIST[0][3] = setVertex(x-10.5f,0.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 1.0f,0.0f);
                            direction[0]   = 1;
                        }
                    }
                    
                    if (x < 19) 
                    {
                        if( BSPMAP[offset+1] == 0) 
                        {
                            VERTLIST[1][0] = setVertex(x-9.5f,3.0f,(20.0f-y)-0.5f, 1.0f,1.0f,1.0f,1.0f, 1.0f,1.0f);
                            VERTLIST[1][1] = setVertex(x-9.5f,3.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 0.0f,1.0f);
                            VERTLIST[1][2] = setVertex(x-9.5f,0.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 0.0f,0.0f);
                            VERTLIST[1][3] = setVertex(x-9.5f,0.0f,(20.0f-y)-0.5f, 1.0f,1.0f,1.0f,1.0f, 1.0f,0.0f);
                            direction[1]   = 1;
                        }
                    }
    
                    if( y > 0 ) 
                    {
                        if( BSPMAP[offset-20] == 0 ) 
                        {
                            VERTLIST[2][0] = setVertex(x- 9.5f,3.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 1.0f,1.0f);
                            VERTLIST[2][1] = setVertex(x-10.5f,3.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 0.0f,1.0f);
                            VERTLIST[2][2] = setVertex(x-10.5f,0.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 0.0f,0.0f);
                            VERTLIST[2][3] = setVertex(x- 9.5f,0.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 1.0f,0.0f);
                            direction[2]   = 1;
                        }
                    }

                    if( y < 39 )
                    {   
                        if( BSPMAP[offset+20] == 0)
                        {
                            VERTLIST[3][0] = setVertex(x-10.5f,3.0f,(20.0f-y)-0.5f, 1.0f,1.0f,1.0f,1.0f, 1.0f,1.0f);
                            VERTLIST[3][1] = setVertex(x- 9.5f,3.0f,(20.0f-y)-0.5f, 1.0f,1.0f,1.0f,1.0f, 0.0f,1.0f);
                            VERTLIST[3][2] = setVertex(x- 9.5f,0.0f,(20.0f-y)-0.5f, 1.0f,1.0f,1.0f,1.0f, 0.0f,0.0f);
                            VERTLIST[3][3] = setVertex(x-10.5f,0.0f,(20.0f-y)-0.5f, 1.0f,1.0f,1.0f,1.0f, 1.0f,0.0f);
                            direction[3]   = 1;
                        }
                    }
                }

                //-------------------------------------------------------------

                for( int a = 0; a < 4; ++a )
                {
                    if( direction[a] != 0 )
                    {
                        if( g_polygonList == NULL )
                        {
                            g_polygonList = addPolygon(NULL,&VERTLIST[a][0],4);
                            child = g_polygonList;
                        }
                        else
                        {
                            child = addPolygon(child,&VERTLIST[a][0],4);
                        }
                    }
                }
            }
        }
    }
}

//-----------------------------------------------------------------------------
// Name : getIntersect
// Desc : Get the ray / plane intersection details
//-----------------------------------------------------------------------------
bool getIntersect( vector3f *linestart, vector3f *lineend, 
                   vector3f *vertex, vector3f *normal, 
                   vector3f *intersection, float *percentage )
{
    vector3f direction;
    vector3f L1;
    float       linelength;
    float       dist_from_plane;

    direction.x = lineend->x - linestart->x;
    direction.y = lineend->y - linestart->y;
    direction.z = lineend->z - linestart->z;

    linelength = dotProduct( direction, *normal );
    
    if( fabsf( linelength ) < 0.001f ) 
        return false; 

    L1.x = vertex->x - linestart->x;
    L1.y = vertex->y - linestart->y;
    L1.z = vertex->z - linestart->z;

    dist_from_plane = dotProduct( L1, *normal );

    // How far from Linestart , intersection is as a percentage of 0 to 1 
    *percentage = dist_from_plane / linelength;

    // The ray does not reach, or is in front of the plane
    if( *percentage < 0.0f || *percentage > 1.0f )
        return false;

    // add the percentage of the line to line start
    intersection->x = linestart->x + direction.x * (*percentage);
    intersection->y = linestart->y + direction.y * (*percentage);
    intersection->z = linestart->z + direction.z * (*percentage);   
    return true;
}

//-----------------------------------------------------------------------------
// Name : splitPolygon
// Desc : This function is used to do ALL the clipping and Splitting of 
//        polygons. It takes a polygon and a Plane and splits 
//        the polygon into to two separate polygons. When used for clipping to 
//        a plane, call this function and then simply discard the front or
//        back depending on your needs.
// NOTE : FRONT and BACK MUST be valid pointers to empty Polygon structures as 
//        this function does NOT allocate the memory for them. The reason for
//        this is that this function is used in so many cases and some of them
//        required the Front and Back already be initialized. 
//-----------------------------------------------------------------------------
void splitPolygon( POLYGON *Poly, POLYGON *Plane, 
                   POLYGON *FrontSplit, POLYGON *BackSplit )
{
    // 50 is used here, as we should never really have more 
    // points on a portal than this.
    VERTEX FrontList[50];
    VERTEX BackList[50];
    int PointLocation[50];
    int CurrentVertex = 0;
    int FrontCounter  = 0;
    int BackCounter   = 0;
    int InFront       = 0;
    int Behind        = 0;
    int OnPlane       = 0;
    int Location      = 0;

    // Determine each points location relative to the plane.
    for( int i = 0; i < Poly->nNumberOfVertices; i++ )  
    {
        vector3f vTemp( Poly->VertexList[i].x,  Poly->VertexList[i].y, Poly->VertexList[i].z );
        Location = classifyPoint( &vTemp, Plane );

        if (Location == POINTPOSITION_FRONT )
            ++InFront;
        else if (Location == POINTPOSITION_BACK )
            ++Behind;
        else
            ++OnPlane;

        PointLocation[i] = Location;
    }
    
    // We set the VertexList[0] location again at the end
    // of the array so that we don't have to check and loop later
    //PointLocation[Poly->nNumberOfVertices] = PointLocation[0];

    if( !InFront ) 
    {
        memcpy(BackList, Poly->VertexList, Poly->nNumberOfVertices * sizeof(VERTEX));
        BackCounter = Poly->nNumberOfVertices;
    }

    if( !Behind ) 
    {
        memcpy(FrontList, Poly->VertexList, Poly->nNumberOfVertices * sizeof(VERTEX));
        FrontCounter = Poly->nNumberOfVertices;
    }

    if( InFront && Behind ) 
    {
        for( i = 0; i < Poly->nNumberOfVertices; ++i) 
        {
            // Store Current vertex remembering to MOD with number of vertices.
            CurrentVertex = (i+1) % Poly->nNumberOfVertices;

            if (PointLocation[i] == POINTPOSITION_ONPLANE ) 
            {
                FrontList[FrontCounter] = Poly->VertexList[i];
                ++FrontCounter;
                BackList[BackCounter] = Poly->VertexList[i];
                ++BackCounter;
                continue; // Skip to next vertex
            }
            if (PointLocation[i] == POINTPOSITION_FRONT ) 
            {
                FrontList[FrontCounter] = Poly->VertexList[i];
                ++FrontCounter;
            } 
            else 
            {
                BackList[BackCounter] = Poly->VertexList[i];
                ++BackCounter;
            }
            
            // If the next vertex is not causing us to span the plane then continue
            //if (PointLocation[i+1] == CP_ONPLANE || PointLocation[i+1] == PointLocation[i]) continue;
            if( PointLocation[CurrentVertex] == POINTPOSITION_ONPLANE || 
                PointLocation[CurrentVertex] == PointLocation[i] ) 
                continue;
            
            // Otherwise create the new vertex
            vector3f IntersectPoint;
            float       percent;

            vector3f vec0( Poly->VertexList[i].x,  Poly->VertexList[i].y, Poly->VertexList[i].z );
            vector3f vec1( Poly->VertexList[CurrentVertex].x,  Poly->VertexList[CurrentVertex].y, Poly->VertexList[CurrentVertex].z );
            vector3f vec2( Plane->VertexList[0].x,  Plane->VertexList[0].y, Plane->VertexList[0].z );
            getIntersect( &vec0, &vec1, &vec2, &Plane->Normal, &IntersectPoint, &percent );

            // create new vertex and calculate new texture coordinate
            VERTEX copy;
            float deltax = Poly->VertexList[CurrentVertex].tu - Poly->VertexList[i].tu;
            float deltay = Poly->VertexList[CurrentVertex].tv - Poly->VertexList[i].tv;
            float texx   = Poly->VertexList[i].tu + ( deltax * percent );
            float texy   = Poly->VertexList[i].tv + ( deltay * percent );
            copy.x       = IntersectPoint.x;
            copy.y       = IntersectPoint.y;
            copy.z       = IntersectPoint.z;
            copy.r       = Poly->VertexList[i].r;
            copy.g       = Poly->VertexList[i].g;
            copy.b       = Poly->VertexList[i].b;
            copy.a       = Poly->VertexList[i].a;
            copy.tu      = texx;
            copy.tv      = texy;

            BackList[BackCounter++]   = copy;
            FrontList[FrontCounter++] = copy;
        }
    }

    //OK THEN LETS BUILD THESE TWO POLYGONAL BAD BOYS

    FrontSplit->nNumberOfVertices = 0;
    BackSplit->nNumberOfVertices  = 0;

    // Copy over the vertices into the new polys
    FrontSplit->nNumberOfVertices = FrontCounter;
    memcpy(FrontSplit->VertexList, FrontList, FrontCounter * sizeof(VERTEX));
    BackSplit->nNumberOfVertices  = BackCounter;
    memcpy(BackSplit->VertexList, BackList, BackCounter * sizeof(VERTEX));

    BackSplit->nNumberOfIndices  = ( BackSplit->nNumberOfVertices  - 2 ) * 3;
    FrontSplit->nNumberOfIndices = ( FrontSplit->nNumberOfVertices - 2 ) * 3;

    // Fill in the Indices
    short IndxBase;
    
    for( short loop = 0, v1 = 1, v2 = 2; 
         loop < FrontSplit->nNumberOfIndices/3; 
         ++loop, v1 = v2, ++v2 )
    {
        IndxBase = loop * 3;
        FrontSplit->Indices[ IndxBase    ] =  0;
        FrontSplit->Indices[ IndxBase + 1] = v1;
        FrontSplit->Indices[ IndxBase + 2] = v2;
    } // Next Tri

    for( loop = 0, v1 = 1, v2 = 2; 
         loop < BackSplit->nNumberOfIndices/3; 
         ++loop, v1 = v2, ++v2 ) 
    {
        IndxBase = loop * 3;
        BackSplit->Indices[ IndxBase    ] =  0;
        BackSplit->Indices[ IndxBase + 1] = v1;
        BackSplit->Indices[ IndxBase + 2] = v2;
    } // Next Tri

    // Copy Extra Values
    FrontSplit->Normal = Poly->Normal;
    BackSplit->Normal  = Poly->Normal;
}

//-----------------------------------------------------------------------------
// Name : addPolygon
// Desc : Takes any convex Polygon and breaks in into multiple Indexed Triangle 
//        Lists and adds the polygon to a Linked list that will be sent to 
//        the BSP Compiler.
//-----------------------------------------------------------------------------
POLYGON *addPolygon( POLYGON *Parent, VERTEX *Vertices, WORD NOV )
{
    POLYGON *Child = new POLYGON;
    WORD v0;
    WORD v1;
    WORD v2;
    int  i = 0;

    Child->nNumberOfVertices = NOV;
    Child->nNumberOfIndices  = (NOV - 2) * 3;
    Child->Next              = NULL;

    // Copy Vertices
    for( i = 0; i < NOV; ++i )
        Child->VertexList[i] = Vertices[i];

    // Calculate indices
    for( i = 0; i < Child->nNumberOfIndices / 3; ++i )
    {
        if( i == 0 )
        {
            v0 = 0;
            v1 = 1;
            v2 = 2;
        }
        else
        {
            v1 = v2;
            ++v2;
        }
        
        Child->Indices[ i * 3     ] = v0;
        Child->Indices[(i * 3) + 1] = v1;
        Child->Indices[(i * 3) + 2] = v2;
    } // Next Tri

    // Generate polygon normal
    vector3f vec0( Child->VertexList[0].x, Child->VertexList[0].y, Child->VertexList[0].z );
    vector3f vec1( Child->VertexList[1].x, Child->VertexList[1].y, Child->VertexList[1].z );
    int j = Child->nNumberOfVertices-1;
    vector3f vec2( Child->VertexList[j].x, Child->VertexList[j].y, Child->VertexList[j].z ); // the last vert

    vector3f edge1 = vec1 - vec0;
    vector3f edge2 = vec2 - vec0;

    Child->Normal = crossProduct( edge1, edge2 );
    Child->Normal.normalize();

    if( Parent != NULL )
        Parent->Next=Child;

    return Child;
}

//-----------------------------------------------------------------------------
// Name : selectBestSplitter
// Desc : Selects the next best splitting plane from the poly linked list passed
//----------------------------------------------------------------------------- 
POLYGON * selectBestSplitter( POLYGON *PolyList )
{
    POLYGON *Splitter     = PolyList;
    POLYGON *CurrentPoly  = NULL;
    POLYGON *SelectedPoly = NULL; 
    ULONG    BestScore    = 100000;

    // Loop round all potential splitters
    while( Splitter != NULL ) 
    {
        CurrentPoly = PolyList;
        ULONG score      = 0;
        ULONG splits     = 0;
        ULONG backfaces  = 0;
        ULONG frontfaces = 0;
        
        // Loop round all polys testing split counts etc
        while( CurrentPoly != NULL ) 
        {
            if( CurrentPoly != Splitter ) 
            {
                int result = classifyPoly(Splitter, CurrentPoly);

                switch( result ) 
                {
                    case POINTPOSITION_ONPLANE:
                        break;
                    case POINTPOSITION_FRONT:
                        ++frontfaces;
                        break;
                    case POINTPOSITION_BACK:
                        ++backfaces;
                        break;
                    case POINTPOSITION_SPANNING:
                        ++splits;
                        break;
                    default:
                        break;
                }
            }
            CurrentPoly = CurrentPoly->Next;
        }
     
        // Calculate Score
        score = abs( (int)(frontfaces-backfaces) ) + (splits * 4) ;

        // Compare scores
        if( score < BestScore ) 
        {
            BestScore    = score;
            SelectedPoly = Splitter;
        }
  
        Splitter = Splitter->Next;
    }
    return SelectedPoly;
}
   
//-----------------------------------------------------------------------------
// Name : lineOfSight() (Recursive)
// Desc : Tests line of sight between two points.
//----------------------------------------------------------------------------- 
bool lineOfSight( vector3f *Start, vector3f *End, NODE *Node )
{
    vector3f intersection;
    float    fTemp = 0.0f;

    if( Node->bIsLeaf == true )
        return !Node->bIsSolid;

    int PointA = classifyPoint(Start, Node->Splitter);
    int PointB = classifyPoint(End,   Node->Splitter);

    if( PointA == POINTPOSITION_ONPLANE && PointB == POINTPOSITION_ONPLANE ) 
        return lineOfSight(Start,End,Node->Front);

    if( PointA == POINTPOSITION_FRONT && PointB == POINTPOSITION_BACK ) 
    {
        vector3f vTemp(Node->Splitter->VertexList[0].x, Node->Splitter->VertexList[0].y, Node->Splitter->VertexList[0].z);
        getIntersect( Start, End, &vTemp, &Node->Splitter->Normal, &intersection, &fTemp);
        return lineOfSight( Start, &intersection, Node->Front) && lineOfSight( End, &intersection, Node->Back); 
    }

    if( PointA == POINTPOSITION_BACK && PointB == POINTPOSITION_FRONT ) 
    {
        vector3f vTemp(Node->Splitter->VertexList[0].x, Node->Splitter->VertexList[0].y, Node->Splitter->VertexList[0].z);
        getIntersect( Start, End, &vTemp, &Node->Splitter->Normal, &intersection, &fTemp);
        return lineOfSight( End, &intersection, Node->Front) && lineOfSight( Start, &intersection, Node->Back);
    }

    // If we get here one of the points is on the plane
    if( PointA == POINTPOSITION_FRONT || PointB == POINTPOSITION_FRONT ) 
        return lineOfSight( Start, End, Node->Front );
    else 
        return lineOfSight( Start, End, Node->Back );

    return true;
}

//-----------------------------------------------------------------------------
// Name: getRealTimeUserInput()
// Desc: 
//-----------------------------------------------------------------------------
void getRealTimeUserInput( void )
{
    g_vOldCameraPos = g_vCameraPos;

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
        g_vCameraPos -= tmpLook*-g_fMoveSpeed*g_fElpasedTime;

    // Down Arrow Key - View moves backward
    if( keys[VK_DOWN] & 0x80 )
        g_vCameraPos += (tmpLook*-g_fMoveSpeed)*g_fElpasedTime;

    // Left Arrow Key - View side-steps or strafes to the left
    if( keys[VK_LEFT] & 0x80 )
        g_vCameraPos -= (tmpRight*g_fMoveSpeed)*g_fElpasedTime;

    // Right Arrow Key - View side-steps or strafes to the right
    if( keys[VK_RIGHT] & 0x80 )
        g_vCameraPos += (tmpRight*g_fMoveSpeed)*g_fElpasedTime;

    // Home Key - View elevates up
    if( keys[VK_HOME] & 0x80 )
        g_vCameraPos.y += g_fMoveSpeed*g_fElpasedTime; 

    // End Key - View elevates down
    if( keys[VK_END] & 0x80 )
        g_vCameraPos.y -= g_fMoveSpeed*g_fElpasedTime;
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

    view.m[12] = -dotProduct(g_vRight, g_vCameraPos);
    view.m[13] = -dotProduct(g_vUp, g_vCameraPos);
    view.m[14] =  dotProduct(g_vLook, g_vCameraPos);
    view.m[15] =  1.0f;

    glMultMatrixf( view.m );
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: 
//-----------------------------------------------------------------------------
void render( void )
{
    getRealTimeUserInput();

    // If the BSP tree can't find a line-of-sight between the player's old 
    // position and its new position, the player's has moved through a maze
    // wall and must be moved back to prevent escape!
    if( lineOfSight( &g_vCameraPos, &g_vOldCameraPos, g_BSPTreeRootNode ) == false )
        g_vCameraPos = g_vOldCameraPos; // Back ya go!

    // Also, restrict player movement by locking the view to the floor. 
    // In other words, don't let the player fly around and leave the maze.
    g_vCameraPos.y = 1.5f;

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    updateViewMatrix();

    glBindTexture( GL_TEXTURE_2D, g_wallTextureID );

    // Walk and Render all the polygons in the tree
    walkBspTree( g_BSPTreeRootNode, &g_vCameraPos );

    SwapBuffers( g_hDC );
}