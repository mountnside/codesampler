//-----------------------------------------------------------------------------
//           Name: ogl_texture_mipmapping.cpp
//         Author: Kevin Harris
//  Last Modified: 02/01/05
//    Description: This sample demonstrates how to mip-map textures with 
//                 OpenGL.
//
//   Control Keys: F1 - Change Minification filter
//                 F2 - Change Magnification filter
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
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glaux.h>
#include "resource.h"
#include "matrix4x4f.h"
#include "vector3f.h"
#include "bitmap_fonts.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND   g_hWnd            = NULL;
HDC	   g_hDC             = NULL;
HGLRC  g_hRC             = NULL;
int    g_nWindowWidth    = 640;
int    g_nWindowHeight   = 480;
GLuint g_mipMapTextureID = -1;

POINT  g_ptLastMousePosit;
POINT  g_ptCurrentMousePosit;
bool   g_bMousing = false;
float  g_fMoveSpeed = 25.0f;
float  g_fElpasedTime;
double g_dCurTime;
double g_dLastTime;

vector3f g_vEye(0.0f, 10.0f, 0.0f);   // Camera Position
vector3f g_vLook(0.5f, -0.4f, -0.5f); // Camera Look Vector
vector3f g_vUp(0.0f, 1.0f, 0.0f);     // Camera Up Vector
vector3f g_vRight(1.0f, 0.0f, 0.0f);  // Camera Right Vector

struct Vertex
{
    float tu, tv;
    float x, y, z;
};

Vertex g_quadVertices[] =
{
    { 0.0f,0.0f, -1.0f, 0.0f, -1.0f },
    { 1.0f,0.0f,  1.0f, 0.0f, -1.0f },
    { 1.0f,1.0f,  1.0f, 0.0f,  1.0f },
    { 0.0f,1.0f, -1.0f, 0.0f,  1.0f }
};

enum FilterTypes
{
	FILTER_TYPE_NEAREST = 0,
	FILTER_TYPE_LINEAR,
    FILTER_TYPE_NEAREST_MIPMAP_NEAREST,
    FILTER_TYPE_LINEAR_MIPMAP_NEAREST,
    FILTER_TYPE_NEAREST_MIPMAP_LINEAR,
    FILTER_TYPE_LINEAR_MIPMAP_LINEAR
};

int	 g_MinFilterType  = FILTER_TYPE_LINEAR_MIPMAP_LINEAR;
int	 g_MagFilterType  = FILTER_TYPE_LINEAR;
bool g_bChangeFilters = true;

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void loadMipMapTexture(void);
void init(void);
void render(void);
void shutDown(void);
void getRealTimeUserInput(void);
void updateViewMatrix(void);
void setMagnificationFilter(void);
void setMinificationFilter(void);
void setMipMapFilter(void);

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
                             "OpenGL - Texturing Mipmapping",
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
					++g_MinFilterType;
					if(g_MinFilterType > 5)
						g_MinFilterType = 0;
					g_bChangeFilters = true;
					break;

                case VK_F2:
					++g_MagFilterType;
					if(g_MagFilterType > 1)
						g_MagFilterType = 0;
					g_bChangeFilters = true;
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
// Name: loadMipMapTexture()
// Desc: 
//-----------------------------------------------------------------------------
void loadMipMapTexture( void )
{
	AUX_RGBImageRec *pTextureImage256base = auxDIBImageLoad( ".\\texbase256.bmp" );

    if( pTextureImage256base != NULL )
	{
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        
        glGenTextures( 1, &g_mipMapTextureID );

		glBindTexture(GL_TEXTURE_2D, g_mipMapTextureID);

        gluBuild2DMipmaps(GL_TEXTURE_2D, 3, pTextureImage256base->sizeX,
			              pTextureImage256base->sizeY, GL_RGB, GL_UNSIGNED_BYTE,
						  pTextureImage256base->data);
	}

	if( pTextureImage256base )
	{
		if( pTextureImage256base->data )
			free( pTextureImage256base->data );

		free( pTextureImage256base );
	}

    // I don't have access to these... must be a SGI only thing!
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 5);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, 2.5);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 4.5);

    // 256 = red
	// 128 = blue
	//  64 = green
	//  32 = yellow
	//  16 = purple
	//   8 = light blue

	AUX_RGBImageRec *pTexArray[6];

	pTexArray[0] = auxDIBImageLoad( ".\\tex256.bmp" );
	pTexArray[1] = auxDIBImageLoad( ".\\tex128.bmp" );
	pTexArray[2] = auxDIBImageLoad( ".\\tex64.bmp" );
	pTexArray[3] = auxDIBImageLoad( ".\\tex32.bmp" );
	pTexArray[4] = auxDIBImageLoad( ".\\tex16.bmp" );
	pTexArray[5] = auxDIBImageLoad( ".\\tex8.bmp" );

	glTexImage2D( GL_TEXTURE_2D, 0, 3, pTexArray[0]->sizeX, pTexArray[0]->sizeY, 0,
				GL_RGB, GL_UNSIGNED_BYTE, pTexArray[0]->data);

	glTexImage2D( GL_TEXTURE_2D, 1, 3, pTexArray[1]->sizeX, pTexArray[1]->sizeY, 0,
				GL_RGB, GL_UNSIGNED_BYTE, pTexArray[1]->data);

	glTexImage2D( GL_TEXTURE_2D, 2, 3, pTexArray[2]->sizeX, pTexArray[2]->sizeY, 0,
				GL_RGB, GL_UNSIGNED_BYTE, pTexArray[2]->data);

	glTexImage2D( GL_TEXTURE_2D, 3, 3, pTexArray[3]->sizeX, pTexArray[3]->sizeY, 0,
				GL_RGB, GL_UNSIGNED_BYTE, pTexArray[3]->data);

	glTexImage2D( GL_TEXTURE_2D, 4, 3, pTexArray[4]->sizeX, pTexArray[4]->sizeY, 0,
				GL_RGB, GL_UNSIGNED_BYTE, pTexArray[4]->data);

	glTexImage2D( GL_TEXTURE_2D, 5, 3, pTexArray[5]->sizeX, pTexArray[5]->sizeY, 0,
				GL_RGB, GL_UNSIGNED_BYTE, pTexArray[5]->data);
	
	for( int i = 0; i < 6; ++i )
	{
		if( pTexArray[i] )
		{
			if( pTexArray[i]->data )
				free( pTexArray[i]->data );

			free( pTexArray[i] );
		}
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

	loadMipMapTexture();

	glClearColor( 0.35f, 0.53f, 0.7f, 1.0f );
    glEnable(GL_TEXTURE_2D);

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0, (GLdouble)g_nWindowWidth / g_nWindowHeight, 0.1, 100.0 );
}

//-----------------------------------------------------------------------------
// Name : updateViewMatrix() 
// Desc : 
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
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )	
{
    glDeleteTextures( 1, &g_mipMapTextureID );

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
// Name : setMinificationFilter()
// Desc : 
//-----------------------------------------------------------------------------
void setMinificationFilter( void )
{
    if( g_MinFilterType == 0 )
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    if( g_MinFilterType == 1 )
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    if( g_MinFilterType == 2 )
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);

    if( g_MinFilterType == 3 )
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

    if( g_MinFilterType == 4 )
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);

    if( g_MinFilterType == 5 )
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
}

//-----------------------------------------------------------------------------
// Name : setMagnificationFilter()
// Desc : 
//-----------------------------------------------------------------------------
void setMagnificationFilter( void )
{
    if( g_MagFilterType == 0 )
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    if( g_MagFilterType == 1 )
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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

	if( g_bChangeFilters == true )
	{
		setMinificationFilter();
		setMagnificationFilter();
		g_bChangeFilters = false;
	}

	glBindTexture( GL_TEXTURE_2D, g_mipMapTextureID );
	
	float x = 0.0f;
	float z = 0.0f;

	for( int i = 0; i < 25; ++i )
    {
		for( int j = 0; j < 25; ++j )
		{
			glPushMatrix();
			glTranslatef( x, 0.0f, z );
			glInterleavedArrays( GL_T2F_V3F, 0, g_quadVertices );
			glDrawArrays( GL_QUADS, 0, 4 );
			glPopMatrix();

			x += 2.0f;
		}
		x  =  0.0f;
		z -= 2.0f;
	}

    //
    // Output the current settings...
    //

	static char strMinFilter[255];
	static char strMagFilter[255];

	if( g_MinFilterType == FILTER_TYPE_NEAREST )
		sprintf( strMinFilter, "GL_TEXTURE_MIN_FILTER = GL_NEAREST    (Change: F1)" );
	else if( g_MinFilterType == FILTER_TYPE_LINEAR )
		sprintf( strMinFilter, "GL_TEXTURE_MIN_FILTER = GL_LINEAR    (Change: F1)" );
	else if( g_MinFilterType == FILTER_TYPE_NEAREST_MIPMAP_NEAREST )
		sprintf( strMinFilter, "GL_TEXTURE_MIN_FILTER = GL_NEAREST_MIPMAP_NEAREST    (Change: F1)" );
	else if( g_MinFilterType == FILTER_TYPE_LINEAR_MIPMAP_NEAREST )
		sprintf( strMinFilter, "GL_TEXTURE_MIN_FILTER = GL_LINEAR_MIPMAP_NEAREST    (Change: F1)" );
	else if( g_MinFilterType == FILTER_TYPE_NEAREST_MIPMAP_LINEAR )
		sprintf( strMinFilter, "GL_TEXTURE_MIN_FILTER = GL_NEAREST_MIPMAP_LINEAR    (Change: F1)" );
    else if( g_MinFilterType == FILTER_TYPE_LINEAR_MIPMAP_LINEAR )
        sprintf( strMinFilter, "GL_TEXTURE_MIN_FILTER = GL_LINEAR_MIPMAP_LINEAR    (Change: F1)" );

	if( g_MagFilterType == FILTER_TYPE_NEAREST )
		sprintf( strMagFilter, "GL_TEXTURE_MAG_FILTER = GL_NEAREST    (Change: F2)" );
	else if( g_MagFilterType == FILTER_TYPE_LINEAR )
		sprintf( strMagFilter, "GL_TEXTURE_MAG_FILTER = GL_LINEAR    (Change: F2)" );

	beginRenderText( g_nWindowWidth, g_nWindowHeight );
	{
		glColor3f( 0.0f, 0.0f, 0.0f );
		renderText( 5, 15, BITMAP_FONT_TYPE_HELVETICA_12, strMinFilter );
		renderText( 5, 30, BITMAP_FONT_TYPE_HELVETICA_12, strMagFilter );
	}
	endRenderText();

	SwapBuffers( g_hDC );
}
