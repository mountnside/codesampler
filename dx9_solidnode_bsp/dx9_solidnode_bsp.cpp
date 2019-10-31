//-----------------------------------------------------------------------------
//           Name: dx9_solidnode_bsp.cpp
//         Author: Kevin Harris (based on a course by Adam Hoult & Gary Simmons)
//  Last Modified: 02/01/05
//    Description: A simple example of a "Solid Node Tree" style BSP compiler.
//
//                 This sample is based on one of the BSP demos that came with 
//                 the "Advanced 3D BSP, PVS and CSG Techniques" course offered
//                 at www.gameinstite.com. If you're truly interested in learning 
//                 about BSP trees and how they're used in games, this course 
//                 is the best by far and I highly recommend it.
//                 
// https://www.gameinstitute.com/gi/courses/coursedescription.asp?courseID=8
//
//    Please Note: This sample demonstrates how to code a BSP tree compiler 
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
#define DIRECTINPUT_VERSION 0x0800

#include <windows.h>
#include <mmsystem.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <dinput.h>
#include "resource.h"

//-----------------------------------------------------------------------------
// MACROS
//-----------------------------------------------------------------------------
#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }
#define KEYDOWN(name,key) (name[key] & 0x80)

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

#define D3DFVF_VERTEX ( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 )

struct VERTEX // Vertex Structure
{
    float    x;    // X Coordinate Component
    float    y;    // Y Coordinate Component
    float    z;    // Z Coordinate Component
    D3DCOLOR rgba; // Diffuse Colour Component
    float    tu;   // U Texture Component
    float    tv;   // V Texture Component
};

struct POLYGON // Polygon Structure
{
    VERTEX       VertexList[10];    // Actual Vertex Data
    D3DXVECTOR3  Normal;            // Polygon Normal
    int          nNumberOfVertices; // Number of Vertices
    int          nNumberOfIndices;  // Number of Indices
    WORD         Indices[30];       // Actual Index Data
    POLYGON     *Next;              // Linked List Next Poly
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
LPDIRECT3D9          g_pD3D       = NULL; // Used to create the D3DDevice
LPDIRECT3DDEVICE9    g_pd3dDevice = NULL; // Our rendering device
LPDIRECT3DTEXTURE9   g_lpTexture  = NULL; // Direct3D Texture to store wall texture
LPDIRECTINPUT8       g_lpdi       = NULL; // DirectInput object
LPDIRECTINPUTDEVICE8 g_pKeyboard  = NULL; // DirectInput device
LPDIRECTINPUTDEVICE8 g_pMouse     = NULL; // DirectInput device

float  g_fMoveSpeed = 5.0f;
float  g_fLookSpeed = 0.0f;
float  g_fPitch     = 0.0f;
float  g_fElpasedTime;
double g_dCurTime;
double g_dLastTime;
D3DXVECTOR3 g_vLook(0.0f, 0.0f, 1.0f);      // Camera Look Vector
D3DXVECTOR3 g_vUp(0.0f, 1.0f, 0.0f);        // Camera Up Vector
D3DXVECTOR3 g_vRight(1.0f, 0.0f, 0.0f);     // Camera Right Vector
D3DXVECTOR3 g_vCameraPos(0.0f, 1.5f, 5.0f); // Camera Position

POLYGON *g_polygonList;     // Top of polygon linked list
NODE    *g_BSPTreeRootNode; // BSP tree root node

//-----------------------------------------------------------------------------
// Notes concerning the global BSPMAP[]:
//
// The values contained in the array called BSPMAP will be used later to 
// create all of the polygons which we'll be using to test our Solid Node 
// BSP compiler. The layout of the array is intended to be viewed as a crude
// game level in which the viewer is lookinng down from above. Each number in 
// the array maps to the type of wall section that is going to be used 
// at that grid location on the maze floor.
//
// 0 =  Open space with no geometry
//      
//      |
// 1 =  |  OR ---- A normal wall section
//      |
//       
//          /
// 2 =    /   A diagonal wall section
//      /
//       
//      \ 
// 3 =    \   Same as 2 but oriented differently
//          \
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
// This is highly chaotic maze with numeous twists and turns
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
// This is blank maze with only a perimiter wall
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
// Function Declarations
//-----------------------------------------------------------------------------
LRESULT CALLBACK WindowProc(HWND hWnd, unsigned uMsg, WPARAM wParam, LPARAM lParam);
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
HRESULT initD3D(HWND hWnd);
void shutDownD3D(void);
HRESULT initDInput(HWND hWnd);
void shutDownDInput(void);
void loadTexture(void);
HRESULT readUserInput(void);
void update(void);
void updateViewMatrix(void);
void render(void);
int classifyPoly(POLYGON *plane, POLYGON *Poly);
int classifyPoint(D3DXVECTOR3 *pos, POLYGON *Plane);
void initLevelPolys(void);
void splitPolygon(POLYGON *Poly,POLYGON *Plane, POLYGON *FrontSplit, POLYGON *BackSplit);
void buildBspTree(NODE *CurrentNode, POLYGON *PolyList);
void walkBspTree(NODE *Node, D3DXVECTOR3 *pos);
void deleteBSPTree(NODE *Node);
bool lineOfSight(D3DXVECTOR3 *Start, D3DXVECTOR3 *End, NODE *Node);
bool getIntersect(D3DXVECTOR3 *linestart, D3DXVECTOR3 *lineend, D3DXVECTOR3 *vertex,
                  D3DXVECTOR3 *normal, D3DXVECTOR3 *intersection, float *percentage);
POLYGON *addPolygon(POLYGON *Parent, VERTEX *Vertices, WORD NumberOfVerts);
POLYGON *selectBestSplitter(POLYGON *PolyList);
VERTEX setVertex(float x, float y, float z,float r, float g, float b, float a,
                 float tu, float tv);

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
            }
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
// Name: WinMain()
// Desc: The application's entry point
//-----------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR     lpCmdLine,
                    int       nCmdShow )
{
    WNDCLASSEX winClass;
    HWND       hWnd;
    MSG        uMsg;

    memset(&uMsg,0,sizeof(uMsg));

    winClass.lpszClassName = "MY_WINDOWS_CLASS";
    winClass.cbSize        = sizeof(WNDCLASSEX);
    winClass.style         = CS_DBLCLKS | CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    winClass.lpfnWndProc   = WindowProc;
    winClass.hInstance     = hInstance;
    winClass.hIcon         = LoadIcon(hInstance, (LPCTSTR)IDI_DIRECTX_ICON);
    winClass.hIconSm       = LoadIcon(hInstance, (LPCTSTR)IDI_DIRECTX_ICON);
    winClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    winClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    winClass.lpszMenuName  = NULL;
    winClass.cbClsExtra    = 0;
    winClass.cbWndExtra    = 0;

    if( !RegisterClassEx(&winClass) )
        return E_FAIL;

    hWnd = CreateWindowEx( NULL, "MY_WINDOWS_CLASS",
                           "Direct3D (DX9) - Solid Node BSP Tree",
                           WS_OVERLAPPEDWINDOW,
                           0, 0, 640, 480, NULL, NULL, hInstance, NULL );

    if( hWnd == NULL )
        return E_FAIL;
    
    ShowWindow( hWnd, nCmdShow );
    UpdateWindow( hWnd );

    initD3D( hWnd );
    initDInput( hWnd );
    initLevelPolys();

    // Once initLevelPolys() creates the required poly's for the scene 
    // geometry, we can compile the BSP tree by calling buildBspTree()...
    g_BSPTreeRootNode = new NODE;
    buildBspTree( g_BSPTreeRootNode, g_polygonList );

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

    shutDownD3D();
    shutDownDInput();
    deleteBSPTree( g_BSPTreeRootNode );

    UnregisterClass( "MY_WINDOWS_CLASS", winClass.hInstance );

    return uMsg.wParam;
}

//-----------------------------------------------------------------------------
// Name: loadTexture()
// Desc: 
//-----------------------------------------------------------------------------
void loadTexture(void)  
{
    // Set some texturing properties...
    D3DXCreateTextureFromFileA( g_pd3dDevice, "wall.bmp", &g_lpTexture );
    g_pd3dDevice->SetTexture( 0, g_lpTexture );
    float fBias = -1.5;
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MIPMAPLODBIAS, *((LPDWORD)(&fBias)) );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
    g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );
}

//-----------------------------------------------------------------------------
// Name : initD3D 
// Desc : 
//-----------------------------------------------------------------------------
HRESULT initD3D( HWND hWnd )
{
    if( NULL == ( g_pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) )
    {
        MessageBox(NULL,"Couldn't create the Direct3D object!",
            "ERROR",MB_OK|MB_ICONEXCLAMATION);
        return E_FAIL;
    }

    D3DDISPLAYMODE d3ddm;

    if( FAILED( g_pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm ) ) )
    {
        MessageBox(NULL,"Couldn't get the adapter display mode!",
            "ERROR",MB_OK|MB_ICONEXCLAMATION);
        return E_FAIL;
    }

    D3DPRESENT_PARAMETERS d3dpp;

    ZeroMemory( &d3dpp, sizeof(d3dpp) );
    d3dpp.Windowed               = TRUE;
    d3dpp.SwapEffect             = D3DSWAPEFFECT_FLIP;
    d3dpp.BackBufferFormat       = d3ddm.Format;
    d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_IMMEDIATE;
    d3dpp.EnableAutoDepthStencil = FALSE; // Turn off the depth buffer so we can 
                                          // test our back-to-front sorting with
                                          // our super cool BSP tree!

    if( FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
                                      D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                      &d3dpp, &g_pd3dDevice ) ) )
    {
        MessageBox(NULL,"Couldn't create the Direct3D device!",
            "ERROR",MB_OK|MB_ICONEXCLAMATION);
        return E_FAIL;
    }

    // Set some general properties...
    g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, false ); // we are using prelit vertices
    D3DXMATRIX proj_m;
    D3DXMatrixPerspectiveFovLH( &proj_m, D3DXToRadian( 45.0f ), 
                                640 / 480, 0.1f, 1000.0f );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &proj_m );

    loadTexture();

    // Since we have only one source for geometry in this sample, we'll 
    // just set it now and not worry about during our calls to render()...
    g_pd3dDevice->SetFVF( D3DFVF_VERTEX );

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: shutDownD3D()
// Desc: 
//-----------------------------------------------------------------------------
void shutDownD3D(void)
{
    SAFE_RELEASE( g_lpTexture );
    SAFE_RELEASE( g_pd3dDevice );
    SAFE_RELEASE( g_pD3D );
}

//-----------------------------------------------------------------------------
// Name: initDInput()
// Desc: Creates a keyboard and mouse device using DirectInput
//-----------------------------------------------------------------------------
HRESULT initDInput( HWND hWnd )
{
    HRESULT hr;

    shutDownDInput();

    if( FAILED( hr = DirectInput8Create( GetModuleHandle(NULL), DIRECTINPUT_VERSION, 
                                         IID_IDirectInput8, (VOID**)&g_lpdi, NULL ) ) )
    {
        MessageBox(NULL,"Couldn't create a DirectInput object!",
            "ERROR",MB_OK|MB_ICONEXCLAMATION);
        return hr;
    }
    
    //
    // Create a keyboard device...
    //

    if( FAILED( hr = g_lpdi->CreateDevice( GUID_SysKeyboard, &g_pKeyboard, NULL ) ) )
    {
        MessageBox(NULL,"Couldn't create a keyboard device!",
            "ERROR",MB_OK|MB_ICONEXCLAMATION);
        return hr;
    }
    
    if( FAILED( hr = g_pKeyboard->SetDataFormat( &c_dfDIKeyboard ) ) )
    {
        MessageBox(NULL,"Couldn't set the data format for a keyboard!",
            "ERROR",MB_OK|MB_ICONEXCLAMATION);
        return hr;
    }
    
    hr = g_pKeyboard->SetCooperativeLevel( hWnd, DISCL_NOWINKEY | 
                                                 DISCL_NONEXCLUSIVE | 
                                                 DISCL_FOREGROUND );

    if( hr == DIERR_UNSUPPORTED )
    {
        shutDownDInput();
        MessageBox(NULL, "SetCooperativeLevel() returned DIERR_UNSUPPORTED!\n"
                   "For security reasons, background exclusive keyboard\n"
                   "access is not allowed.", "ERROR", MB_OK|MB_ICONEXCLAMATION );
        return S_OK;
    }
    else if( FAILED(hr) )
    {
        MessageBox(NULL,"Couldn't set the cooperative level requested for the keyboard!",
            "ERROR",MB_OK|MB_ICONEXCLAMATION);
        return hr;
    }

    g_pKeyboard->Acquire();

    //
    // Create a mouse device...
    //

    if( FAILED( hr = g_lpdi->CreateDevice( GUID_SysMouse, &g_pMouse, NULL ) ) )
    {
        MessageBox(NULL,"Couldn't create a mouse device!",
            "ERROR",MB_OK|MB_ICONEXCLAMATION);
        return hr;
    }
    
    if( FAILED( hr = g_pMouse->SetDataFormat( &c_dfDIMouse2 ) ) )
    {
        MessageBox(NULL,"Couldn't set the data format for a mouse!",
            "ERROR",MB_OK|MB_ICONEXCLAMATION);
        return hr;
    }
    
    if( FAILED( hr = g_pMouse->SetCooperativeLevel( hWnd, 
                                    DISCL_NONEXCLUSIVE | DISCL_FOREGROUND ) ) )
    {
        MessageBox(NULL,"Couldn't set the cooperative level requested for the mouse!",
            "ERROR",MB_OK|MB_ICONEXCLAMATION);
        return hr;
    }

    g_pMouse->Acquire();

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: shutDownDInput()
// Desc: Initialize the DirectInput variables.
//-----------------------------------------------------------------------------
void shutDownDInput(void)
{
    if( g_pKeyboard ) 
        g_pKeyboard->Unacquire();

    if( g_pMouse )
        g_pMouse->Unacquire();
    
    SAFE_RELEASE( g_pMouse );
    SAFE_RELEASE( g_pKeyboard );
    SAFE_RELEASE( g_lpdi );
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

    vertex.x    = x;
    vertex.y    = y;
    vertex.z    = z;
    vertex.rgba = D3DCOLOR_COLORVALUE(r,g,b,a);
    vertex.tu   = tu;
    vertex.tv   = tv;
    
    return vertex;
}

//-----------------------------------------------------------------------------
// Name : render()
// Desc : 
//-----------------------------------------------------------------------------
void render( void )
{
    D3DXVECTOR3 vOldCameraPos = g_vCameraPos;

    readUserInput();

    // If the BSP tree can't find a line-of-sight between the player's old 
    // position and its new position, the player's has moved through a maze
    // wall and must be moved back to prevent escape!
    if( lineOfSight( &g_vCameraPos, &vOldCameraPos, g_BSPTreeRootNode ) == false )
        g_vCameraPos = vOldCameraPos; // Back ya go!

    // Also, restrict player movement by locking the view to the floor. 
    // In other words, don't let the player fly around and leave the maze.
    g_vCameraPos.y = 1.5f;

    g_pd3dDevice->Clear( 0, NULL,D3DCLEAR_TARGET,
                         D3DCOLOR_COLORVALUE( 0.350f, 0.530f, 0.701, 1.0f ),
                         1.0f, 0 );

    g_pd3dDevice->BeginScene();

    updateViewMatrix();

    // Walk and Render all the polygons in the tree
    walkBspTree( g_BSPTreeRootNode, (D3DXVECTOR3*)&g_vCameraPos );

    g_pd3dDevice->EndScene();
    g_pd3dDevice->Present(NULL,NULL,NULL,NULL);
}

//-----------------------------------------------------------------------------
// Name: readUserInput()
// Desc: Reads user input and modifies a set of globals that will later be 
//       used to build the camera's current view.
//-----------------------------------------------------------------------------
HRESULT readUserInput( void )
{
    DIMOUSESTATE2 dims;
    char buffer[256];
    HRESULT hr;

    ZeroMemory( &dims, sizeof(dims) );

    if( FAILED( hr = g_pMouse->GetDeviceState( sizeof(DIMOUSESTATE2), &dims ) ) )
    {
        // If input is lost then acquire and keep trying
        hr = g_pMouse->Acquire();
        while( hr == DIERR_INPUTLOST )
            hr = g_pMouse->Acquire();
        return S_OK;
    }

    if( FAILED( hr = g_pKeyboard->GetDeviceState(sizeof(buffer),(LPVOID)&buffer) ) )
        return hr;

    D3DXMATRIX mtxRot;

    // If the left mouse button is down - do mouse looking
    if( dims.rgbButtons[0] & 0x80 )
    {
        if( dims.lY != 0 )
        {
            float g_fPitchAmount = ((float)dims.lY / 3.0f);
            g_fPitch += g_fPitchAmount;

            D3DXMatrixRotationAxis( &mtxRot, &g_vRight, D3DXToRadian(g_fPitchAmount));
            D3DXVec3TransformCoord( &g_vLook, &g_vLook, &mtxRot );
            D3DXVec3TransformCoord( &g_vUp, &g_vUp, &mtxRot );
        }

        if( dims.lX != 0 )
        {
            D3DXMatrixRotationAxis( &mtxRot, &D3DXVECTOR3(0,1,0), D3DXToRadian((float)dims.lX / 3.0f) );
            D3DXVec3TransformCoord( &g_vLook, &g_vLook, &mtxRot );
            D3DXVec3TransformCoord( &g_vUp, &g_vUp, &mtxRot );
        }
    }

    D3DXVECTOR3 tmpLook  = g_vLook;
    D3DXVECTOR3 tmpRight = g_vRight;

    // View moves forward
    if( KEYDOWN(buffer, DIK_UP) )
        g_vCameraPos += (g_fMoveSpeed*tmpLook)*g_fElpasedTime;

    // View moves backward
    if( KEYDOWN(buffer, DIK_DOWN) )
        g_vCameraPos -= (g_fMoveSpeed*tmpLook)*g_fElpasedTime;

    // View side-steps or strafes to the left
    if( KEYDOWN(buffer, DIK_LEFT) )
        g_vCameraPos -= (g_fMoveSpeed*tmpRight)*g_fElpasedTime;

    // View side-steps or strafes to the right
    if( KEYDOWN(buffer, DIK_RIGHT) )
        g_vCameraPos += (g_fMoveSpeed*tmpRight)*g_fElpasedTime;

    // View elevates up
    if( KEYDOWN(buffer, DIK_HOME) )
        g_vCameraPos.y += g_fMoveSpeed*g_fElpasedTime; 
    
    // View elevates down
    if( KEYDOWN(buffer, DIK_END) )
        g_vCameraPos.y -= g_fMoveSpeed*g_fElpasedTime;

    return S_OK;
}
  
//-----------------------------------------------------------------------------
// Name : updateViewMatrixPos ()
// Desc : Updates our camera position and matrices.
//-----------------------------------------------------------------------------
void updateViewMatrix()
{  
    D3DXMATRIX view;
    D3DXMatrixIdentity(&view);

    // Vector regeneration
    D3DXVec3Normalize( &g_vLook, &g_vLook );
    D3DXVec3Cross( &g_vRight, &g_vUp, &g_vLook );
    D3DXVec3Normalize( &g_vRight, &g_vRight );
    D3DXVec3Cross( &g_vUp, &g_vLook, &g_vRight );
    D3DXVec3Normalize( &g_vUp, &g_vUp );

    // Build the view matrix
    view._11 = g_vRight.x; view._12 = g_vUp.x; view._13 = g_vLook.x;
    view._21 = g_vRight.y; view._22 = g_vUp.y; view._23 = g_vLook.y;
    view._31 = g_vRight.z; view._32 = g_vUp.z; view._33 = g_vLook.z;

    view._41 =- D3DXVec3Dot( &g_vCameraPos, &g_vRight );
    view._42 =- D3DXVec3Dot( &g_vCameraPos, &g_vUp    );
    view._43 =- D3DXVec3Dot( &g_vCameraPos, &g_vLook  );

    g_pd3dDevice->SetTransform( D3DTS_VIEW, &view ); 
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
    D3DXVECTOR3 *vec1 = (D3DXVECTOR3*)&Plane->VertexList[0];
    int Infront = 0;
    int Behind  = 0;
    int OnPlane = 0;
    float result;
    
    // Loop round each of the vertices
    for( int i = 0; i < Poly->nNumberOfVertices; ++i ) 
    {
        D3DXVECTOR3 *vec2     = (D3DXVECTOR3 *)&Poly->VertexList[i];
        D3DXVECTOR3 Direction = (*vec1) - (*vec2);
        
        result = D3DXVec3Dot(&Direction,&Plane->Normal);
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
int classifyPoint( D3DXVECTOR3 *pos, POLYGON *Plane )
{
    D3DXVECTOR3 *vec1     = (D3DXVECTOR3 *)&Plane->VertexList[0];
    D3DXVECTOR3 Direction = (*vec1) - (*pos);
    float       result    = D3DXVec3Dot(&Direction,&Plane->Normal);

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
void walkBspTree( NODE *Node, D3DXVECTOR3 *pos )
{
    if( Node->bIsLeaf == true )
        return;

    int result = classifyPoint( pos, Node->Splitter );
    
    if( result == POINTPOSITION_FRONT )
    {
        if( Node->Back  != NULL ) 
            walkBspTree( Node->Back, pos );

        g_pd3dDevice->DrawIndexedPrimitiveUP( D3DPT_TRIANGLELIST,
                                              0,
                                              Node->Splitter->nNumberOfVertices,
                                              Node->Splitter->nNumberOfIndices / 3,
                                              &Node->Splitter->Indices[0],
                                              D3DFMT_INDEX16,
                                              &Node->Splitter->VertexList[0],
                                              sizeof(VERTEX) );

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
                    VERTLIST[0][0] = setVertex(x-10.5f,3.0f,(20.0f-y)-0.5f, 1.0f,1.0f,1.0f,1.0f, 0.0f,0.0f);
                    VERTLIST[0][1] = setVertex(x- 9.5f,3.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 1.0f,0.0f);
                    VERTLIST[0][2] = setVertex(x- 9.5f,0.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 1.0f,1.0f);
                    VERTLIST[0][3] = setVertex(x-10.5f,0.0f,(20.0f-y)-0.5f, 1.0f,1.0f,1.0f,1.0f, 0.0f,1.0f);
                    direction[0]   = 1;
                    
                    if( x > 0 ) 
                    {
                        if( BSPMAP[offset-1] == 0) 
                        {
                            VERTLIST[1][0] = setVertex(x-10.5f,3.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 0.0f,0.0f);
                            VERTLIST[1][1] = setVertex(x-10.5f,3.0f,(20.0f-y)-0.5f, 1.0f,1.0f,1.0f,1.0f, 1.0f,0.0f);
                            VERTLIST[1][2] = setVertex(x-10.5f,0.0f,(20.0f-y)-0.5f, 1.0f,1.0f,1.0f,1.0f, 1.0f,1.0f);
                            VERTLIST[1][3] = setVertex(x-10.5f,0.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 0.0f,1.0f);
                            direction[1]   = 1;
                        }
                    }

                    if( y > 0 ) 
                    {
                        if( BSPMAP[offset-20] == 0 ) 
                        {
                            VERTLIST[2][0] = setVertex(x- 9.5f,3.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 0.0f,0.0f);
                            VERTLIST[2][1] = setVertex(x-10.5f,3.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 1.0f,0.0f);
                            VERTLIST[2][2] = setVertex(x-10.5f,0.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 1.0f,1.0f);
                            VERTLIST[2][3] = setVertex(x- 9.5f,0.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 0.0f,1.0f);
                            direction[2]   = 1;
                        }
                    }
                }
                
                if( BSPMAP[offset] == 3 )
                {   
                    VERTLIST[0][0] = setVertex(x-10.5f,3.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 0.0f,0.0f);
                    VERTLIST[0][1] = setVertex(x- 9.5f,3.0f,(20.0f-y)-0.5f, 1.0f,1.0f,1.0f,1.0f, 1.0f,0.0f);
                    VERTLIST[0][2] = setVertex(x- 9.5f,0.0f,(20.0f-y)-0.5f, 1.0f,1.0f,1.0f,1.0f, 1.0f,1.0f);
                    VERTLIST[0][3] = setVertex(x-10.5f,0.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 0.0f,1.0f);
                    direction[0]   = 1;
    
                    if(x < 19)
                    {
                        if( BSPMAP[offset+1] == 0)
                        {
                            VERTLIST[1][0] = setVertex(x-9.5f,3.0f,(20.0f-y)-0.5f, 1.0f,1.0f,1.0f,1.0f, 0.0f,0.0f);
                            VERTLIST[1][1] = setVertex(x-9.5f,3.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 1.0f,0.0f);
                            VERTLIST[1][2] = setVertex(x-9.5f,0.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 1.0f,1.0f);
                            VERTLIST[1][3] = setVertex(x-9.5f,0.0f,(20.0f-y)-0.5f, 1.0f,1.0f,1.0f,1.0f, 0.0f,1.0f);
                            direction[1]   = 1;
                        }
                    }

                    if( y > 0 )
                    {
                        if( BSPMAP[offset-20] == 0 )
                        {
                            VERTLIST[2][0] = setVertex(x- 9.5f,3.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 0.0f,0.0f);
                            VERTLIST[2][1] = setVertex(x-10.5f,3.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 1.0f,0.0f);
                            VERTLIST[2][2] = setVertex(x-10.5f,0.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 1.0f,1.0f);
                            VERTLIST[2][3] = setVertex(x- 9.5f,0.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 0.0f,1.0f);
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
                            VERTLIST[0][0] = setVertex(x-10.5f,3.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 0.0f,0.0f);
                            VERTLIST[0][1] = setVertex(x-10.5f,3.0f,(20.0f-y)-0.5f, 1.0f,1.0f,1.0f,1.0f, 1.0f,0.0f);
                            VERTLIST[0][2] = setVertex(x-10.5f,0.0f,(20.0f-y)-0.5f, 1.0f,1.0f,1.0f,1.0f, 1.0f,1.0f);
                            VERTLIST[0][3] = setVertex(x-10.5f,0.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 0.0f,1.0f);
                            direction[0]   = 1;
                        }
                    }
                    
                    if (x < 19) 
                    {
                        if( BSPMAP[offset+1] == 0) 
                        {
                            VERTLIST[1][0] = setVertex(x-9.5f,3.0f,(20.0f-y)-0.5f, 1.0f,1.0f,1.0f,1.0f, 0.0f,0.0f);
                            VERTLIST[1][1] = setVertex(x-9.5f,3.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 1.0f,0.0f);
                            VERTLIST[1][2] = setVertex(x-9.5f,0.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 1.0f,1.0f);
                            VERTLIST[1][3] = setVertex(x-9.5f,0.0f,(20.0f-y)-0.5f, 1.0f,1.0f,1.0f,1.0f, 0.0f,1.0f);
                            direction[1]   = 1;
                        }
                    }
    
                    if( y > 0 ) 
                    {
                        if( BSPMAP[offset-20] == 0 ) 
                        {
                            VERTLIST[2][0] = setVertex(x- 9.5f,3.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 0.0f,0.0f);
                            VERTLIST[2][1] = setVertex(x-10.5f,3.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 1.0f,0.0f);
                            VERTLIST[2][2] = setVertex(x-10.5f,0.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 1.0f,1.0f);
                            VERTLIST[2][3] = setVertex(x- 9.5f,0.0f,(20.0f-y)+0.5f, 1.0f,1.0f,1.0f,1.0f, 0.0f,1.0f);
                            direction[2]   = 1;
                        }
                    }

                    if( y < 39 )
                    {   
                        if( BSPMAP[offset+20] == 0)
                        {
                            VERTLIST[3][0] = setVertex(x-10.5f,3.0f,(20.0f-y)-0.5f, 1.0f,1.0f,1.0f,1.0f, 0.0f,0.0f);
                            VERTLIST[3][1] = setVertex(x- 9.5f,3.0f,(20.0f-y)-0.5f, 1.0f,1.0f,1.0f,1.0f, 1.0f,0.0f);
                            VERTLIST[3][2] = setVertex(x- 9.5f,0.0f,(20.0f-y)-0.5f, 1.0f,1.0f,1.0f,1.0f, 1.0f,1.0f);
                            VERTLIST[3][3] = setVertex(x-10.5f,0.0f,(20.0f-y)-0.5f, 1.0f,1.0f,1.0f,1.0f, 0.0f,1.0f);
                            direction[3]   = 1;
                        }
                    }

                }

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
bool getIntersect( D3DXVECTOR3 *linestart, D3DXVECTOR3 *lineend, 
                   D3DXVECTOR3 *vertex, D3DXVECTOR3 *normal, 
                   D3DXVECTOR3 *intersection, float *percentage )
{
    D3DXVECTOR3 direction;
    D3DXVECTOR3 L1;
    float       linelength;
    float       dist_from_plane;

    direction.x = lineend->x - linestart->x;
    direction.y = lineend->y - linestart->y;
    direction.z = lineend->z - linestart->z;

    linelength = D3DXVec3Dot(&direction, normal);
    
    if( fabsf( linelength ) < 0.001f ) 
        return false;

    L1.x = vertex->x - linestart->x;
    L1.y = vertex->y - linestart->y;
    L1.z = vertex->z - linestart->z;

    dist_from_plane = D3DXVec3Dot(&L1, normal);

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
//        the polygon into to two seperate polygons. When used for clipping to 
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
        Location = classifyPoint((D3DXVECTOR3*)&Poly->VertexList[i], Plane);

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
            D3DXVECTOR3 IntersectPoint;
            float       percent;

            getIntersect( (D3DXVECTOR3*)&Poly->VertexList[i], 
                          (D3DXVECTOR3*)&Poly->VertexList[CurrentVertex], 
                          (D3DXVECTOR3*)&Plane->VertexList[0], 
                          &Plane->Normal, &IntersectPoint, &percent );

            // create new vertex and calculate new texture coordinate
            VERTEX copy;
            float deltax = Poly->VertexList[CurrentVertex].tu - Poly->VertexList[i].tu;
            float deltay = Poly->VertexList[CurrentVertex].tv - Poly->VertexList[i].tv;
            float texx   = Poly->VertexList[i].tu + ( deltax * percent );
            float texy   = Poly->VertexList[i].tv + ( deltay * percent );
            copy.x       = IntersectPoint.x;
            copy.y       = IntersectPoint.y;
            copy.z       = IntersectPoint.z;
            copy.rgba    = Poly->VertexList[i].rgba;
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
    D3DXVECTOR3 *vec0 = (D3DXVECTOR3*) &Child->VertexList[0];
    D3DXVECTOR3 *vec1 = (D3DXVECTOR3*) &Child->VertexList[1];
    D3DXVECTOR3 *vec2 = (D3DXVECTOR3*) &Child->VertexList[Child->nNumberOfVertices-1];// the last vert

    D3DXVECTOR3 edge1 = (*vec1)-(*vec0);
    D3DXVECTOR3 edge2 = (*vec2)-(*vec0);
    D3DXVec3Cross(&Child->Normal,&edge1,&edge2);
    D3DXVec3Normalize(&Child->Normal,&Child->Normal);

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
        score = abs( (int)(frontfaces-backfaces) ) + (splits*4);

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
bool lineOfSight( D3DXVECTOR3 *Start, D3DXVECTOR3 *End, NODE *Node )
{
    D3DXVECTOR3 intersection;
    float       temp = 0.0f;

    if( Node->bIsLeaf == true )
        return !Node->bIsSolid;

    int PointA = classifyPoint(Start, Node->Splitter);
    int PointB = classifyPoint(End,   Node->Splitter);

    if( PointA == POINTPOSITION_ONPLANE && PointB == POINTPOSITION_ONPLANE ) 
        return lineOfSight(Start,End,Node->Front);

    if( PointA == POINTPOSITION_FRONT && PointB == POINTPOSITION_BACK ) 
    {
        getIntersect( Start, End, (D3DXVECTOR3 *)&Node->Splitter->VertexList[0], &Node->Splitter->Normal, &intersection, &temp);
        return lineOfSight( Start, &intersection, Node->Front) && lineOfSight( End, &intersection, Node->Back); 
    }

    if( PointA == POINTPOSITION_BACK && PointB == POINTPOSITION_FRONT ) 
    {
        getIntersect( Start, End, (D3DXVECTOR3 *)&Node->Splitter->VertexList[0], &Node->Splitter->Normal, &intersection, &temp);
        return lineOfSight( End, &intersection, Node->Front) && lineOfSight( Start, &intersection, Node->Back);
    }

    // if we get here one of the points is on the plane
    if( PointA == POINTPOSITION_FRONT || PointB == POINTPOSITION_FRONT ) 
        return lineOfSight( Start, End, Node->Front );
    else 
        return lineOfSight( Start, End, Node->Back );

    return true;
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

