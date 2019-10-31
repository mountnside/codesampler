//-----------------------------------------------------------------------------
//           Name: ogl_basic_collision.cpp
//         Author: Kevin Harris
//  Last Modified: 02/01/05
//    Description: As you may or may not know, truly accurate collision
//                 detection between two 3D objects can only be done by
//                 performing an intersection test on every triangle in the
//                 first object against every triangle of the other object.
//                 Obviously, it's too wasteful to just go around blindly
//                 performing this costly procedure on every object in the
//                 scene against every other object. To save us considerable
//                 CPU cycles, we'll calculate and assign a bounding sphere
//                 for each of our 3D objects so we can perform a quick check
//                 to see whether or not the two objects are even close to one
//                 another and require further testing. In short, if the two
//                 bounding spheres of our objects overlap, then there exists
//                 a possible collision and we need investigate further by
//                 checking triangles. If the two spheres don't overlap, a
//                 collision is not possible and we can move on.
//
// Note: Most of the math used in this sample is based on a small paper
// written by Gary Simmons, which is one of the instructors at
// www.gameinstitute.com. I've included the paper with the sample for reference.
// Please see it for further details.
//
//   Control Keys: F1 - Toggle bounding sphere visibility
//                 F2 - Toggle triangle motion
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
#include <mmsystem.h>
#include <iostream>
#include <io.h>
#include <fcntl.h>
using namespace std;
#include <GL/gl.h>
#include <GL/glu.h>
#include "resource.h"
#include "geometry.h"
#include "matrix4x4f.h"
#include "vector3f.h"

//-----------------------------------------------------------------------------
// SYMBOLIC CONSTANTS
//-----------------------------------------------------------------------------
enum Collision
{
    COLLISION_NO,
    COLLISION_YES,
    COLLISION_NOT_CHECKED
};

//-----------------------------------------------------------------------------
// STRUCTS
//-----------------------------------------------------------------------------
struct triangle
{
    vector3f v0;
    vector3f v1;
    vector3f v2;
    vector3f vNormal;

    // Bounding sphere
    vector3f vCenter;
    float    fRadius;
};
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

bool   g_bFirstRendering = true;
float  g_fElpasedTime = 0.0f;
double g_dCurTime;
double g_dLastTime;

vector3f g_vEye(8.0f, 8.0f, 8.0f);     // Eye Position
vector3f g_vLook(-0.5f, -0.5f, -0.5f); // Look Vector
vector3f g_vUp(0.0f, 1.0f, 0.0f);      // Up Vector
vector3f g_vRight(1.0f, 0.0f, 0.0f);   // Right Vector

triangle g_tri1;
triangle g_tri2;
bool     g_bDrawBoundingSpheres = true;
bool     g_bMoveSpheres = true;

struct Vertex
{
    // GL_C4UB_V3F
    unsigned char r, g, b, a;
    float x, y, z;
};

Vertex g_lineVertices[] =
{
    { 255,   0,   0, 255,  0.0f, 0.0f, 0.0f }, // red   = +x Axis
    { 255,   0,   0, 255,  5.0f, 0.0f, 0.0f },
    {   0, 255,   0, 255,  0.0f, 0.0f, 0.0f }, // green = +y Axis
    {   0, 255,   0, 255,  0.0f, 5.0f, 0.0f },
    {   0,   0, 255, 255,  0.0f, 0.0f, 0.0f }, // blue  = +z Axis
    {   0,   0, 255, 255,  0.0f, 0.0f, 5.0f }
};

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void redirectIOToConsole(void);
void init(void);
void render(void);
void shutDown(void);
void getRealTimeUserInput(void);
void updateViewMatrix(void);
void createBoundingSphere(triangle *tri);
bool doSpheresIntersect(triangle *tri1, triangle *tri2);
bool doTrianglesIntersect(triangle tri1, triangle tri2);
bool getLinePlaneIntersectionPoint( vector3f *vLineStart, vector3f *vLineEnd,
                                   vector3f *vPointInPlane, vector3f *vPlaneNormal,
                                   vector3f *vIntersection );
bool isPointInsideTriangle( vector3f *vIntersectionPoint, triangle *tri );

//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: The application's entry point
//-----------------------------------------------------------------------------
int WINAPI WinMain(	HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPSTR     lpCmdLine,
					int       nCmdShow )
{
    redirectIOToConsole();

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
                             "OpenGL - Basic 3D Collision Detection",
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
            if( g_bFirstRendering == true )
            {
                g_dLastTime = g_dCurTime = timeGetTime();
                g_bFirstRendering = false;
            }

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
                    g_bDrawBoundingSpheres = !g_bDrawBoundingSpheres;
                    break;

                case VK_F2:
                    g_bMoveSpheres = !g_bMoveSpheres;
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
			gluPerspective( 45.0, (GLdouble)nWidth / (GLdouble)nHeight, 0.1, 1000.0);
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
// Name: redirectIOToConsole()
// Desc: 
//-----------------------------------------------------------------------------
void redirectIOToConsole( void )
{
    // Allocate a console so we can output some useful information.
    AllocConsole();

    // Get the handle for STDOUT's file system.
    HANDLE stdOutputHandle = GetStdHandle( STD_OUTPUT_HANDLE );

    // Redirect STDOUT to the new console by associating STDOUT's file 
    // descriptor with an existing operating-system file handle.
    int hConsoleHandle = _open_osfhandle( (intptr_t)stdOutputHandle, _O_TEXT );
    FILE *pFile = _fdopen( hConsoleHandle, "w" );
    *stdout = *pFile;
    setvbuf( stdout, NULL, _IONBF, 0 );

    // This call ensures that iostream and C run-time library operations occur  
    // in the order that they appear in source code.
    ios::sync_with_stdio();
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
    pfd.dwFlags    = PFD_DRAW_TO_WINDOW |PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
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

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0f, 640.0f / 480.0f, 0.1f, 1000.0f);

    //
    // Triangle #1 (small/blue)
    //

    g_tri1.v0.x =  1.0f;
    g_tri1.v0.y = -1.0f;
    g_tri1.v0.z =  1.0f;

    g_tri1.v1.x = -1.0f;
    g_tri1.v1.y =  1.0f;
    g_tri1.v1.z =  1.0f;

    g_tri1.v2.x = -3.0f;
    g_tri1.v2.y = -1.0f;
    g_tri1.v2.z =  1.0f;

    g_tri1.vNormal = vector3f( 0.0f, 0.0f, 0.0f );

    createBoundingSphere( &g_tri1 );

    //
    // Triangle #2 (large/green)
    //

    g_tri2.v0.x = 0.0f;
    g_tri2.v0.y = 2.0f;
    g_tri2.v0.z = 0.0f;

    g_tri2.v1.x =  0.0f;
    g_tri2.v1.y = -2.0f;
    g_tri2.v1.z =  2.0f;

    g_tri2.v2.x =  0.0f;
    g_tri2.v2.y = -2.0f;
    g_tri2.v2.z = -2.0f;

    g_tri2.vNormal = vector3f( 0.0f, 0.0f, 0.0f );

    createBoundingSphere( &g_tri2 );
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
// Name: createBoundingSphere()
// Desc:
//-----------------------------------------------------------------------------
void createBoundingSphere( triangle *tri )
{
    float fMinX;
    float fMinY;
    float fMinZ;

    float fMaxX;
    float fMaxY;
    float fMaxZ;

    float fRadius1;
    float fRadius2;

    fMinX = fMaxX = tri->v0.x;
    fMinY = fMaxY = tri->v0.y;
    fMinZ = fMaxZ = tri->v0.z;

    if( tri->v1.x < fMinX ) fMinX = tri->v1.x;
    if( tri->v2.x < fMinX ) fMinX = tri->v2.x;
    if( tri->v1.y < fMinY ) fMinY = tri->v1.y;
    if( tri->v2.y < fMinY ) fMinY = tri->v2.y;
    if( tri->v1.z < fMinZ ) fMinZ = tri->v1.z;
    if( tri->v2.z < fMinZ ) fMinZ = tri->v2.z;

    if( tri->v1.x > fMaxX ) fMaxX = tri->v1.x;
    if( tri->v2.x > fMaxX ) fMaxX = tri->v2.x;
    if( tri->v1.y > fMaxY ) fMaxY = tri->v1.y;
    if( tri->v2.y > fMaxY ) fMaxY = tri->v2.y;
    if( tri->v1.z > fMaxZ ) fMaxZ = tri->v1.z;
    if( tri->v2.z > fMaxZ ) fMaxZ = tri->v2.z;

    tri->vCenter.x = (fMinX + fMaxX) / 2;
    tri->vCenter.y = (fMinY + fMaxY) / 2;
    tri->vCenter.z = (fMinZ + fMaxZ) / 2;

    fRadius1 = sqrt( ((tri->vCenter.x - tri->v0.x) * (tri->vCenter.x - tri->v0.x)) +
                     ((tri->vCenter.y - tri->v0.y) * (tri->vCenter.y - tri->v0.y)) +
                     ((tri->vCenter.z - tri->v0.z) * (tri->vCenter.z - tri->v0.z)) );

    fRadius2 = sqrt( ((tri->vCenter.x - tri->v1.x) * (tri->vCenter.x - tri->v1.x)) +
                     ((tri->vCenter.y - tri->v1.y) * (tri->vCenter.y - tri->v1.y)) +
                     ((tri->vCenter.z - tri->v1.z) * (tri->vCenter.z - tri->v1.z)) );

    if( fRadius1 < fRadius2 )
        fRadius1 = fRadius2;

    fRadius2 = sqrt( ((tri->vCenter.x - tri->v2.x) * (tri->vCenter.x - tri->v2.x)) +
                     ((tri->vCenter.y - tri->v2.y) * (tri->vCenter.y - tri->v2.y)) +
                     ((tri->vCenter.z - tri->v2.z) * (tri->vCenter.z - tri->v2.z)) );

    if( fRadius1 < fRadius2 )
		fRadius1 = fRadius2;

    tri->fRadius = fRadius1;

    return;
}

//-----------------------------------------------------------------------------
// Name: doSpheresIntersect()
// Desc: Determine whether two bounding spheres of "tri1" and "tr2" intersect.
//-----------------------------------------------------------------------------
bool doSpheresIntersect( triangle *tri1, triangle *tri2 )
{
    float fDistance = tri1->fRadius + tri2->fRadius;
	float fRadius;

    fRadius = sqrt( ((tri2->vCenter.x - tri1->vCenter.x) * (tri2->vCenter.x - tri1->vCenter.x)) +
                    ((tri2->vCenter.y - tri1->vCenter.y) * (tri2->vCenter.y - tri1->vCenter.y)) +
                    ((tri2->vCenter.z - tri1->vCenter.z) * (tri2->vCenter.z - tri1->vCenter.z)) );

    if( fRadius < fDistance )
        return true;

    else
        return false;
}

//-----------------------------------------------------------------------------
// Name: doTrianglesIntersect()
// Desc: Determine whether triangle "tri1" intersects "tri2".
//-----------------------------------------------------------------------------
bool doTrianglesIntersect( triangle tri1, triangle tri2 )
{
	bool bIntersect = false;
	vector3f vPoint;

	//
	// Create a normal for 'tri1'
	//

	vector3f vEdgeVec1 = tri1.v1 - tri1.v0;
	vector3f vEdgeVec2 = tri1.v2 - tri1.v0;
	tri1.vNormal = crossProduct( vEdgeVec1, vEdgeVec2 );
	//tri1.vNormal.normalize(); // Some people feel compelled to normalize this, but it's not really necessary.

	//
	// Check the first line segment of triangle #2 against triangle #1
    //
    // The first line segment is defined by vertices v0 and v1.
	//

	bIntersect = getLinePlaneIntersectionPoint( &tri2.v0,      // Line start
		                                        &tri2.v1,      // Line end
				                                &tri1.v0,      // A point in the plane
							                    &tri1.vNormal, // The plane's normal
				                                &vPoint );     // Holds the intersection point, if the function returns true

	if( bIntersect == true )
	{
		//
		// The line segment intersects the plane, but does it actually go 
		// through the triangle?
		//

		if( isPointInsideTriangle( &vPoint, &tri1 ) == true )
			return true;
	}

    //
	// Check the second line segment of triangle #2 against triangle #1
    //
    // The second line segment is defined by vertices v1 and v2.
	//

	bIntersect = getLinePlaneIntersectionPoint( &tri2.v1,      // Line start
		                                        &tri2.v2,      // Line end
				                                &tri1.v0,      // A point in the plane
							                    &tri1.vNormal, // The plane's normal
				                                &vPoint );     // Holds the intersection point, if the function returns true

	if( bIntersect == true )
	{
		//
		// The line segment intersects the plane, but does it actually go 
		// through the triangle?
		//

		if( isPointInsideTriangle( &vPoint, &tri1 ) == true )
			return true;
	}

    //
	// Check the third line segment of triangle #2 against triangle #1
    //
    // The third line segment is defined by vertices v2 and v0.
	//

	bIntersect = getLinePlaneIntersectionPoint( &tri2.v2,      // Line start
		                                        &tri2.v0,      // Line end
				                                &tri1.v0,      // A point in the plane
							                    &tri1.vNormal, // The plane's normal
				                                &vPoint );     // Holds the intersection point, if the function returns true

	if( bIntersect == true )
	{
		//
		// The line segment intersects the plane, but does it actually go 
		// through the triangle?
		//
		
		if( isPointInsideTriangle( &vPoint, &tri1 ) == true )
			return true;
	}

    return false;
}

//-----------------------------------------------------------------------------
// Name : getLinePlaneIntersectionPoint
// Desc : Determine whether a line or ray defined by "vLineStart" and "vLineEnd",
//        intersects with a plane which is defined by "vPlaneNormal" and
//        "vPointInPlane". If it doesn't, return false, otherwise, return true
//        and set "vIntersection" to the intersection point in 3D space.
//-----------------------------------------------------------------------------
bool getLinePlaneIntersectionPoint( vector3f *vLineStart, vector3f *vLineEnd,
				                    vector3f *vPointInPlane, vector3f *vPlaneNormal,
				                    vector3f *vIntersection )
{
	vector3f vDirection;
	vector3f L1;
	float	 fLineLength;
    float    fDistanceFromPlane;
	float    fPercentage;

	vDirection.x = vLineEnd->x - vLineStart->x;
	vDirection.y = vLineEnd->y - vLineStart->y;
	vDirection.z = vLineEnd->z - vLineStart->z;

	fLineLength = dotProduct( vDirection, *vPlaneNormal );

	// Check the line's length allowing for some tolerance for floating point
	// rounding errors. If it's 0 or really close to 0, the line is parallel to
	// the plane and can not intersect it.
	if( fabsf( fLineLength ) < 0.001f )
        return false;

	L1.x = vPointInPlane->x - vLineStart->x;
	L1.y = vPointInPlane->y - vLineStart->y;
	L1.z = vPointInPlane->z - vLineStart->z;

	fDistanceFromPlane = dotProduct( L1, *vPlaneNormal );

	// How far from Linestart , intersection is as a percentage of 0 to 1
	fPercentage	= fDistanceFromPlane / fLineLength;

	if( fPercentage < 0.0f || // The plane is behind the start of the line
		fPercentage > 1.0f )  // The line segment does not reach the plane
        return false;

	// Add the percentage of the line to line start
	vIntersection->x = vLineStart->x + vDirection.x * fPercentage;
	vIntersection->y = vLineStart->y + vDirection.y * fPercentage;
	vIntersection->z = vLineStart->z + vDirection.z * fPercentage;

	return true;
}

//-----------------------------------------------------------------------------
// Name : isPointInsideTriangle
// Desc : Determine wether a point in 3D space, "vIntersectionPoint", can be
//        considered to be inside of the three vertices of a triangle as
//        defined by "tri".
//-----------------------------------------------------------------------------
bool isPointInsideTriangle( vector3f *vIntersectionPoint, triangle *tri )
{
	vector3f vVectors[3];
	float fTotalAngle = 0.0f; // As radians

	//
	// Create and normalize three vectors that radiate out from the
	// intersection point towards the triangle's three vertices.
	//

	vVectors[0] = *vIntersectionPoint - tri->v0;
	vVectors[0].normalize();

	vVectors[1] = *vIntersectionPoint - tri->v1;
	vVectors[1].normalize();

	vVectors[2] = *vIntersectionPoint - tri->v2;
	vVectors[2].normalize();

	//
	// We then sum together the angles that exist between each of the vectors.
	//
	// Here's how:
	//
	// 1. Use dotProduct() to get cosine of the angle between the two vectors.
	// 2. Use acos() to convert cosine back into an angle.
	// 3. Add angle to fTotalAngle to keep track of running sum.
	//

	fTotalAngle  = acos( dotProduct( vVectors[0], vVectors[1] ) );
	fTotalAngle += acos( dotProduct( vVectors[1], vVectors[2] ) );
	fTotalAngle += acos( dotProduct( vVectors[2], vVectors[0] ) );

	//
	// If we are able to sum together all three angles and get 360.0, the
	// intersection point is inside the triangle.
	//
	// We can check this by taking away 6.28 radians (360 degrees) away from
	// fTotalAngle and if we're left with 0 (allowing for some tolerance) the
	// intersection point is definitely inside the triangle.
	//

	if( fabsf( fTotalAngle - 6.28f ) < 0.01f )
		return true;

	return false;
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

    //
	// Place one of the triangles in motion to demonstrate collision detection.
	//

	static bool bMoveBack = true;

	if( g_bMoveSpheres == true )
	{
        float fMoveAmount = 2.0f * g_fElpasedTime;

		if( bMoveBack == true )
		{
			g_tri1.v0.x -= fMoveAmount;
			g_tri1.v1.x -= fMoveAmount;
			g_tri1.v2.x -= fMoveAmount;
			g_tri1.vCenter.x -= fMoveAmount;

			if( g_tri1.vCenter.x < -7.0f )
				bMoveBack = false;
		}
		else
		{
			g_tri1.v0.x += fMoveAmount;
			g_tri1.v1.x += fMoveAmount;
			g_tri1.v2.x += fMoveAmount;
			g_tri1.vCenter.x += fMoveAmount;

			if( g_tri1.vCenter.x > 7.0f )
				bMoveBack = true;
		}
	}

	//
    // Check for collisions...
	//

	Collision nCollisionStateOfSpheres = COLLISION_NO;
	Collision nCollisionStateOfTris    = COLLISION_NO;

    if( doSpheresIntersect( &g_tri1, &g_tri2 ) == true )
    {
		// Hmmm... the spheres are colliding, so it's possible that the triangles are colliding as well.
		nCollisionStateOfSpheres = COLLISION_YES;

		// We'll start off by checking tri1 against tri2
        if( doTrianglesIntersect( g_tri1, g_tri2 ) == true )
			nCollisionStateOfTris = COLLISION_YES;
        else
        {
			// Ok, tr1 doesn't seem to intersect tri2, but maybe tri2 will intersect tr1.
            if( doTrianglesIntersect( g_tri2, g_tri1 ) == true )
				nCollisionStateOfTris = COLLISION_YES;
        }
    }
	else
	{
		// The spheres aren't colliding, so the triangles couldn't
		// possibly be colliding either.
		nCollisionStateOfTris = COLLISION_NOT_CHECKED;
	}

	//
	// Print out collision states for both spheres and triangles...
	//
    
	if( nCollisionStateOfSpheres == COLLISION_NO )
		cout << "Spheres = COLLISION_NO   |  ";
	else if( nCollisionStateOfSpheres == COLLISION_YES )
		cout << "Spheres = COLLISION_YES  |  ";

	if( nCollisionStateOfTris == COLLISION_NO )
		cout << "Triangles = COLLISION_NO" << endl;
	else if( nCollisionStateOfTris == COLLISION_YES )
		cout << "Triangles = COLLISION_YES" << endl;
	else if( nCollisionStateOfTris == COLLISION_NOT_CHECKED )
		cout << "Triangles = COLLISION_NOT_CHECKED" << endl;

	//
    // Draw triangle 1...
	//

    if( nCollisionStateOfTris == COLLISION_NO ||
		nCollisionStateOfTris == COLLISION_NOT_CHECKED )
        glColor3f( 0.0f, 0.0f, 1.0f );
    else if( nCollisionStateOfTris == COLLISION_YES )
        glColor3f( 1.0f, 0.0f, 0.0f );

    glBegin(GL_POLYGON);
	{
		glVertex3f( g_tri1.v0.x, g_tri1.v0.y, g_tri1.v0.z );
		glVertex3f( g_tri1.v1.x, g_tri1.v1.y, g_tri1.v1.z );
		glVertex3f( g_tri1.v2.x, g_tri1.v2.y, g_tri1.v2.z );
	}
    glEnd();

    if( g_bDrawBoundingSpheres == true )
    {
        if( nCollisionStateOfSpheres == COLLISION_NO )
            glColor3f( 0.0f, 0.0f, 1.0f );
        else if( nCollisionStateOfSpheres == COLLISION_YES )
            glColor3f( 1.0f, 0.0f, 0.0f );

        glPushMatrix();
        glTranslatef( g_tri1.vCenter.x, g_tri1.vCenter.y, g_tri1.vCenter.z );
        renderWireSphere( g_tri1.fRadius, 16, 16 );
        glPopMatrix();
    }

	//
    // Draw triangle 2...
	//

    if( nCollisionStateOfTris == COLLISION_NO ||
		nCollisionStateOfTris == COLLISION_NOT_CHECKED )
        glColor3f( 0.0f, 1.0f, 0.0f );
    else if( nCollisionStateOfTris == COLLISION_YES )
        glColor3f( 1.0f, 1.0f, 0.0f );

    glBegin(GL_POLYGON);
	{
		glVertex3f( g_tri2.v0.x, g_tri2.v0.y, g_tri2.v0.z );
		glVertex3f( g_tri2.v1.x, g_tri2.v1.y, g_tri2.v1.z );
		glVertex3f( g_tri2.v2.x, g_tri2.v2.y, g_tri2.v2.z );
	}
    glEnd();

	//
    // Draw bounding spheres...
	//

    if( g_bDrawBoundingSpheres == true )
    {
        if( nCollisionStateOfSpheres == COLLISION_NO )
            glColor3f( 0.0f, 1.0f, 0.0f );
        else if( nCollisionStateOfSpheres == COLLISION_YES )
            glColor3f( 1.0f, 1.0f, 0.0f );

        glPushMatrix();
        glTranslatef( g_tri2.vCenter.x, g_tri2.vCenter.y, g_tri2.vCenter.z );
        renderWireSphere( g_tri2.fRadius, 16, 16 );
        glPopMatrix();
    }

    //
    // Draw the X, Y, and Z axis...
    //

    glInterleavedArrays( GL_C4UB_V3F, 0, g_lineVertices );
    glDrawArrays( GL_LINES, 0, 6 );

	SwapBuffers( g_hDC );
}
