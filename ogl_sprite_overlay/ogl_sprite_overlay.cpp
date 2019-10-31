//-----------------------------------------------------------------------------
//           Name: ogl_sprite_overlay.cpp
//         Author: Kevin Harris
//  Last Modified: 04/28/05
//    Description: This sample demonstrates how to create animated sprites, 
//                 which are useful for rendering 2D overlays to the screen. 
//                 The sprites manipulate texture coordinates to create 
//                 animations on a simple quad instead using the traditional 
//                 method of bit-blitting straight to video memory.
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <mmsystem.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "resource.h"
#include "sprite.h"
#include "geometry.h"
#include "tga.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND  g_hWnd = NULL;
HDC	  g_hDC  = NULL;
HGLRC g_hRC  = NULL;

int g_nWindowWidth  = 640;
int g_nWindowHeight = 480;

GLuint g_spriteTextureID = -1;

bool   g_bFirstRendering = true;
float  g_fElpasedTime = 0.0f;
double g_dCurTime;
double g_dLastTime;

float g_fSpinX = 0.0f;
float g_fSpinY = 0.0f;

sprite g_donutSprite;
sprite g_numberSprite;

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
void setupViewForSpriteRendering(void);

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
                             "OpenGL - Animated Sprite Overlays",
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

    UnregisterClass( "MY_WINDOWS_CLASS", hInstance );

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
// Name: loadTexture()
// Desc: 
//-----------------------------------------------------------------------------
void loadTexture(void)	
{
    //
    // The animations for both the spinning donut and the numbers are stored 
    // together in a .tga file which has an alpha channel.
    //

    tgaImageFile tgaImage;
    tgaImage.load( "sprites.tga" );

    glGenTextures( 1, &g_spriteTextureID );

    glBindTexture( GL_TEXTURE_2D, g_spriteTextureID );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    glTexImage2D( GL_TEXTURE_2D, 0, tgaImage.m_texFormat, 
                  tgaImage.m_nImageWidth, tgaImage.m_nImageHeight, 
                  0, tgaImage.m_texFormat, GL_UNSIGNED_BYTE, 
                  tgaImage.m_nImageData );
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
    glEnable( GL_DEPTH_TEST );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
    gluPerspective( 45.0, (GLdouble)g_nWindowWidth / (GLdouble)g_nWindowHeight, 0.1, 100.0);

    // Set up a material
    GLfloat ambient_mtrl[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat diffuse_mtrl[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv( GL_FRONT, GL_AMBIENT, ambient_mtrl ); 
    glMaterialfv( GL_FRONT, GL_DIFFUSE, diffuse_mtrl );

    // Set light 0 to be a pure white directional light
    glEnable( GL_LIGHTING );
    glEnable( GL_LIGHT0 );
    GLfloat diffuse_light0[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat specular_light0[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat position_light0[] = { 0.0f, 0.0f, 1.0f, 0.0f };
    glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuse_light0 );
    glLightfv( GL_LIGHT0, GL_SPECULAR, specular_light0 );
    glLightfv( GL_LIGHT0, GL_POSITION, position_light0 );

    // Enable some dim, grey ambient lighting so objects that are not lit 
    // by the other lights are not completely black.
    GLfloat ambient_lightModel[] = { 0.2f, 0.2f, 0.2f, 0.2f };
    glLightModelfv( GL_LIGHT_MODEL_AMBIENT, ambient_lightModel );

    //
    // Load up the sprite texture...
    //

    loadTexture();

    g_donutSprite.setTextureID( g_spriteTextureID );
    g_donutSprite.setPositionX( 0.0f );
    g_donutSprite.setPositionY( 0.0f );
    g_donutSprite.setAlpha( 1.0f );
    g_donutSprite.setWidth( 1.0f );
    g_donutSprite.setHeight( 1.0f );
    g_donutSprite.setFrameDelay( 0.01f );
    g_donutSprite.setTextureAnimeInfo( 512, 512, 64, 64, 5, 6, 30 );

    //-------------------------------------------------------------------------
    //
    // -- Argument explantion for setTextureAnimeInfo() call above -- 
    //
    // From a 512 x 512 texture map we're going to pull a series of frames 
    // that are 64 x 64.
    // 
    // The frames have been laid out in a column/row fashion starting in the 
    // upper left corner with 5 frames in each of the 6 rows.
    //
    // The total frame count is 30 because all rows are fully complete and 
    // contain 5 frames with none of them being short any frames.
    //
    //-------------------------------------------------------------------------

    //
    // Load up the number sprite...
    //

    g_numberSprite.setTextureID( g_spriteTextureID );
    g_numberSprite.setPositionX( -5.0f );
    g_numberSprite.setPositionY( -3.5f );
    g_numberSprite.setAlpha( 1.0f );
    g_numberSprite.setWidth( 0.5f );
    g_numberSprite.setHeight( 0.5f );
    g_numberSprite.setFrameDelay( 1.0f );
    g_numberSprite.setTextureAnimeInfo( 512, 512, 15, 20, 5, 2, 10, 328, 4 );

    //-------------------------------------------------------------------------
    //
    // -- Argument explantion for setTextureAnimeInfo() call above -- 
    //
    // From the same 512 x 512 texture map we're going to pull a series of 
    // frames that are 15 x 20.
    // 
    // The frames have been laid out in a column/row fashion starting in the 
    // upper left corner with 5 frames in each of the 2 rows.
    //
    // The total frame count is 10 because all rows are fully complete and 
    // contain 5 frames with none of them being short any frames.
    //
    // Finally, unlike the donut sprites, the first frame for our number 
    // sprites is not located in the upper left hand corner. This means we 
    // need to provide an x/y offset measured in pixels from the upper left
    // corner so the code that generates the correct texture coordinates for 
    // the sprite can find the first frame.
    //
    //-------------------------------------------------------------------------
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )	
{
    glDeleteTextures( 1, &g_spriteTextureID );

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
// Name: setupViewForSpriteRendering()
// Desc: 
//-----------------------------------------------------------------------------
void setupViewForSpriteRendering( void )
{
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective( 45.0f, (GLdouble)g_nWindowWidth / (GLdouble)g_nWindowHeight, 0.1f, 100.0f );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    // Set up current camera
    gluLookAt( 0.0, 0.0, 10,  // Camera Position
               0.0, 0.0, 0.0, // Look At Point
               0.0, 1.0, 0.0);// Up Vector
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: 
//-----------------------------------------------------------------------------
void render( void )
{
    //
    // Place the donut sprite in motion so we can test the methods of our 
    // sprite class.
    //

    static float velocityX  = 5.0f;
    static float velocityY  = 5.0f;
    static float widthRate  = 2.0f;
    static float heightRate = 2.0f;
    static float alphaRate  = 1.0f;

    g_donutSprite.setPositionX( g_donutSprite.getPositionX() + (velocityX*g_fElpasedTime) );
    g_donutSprite.setPositionY( g_donutSprite.getPositionY() + (velocityY*g_fElpasedTime) );
    g_donutSprite.setWidth( g_donutSprite.getWidth() + (widthRate*g_fElpasedTime) );
    g_donutSprite.setHeight( g_donutSprite.getHeight() + (heightRate*g_fElpasedTime) );
    g_donutSprite.setAlpha( g_donutSprite.getAlpha() + (alphaRate*g_fElpasedTime));

    if( g_donutSprite.getPositionX() > 5.0f || g_donutSprite.getPositionX() < -5.0f )
        velocityX *= -1.0f; // invert direction along the x axis

    if( g_donutSprite.getPositionY() > 3.0f || g_donutSprite.getPositionY() < -3.0f )
        velocityY *= -1.0f; // invert direction along the y axis

    if( g_donutSprite.getWidth() > 5.0f || g_donutSprite.getWidth() < 1.0f )
        widthRate *= -1.0f; // invert change in width

    if( g_donutSprite.getHeight() > 5.0f|| g_donutSprite.getHeight() < 1.0f )
        heightRate *= -1.0f; // invert change in height

    if( g_donutSprite.getAlpha() > 1.0f || g_donutSprite.getAlpha() < 0.0f )
        alphaRate *= -1.0f; // invert change in alpha

    // Perform a boundary check and make sure our sprite hasn't jumped from 
    // the screen duing an update or faded out too much. 
    if( g_donutSprite.getPositionX() > 5.0f )
        g_donutSprite.setPositionX( 5.0f );
        
    if( g_donutSprite.getPositionX() < -5.0f )
        g_donutSprite.setPositionX( -5.0f );

    if( g_donutSprite.getPositionY() > 3.0f )
        g_donutSprite.setPositionY( 3.0f );

    if( g_donutSprite.getPositionY() < -3.0f )
        g_donutSprite.setPositionY( -3.0f );

    if( g_donutSprite.getAlpha() > 1.0f )
        g_donutSprite.setAlpha( 1.0f );

    if( g_donutSprite.getAlpha() < 0.0f )
        g_donutSprite.setAlpha( 0.0f );

    //
    // Render...
    //

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glTranslatef( 0.0f, 0.0f, -5.0f );
    glRotatef( -g_fSpinY, 1.0f, 0.0f, 0.0f );
    glRotatef( -g_fSpinX, 0.0f, 1.0f, 0.0f );

    //
    // First, we'll render a teapot so our scene will have at least something 
    // 3D in it.
    //

    renderSolidTeapot( 1.0f );
    //renderSolidTorus( 0.2, 1.0, 20, 20 );

    //
    // Finally, we render our two animated sprites as overlays...
    //

    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();

    // Set up a perspective projection matrix suitable for rendering sprite
    // overlays
    setupViewForSpriteRendering();

    g_numberSprite.render();
    g_donutSprite.render();

    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();

	SwapBuffers( g_hDC );
}
