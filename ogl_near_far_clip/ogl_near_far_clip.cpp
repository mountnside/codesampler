//-----------------------------------------------------------------------------
//           Name: ogl_near_far_clip.cpp
//         Author: Kevin Harris
//  Last Modified: 02/01/05
//    Description: This sample demonstrates how adjustments to OpenGL's 
//                 near and far clip planes effect the view.
//
//   Control Keys: Up         - View moves forward
//                 Down       - View moves backward
//                 Left       - View strafes left
//                 Right      - View strafes Right
//                 Left Mouse - Perform looking
//                 Mouse      - Look about the scene
//
//                 F1         - Increase near clip value
//                 F2         - Decrease near clip value
//                 F3         - Increase far clip value
//                 F4         - Decrease far clip value
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <mmsystem.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "geometry.h"
#include "resource.h"

#include "matrix4x4f.h"
#include "vector3f.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND  g_hWnd = NULL;
HDC	  g_hDC  = NULL;
HGLRC g_hRC  = NULL;

POINT  g_ptLastMousePosit;
POINT  g_ptCurrentMousePosit;
bool   g_bMousing = false;
float  g_fMoveSpeed = 25.0f;
float  g_fElpasedTime;
double g_dCurTime;
double g_dLastTime;

vector3f g_vEye(7.0f, 4.0f, 7.0f);      // Eye Position
vector3f g_vLook(-0.5f, -0.4f, -0.5f);  // Look Vector
vector3f g_vUp(0.0f, 1.0f, 0.0f);       // Up Vector
vector3f g_vRight(1.0f, 0.0f, 0.0f);    // Right Vector

float g_fNearClipPlane =  8.0f;
float g_fFarClipPlane  = 20.0f;
bool  g_bNearFarClipPlanesAdjusted = false;

/*
const int g_nNumVertsX = 32; // Number of wall vertices along x axis
const int g_nNumVertsZ = 32; // Number of wall vertices along z axis

// Number of triangles needed for the wall
const int g_nTriCount = (g_nNumVertsX-1)*(g_nNumVertsZ-1)*2;

struct MeshVertex
{
    float nx, ny, nz;
    float x, y, z;
};

MeshVertex g_wallVertices[g_nTriCount*3];
*/

// Mesh properties...
const int   g_nNumVertsAlongX   = 32;
const int   g_nNumVertsAlongZ   = 32;
const float g_fMeshLengthAlongX = 10.0f;
const float g_fMeshLengthAlongZ = 10.0f;

const int g_nMeshVertCount = (g_nNumVertsAlongX-1) * (g_nNumVertsAlongZ-1) * 6;

struct Vertex
{
    // GL_N3F_V3F
    float nx, ny, nz;
    float x, y, z;
};

Vertex g_meshVertices[g_nMeshVertCount];

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void init(void);
void render(void);
void shutDown(void);
void createMesh(void);
void getRealTimeUserInput(void);
void updateViewMatrix(void);

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
                             "OpenGL - Near/Far Clip Plane",
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
					g_fNearClipPlane += 0.1f;
                    g_bNearFarClipPlanesAdjusted = true;
					break;

                case VK_F2:
					g_fNearClipPlane -= 0.1f;
                    g_bNearFarClipPlanesAdjusted = true;
					break;

                case VK_F3:
					g_fFarClipPlane += 0.1f;
                    g_bNearFarClipPlanesAdjusted = true;
					break;

                case VK_F4:
					g_fFarClipPlane -= 0.1f;
                    g_bNearFarClipPlanesAdjusted = true;
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
			gluPerspective( 45.0, (GLdouble)nWidth / (GLdouble)nHeight, g_fNearClipPlane, g_fFarClipPlane);
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

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0f, 640.0f / 480.0f, g_fNearClipPlane, g_fFarClipPlane);

    createMesh();
}

//-----------------------------------------------------------------------------
// Name: createMesh()
// Desc: 
//-----------------------------------------------------------------------------
void createMesh( void )
{
    // Compute position deltas for moving down the X, and Z axis during mesh creation
    const float dX =  (1.0f/(g_nNumVertsAlongX-1));
    const float dZ = -(1.0f/(g_nNumVertsAlongZ-1));

    int i = 0;
    int x = 0;
    int z = 0;

    // These are all the same...
    for( i = 0; i < g_nMeshVertCount; ++i )
    {
        // Mesh tesselation occurs in the X,Z plane, so Y is always zero
        g_meshVertices[i].y = 0.0f;

        g_meshVertices[i].nx = 0.0f;
        g_meshVertices[i].ny = 1.0f;
        g_meshVertices[i].nz = 0.0f;
    }

    //
    // Create all the vertex points required by the mesh...
    //
    // Note: Mesh tesselation occurs in the X,Z plane.
    //

    // For each row of our mesh...
    for( z = 0, i = 0; z < (g_nNumVertsAlongZ-1); ++z )
    {
        // Fill the row with quads which are composed of two triangles each...
        for( x = 0; x < (g_nNumVertsAlongX-1); ++x )
        {
            // First triangle of the current quad
            //   ___ 2
            //  |  /|
            //  |/__|
            // 0     1

            // 0
            g_meshVertices[i].x  = g_fMeshLengthAlongX * x * dX;
            g_meshVertices[i].z  = g_fMeshLengthAlongZ * z * dZ;
            ++i;

            // 1
            g_meshVertices[i].x  = g_fMeshLengthAlongX * (x+1.0f) * dX;
            g_meshVertices[i].z  = g_fMeshLengthAlongZ * z * dZ;
            ++i;

            // 2
            g_meshVertices[i].x  = g_fMeshLengthAlongX * (x+1.0f) * dX;
            g_meshVertices[i].z  = g_fMeshLengthAlongZ * (z+1.0f) * dZ;
            ++i;

            // Second triangle of the current quad
            // 2 ___ 1
            //  |  /|
            //  |/__|
            // 0

            // 0
            g_meshVertices[i].x  = g_fMeshLengthAlongX * x * dX;
            g_meshVertices[i].z  = g_fMeshLengthAlongZ * z * dZ;
            ++i;

            // 1
            g_meshVertices[i].x  = g_fMeshLengthAlongX * (x+1.0f) * dX;
            g_meshVertices[i].z  = g_fMeshLengthAlongZ * (z+1.0f) * dZ;
            ++i;

            // 2
            g_meshVertices[i].x = g_fMeshLengthAlongX * x * dX;
            g_meshVertices[i].z = g_fMeshLengthAlongZ * (z+1.0f) * dZ;
            ++i;
        }
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
// Name: render()
// Desc: 
//-----------------------------------------------------------------------------
void render( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    //
    // Set near/far clip planes...
    //

	if( g_bNearFarClipPlanesAdjusted == true )
    {
        glMatrixMode( GL_PROJECTION );
	    glLoadIdentity();
	    gluPerspective( 45.0f, 640.0f / 480.0f, g_fNearClipPlane, g_fFarClipPlane);
        g_bNearFarClipPlanesAdjusted = false;
    }

    getRealTimeUserInput();

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    updateViewMatrix();

    //
    // Render floor...
    //

    glColor3f( 1.0f, 1.0f, 1.0f );
    glPolygonMode( GL_FRONT, GL_LINE );

    glPushMatrix();
    {
        glScalef( 2.0f, 2.0f, 2.0f );
        glTranslatef( -5.0f, -0.8f, 5.0f );
        
	    glInterleavedArrays( GL_N3F_V3F, 0, g_meshVertices );
        glDrawArrays( GL_TRIANGLES , 0, g_nMeshVertCount );
    }
    glPopMatrix();

    //
    // Render teapot...
    //

    glColor3f( 0.0f, 0.0f, 1.0f );
    renderWireTeapot( 2.0f );

	SwapBuffers( g_hDC );
}
