//-----------------------------------------------------------------------------
//           Name: ogl_point_rotated_billboard.cpp
//         Author: Kevin Harris
//  Last Modified: 02/01/05
//    Description: An example of point rotated billboarding.
//
//   Control Keys: F1         - Toggle billboarding
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
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glaux.h>
#include "resource.h"
#include "matrix4x4f.h"
#include "vector3f.h"
#include "tga.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND  g_hWnd = NULL;
HDC	  g_hDC  = NULL;
HGLRC g_hRC  = NULL;

GLuint g_sphereTextureID = -1;
GLuint g_tilesTextureID = -1;
bool   g_bBillboard = true;
float  g_fSize      = 0.5f;

POINT  g_ptLastMousePosit;
POINT  g_ptCurrentMousePosit;
bool   g_bMousing = false;
float  g_fMoveSpeed = 25.0f;
float  g_fElpasedTime;
double g_dCurTime;
double g_dLastTime;

vector3f g_vEye(0.0f, 0.0f, 5.0f);   // Eye Position
vector3f g_vLook(0.0f, 0.0f, -1.0f); // Look Vector
vector3f g_vUp(0.0f, 1.0f, 0.0f);    // Up Vector
vector3f g_vRight(1.0f, 0.0f, 0.0f); // Right Vector

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
                             "OpenGL - Point Rotated Billboard",
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
                    g_bBillboard = !g_bBillboard;
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
void loadTexture( void )	
{
    //
    // For the billboard poly, load a texture of a steel sphere. We'll use a 
    // .tga file for this image so we can use an alpha channel.
    //

    tgaImageFile tgaImage;
    tgaImage.load( "steel_sphere.tga" );

    glGenTextures( 1, &g_sphereTextureID );

    glBindTexture( GL_TEXTURE_2D, g_sphereTextureID );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    glTexImage2D( GL_TEXTURE_2D, 0, tgaImage.m_texFormat, 
                  tgaImage.m_nImageWidth, tgaImage.m_nImageHeight, 
                  0, tgaImage.m_texFormat, GL_UNSIGNED_BYTE, 
                  tgaImage.m_nImageData );

    //
    // For the ground poly, load a texture of some tiles from a regular 
    // .bmp file.
    //

    AUX_RGBImageRec *pTextureImage = auxDIBImageLoad( ".\\tiles.bmp" );

    if( pTextureImage != NULL )
    {
        glGenTextures(1, &g_tilesTextureID);

        glBindTexture(GL_TEXTURE_2D, g_tilesTextureID);

        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

        glTexImage2D( GL_TEXTURE_2D, 0, 3,
            pTextureImage->sizeX,
            pTextureImage->sizeY,
            0, GL_RGB, GL_UNSIGNED_BYTE,
            pTextureImage->data );
    }

    if( pTextureImage != NULL )
    {
        if( pTextureImage->data != NULL )
            free( pTextureImage->data );

        free( pTextureImage );
    }
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

    glClearColor( 0.35f, 0.53f, 0.7f, 1.0f );
    glEnable( GL_TEXTURE_2D );
    glEnable( GL_DEPTH_TEST );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0f, 640.0f / 480.0f, 0.1f, 100.0f);

    loadTexture();
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )
{
    glDeleteTextures( 1, &g_sphereTextureID );
    glDeleteTextures( 1, &g_tilesTextureID );

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
    // Render Floor Poly
    //

    glBindTexture( GL_TEXTURE_2D, g_tilesTextureID );

    glBegin( GL_QUADS );
    {
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-2.0f, -1.0f, -2.0f);

        glTexCoord2f(1.0f, 0.0f);
        glVertex3f( 2.0f, -1.0f, -2.0f);

        glTexCoord2f(1.0f, 1.0f);
        glVertex3f( 2.0f, -1.0f, 2.0f);

        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-2.0f, -1.0f, 2.0f);
    }
    glEnd();

    //
    // Render Billboard poly
    //

    glPushMatrix();

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glEnable( GL_ALPHA_TEST );
    glAlphaFunc( GL_GREATER, 0 );

    glBindTexture( GL_TEXTURE_2D, g_sphereTextureID );

    // Start of with a simple quad...
    vector3f vPoint0(-1.0, -1.0, 0.0f);
    vector3f vPoint1( 1.0, -1.0, 0.0f);
    vector3f vPoint2( 1.0,  1.0, 0.0f);
    vector3f vPoint3(-1.0,  1.0, 0.0f);

    if( g_bBillboard == true )
    {
        // If billboarding is toggled on, recompute the quad points
        // with respect to the view matrix and the quad's center point.
        // For this demo, the quad's center is always at the origin...
        float mat[16];
        glGetFloatv( GL_MODELVIEW_MATRIX, mat );

        vector3f vRight( mat[0], mat[4], mat[8] );
        vector3f vUp( mat[1], mat[5], mat[9] );
        vector3f vCenter( 0.0f, 0.0f, 0.0f );

        // Now, build a quad around the center point based on the vRight 
        // and vUp vectors. This will guarantee that the quad will be 
        // orthogonal to the view.
        vPoint0 = vCenter + ((-vRight - vUp) * g_fSize);
        vPoint1 = vCenter + (( vRight - vUp) * g_fSize);
        vPoint2 = vCenter + (( vRight + vUp) * g_fSize);
        vPoint3 = vCenter + ((-vRight + vUp) * g_fSize);

        //---------------------------------------------------------------------
        //
        // vPoint3                vPoint2
        //         +------------+
        //         |            |
        //         |            |
        //         |     +      |
        //         |  vCenter   |
        //         |            |
        //         |            |
        //         +------------+
        // vPoint0                vPoint1
        //
        //---------------------------------------------------------------------
    }

    glBegin( GL_QUADS );
    {
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(vPoint0.x,vPoint0.y,vPoint0.z);

        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(vPoint1.x, vPoint1.y,vPoint1.z);

        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(vPoint2.x,vPoint2.y,vPoint2.z);

        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(vPoint3.x,vPoint3.y,vPoint3.z);
    }
    glEnd();

    glDisable( GL_BLEND );
    glDisable( GL_ALPHA_TEST );

	SwapBuffers( g_hDC );
}
