//-----------------------------------------------------------------------------
//           Name: dx8_2d_demo_game.cpp
//         Author: Kevin Harris
//  Last Modified: 05/11/04
//    Description: 2D Sample game written in DirectX 8.0
//       Controls: Use the arrow keys to move and the space bar to shoot!
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN
#define INITGUID

#include <windows.h>
#include <mmsystem.h>
#include <ddraw.h>
#include <dinput.h>
#include "ddutil.h"
#include "resource.h"
#include "sprite.h"
#include <list>
using namespace std;

//-----------------------------------------------------------------------------
// SYMBOLIC CONSTANTS
//-----------------------------------------------------------------------------
const int SCREEN_WIDTH    = 800;
const int SCREEN_HEIGHT   = 600;
const int SCREEN_BPP      = 16;

const int TOTAL_STARS     = 100;
const int TOTAL_PARTICLES = 250;
const int SHIP_SPEED      = 5;

//-----------------------------------------------------------------------------
// MACROS
//-----------------------------------------------------------------------------
#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }
#define KEYDOWN(name,key) (name[key] & 0x80)

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
CDisplay *g_pDisplay        = NULL;
CSurface *g_pFighterBitmap  = NULL;  // Off-screen surface for the fighter bitmap.
CSurface *g_pNumbersBitmap  = NULL;  // Off-screen surface for the numbers bitmap.
CSurface *g_ExplosionBitmap = NULL;  // Off-screen surface for the explosion bitmap.
CSurface *g_pMiscBitmap     = NULL;  // Off-screen surface for the misc bitmap.

LPDIRECTINPUT8       g_lpdi;         // DirectInput object
LPDIRECTINPUTDEVICE8 g_pKeyboard;    // DirectInput device

RECT  g_rcWindow;
RECT  g_rcViewport;
RECT  g_rcScreen;
BOOL  g_bWindowed       = true;
BOOL  g_bActive         = false;
DWORD g_dwLastTick      = 0;
DWORD g_dwLastFireTick  = 0;
int   g_nShipsLeft      = 3;
int   g_nScore          = 0;
int   g_nPowerLevel     = 20;

double g_dElpasedTime;
double g_dCurTime;
double g_dLastTime;
double g_dAnimationTimer = 0.0;

typedef struct STAR
{
    int x;          // Star posit x
	int y;          // Star posit y
    int nVelocity;  // Star velocity
	COLORREF color; // Star color
};

STAR stars[TOTAL_STARS]; // Star field array

typedef struct PARTICLE
{
    int  x;          // Particle posit x
	int  y;          // Particle posit y
    int  nVelocity;  // Particle velocity
	bool bVisible;   // Is particle visible or active
	COLORREF color;  // Particle color
};

PARTICLE exhaust[TOTAL_PARTICLES]; // Particle array for engine exhaust

// Create a linked list with STL to hold and manage CSprite objects
typedef list<CSprite> SPRITELIST;

SPRITELIST g_SpriteList;
SPRITELIST::iterator g_sprite_i;
SPRITELIST::iterator g_sprite_j;
SPRITELIST::iterator g_sprite_k;

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
HRESULT WinInit(HINSTANCE hInst, int nCmdShow, HWND* phWnd, HACCEL* phAccel);

HRESULT GameStart(HWND hWnd);
HRESULT GameMain(HWND hWnd);
void    GameOver(void);

HRESULT InitDirectDraw(HWND hWnd, BOOL bWindowed);
void    FreeDirectDraw(void);
HRESULT RestoreSurfaces(void);
HRESULT AttachClipper(LPRECT lpRect);

HRESULT InitDirectInput(HWND hWnd);
void    FreeDirectInput(void);

void    InitializeSprites(void);
void    InitializeStars(void);
void    InitializeExhaust(void);
void    MoveShip(void);
void    MoveSprites(void);
void    MoveStars(void);
void    MoveExhaust(void);
void    CheckCollisions(void);
bool    SpriteCollisionTest(CSprite sprite1, CSprite sprite2);
bool    CollisionTest(int x1, int y1, int w1, int h1, 
					  int x2, int y2, int w2, int h2);
void    ComputeScore(void);
HRESULT DisplayFrame(void);
HRESULT DrawStars(void);
HRESULT DrawExhaust(void);
HRESULT DrawPowerBar(void);
void    WriteToSurface(char *label, int value, int x, int y, 
					   LPDIRECTDRAWSURFACE7 lpdds, bool bValuePassed);
int     RandomInt(int nLow, int nHigh);


//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point to the program. Initializes everything and calls.
//-----------------------------------------------------------------------------
int APIENTRY WinMain( HINSTANCE hInst, 
					  HINSTANCE hPrevInst, 
					  LPSTR     pCmdLine, 
					  int       nCmdShow )
{
    MSG	   msg;
    HWND   hWnd;
    HACCEL hAccel;

    memset(&msg,0,sizeof(msg));
    
    WinInit( hInst, nCmdShow, &hWnd, &hAccel );

	GameStart( hWnd );

    while( TRUE )
    {
        // Look for messages, if none are found then 
        // update the state and display it
        if( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
        {
            if( 0 == GetMessage(&msg, NULL, 0, 0 ) )
            {
                // WM_QUIT was posted, so exit
                return (int)msg.wParam;
            }

            // Translate and dispatch the message
            if( 0 == TranslateAccelerator( hWnd, hAccel, &msg ) )
            {
                TranslateMessage( &msg ); 
                DispatchMessage( &msg );
            }
        }
        else
        {
            if( g_bActive )
            {
                // Move the sprites, blt them to the back buffer, then 
                // flip or blt the back buffer to the primary buffer

				g_dCurTime     = timeGetTime();
				g_dElpasedTime = ((g_dCurTime - g_dLastTime) * 0.001);
				g_dLastTime    = g_dCurTime;

                if( FAILED( GameMain( hWnd ) ) )
                {
                    SAFE_DELETE( g_pDisplay );

                    MessageBox( hWnd, TEXT("GameMain() failed. ")
                                TEXT("The game will now exit. "), TEXT("TestGame"), 
                                MB_ICONERROR | MB_OK );
                    return FALSE;
                }
            }
            else
            {
                // Go to sleep if we have nothing else to do
                WaitMessage();

                // Ignore time spent inactive 
                g_dwLastTick = timeGetTime();
            }
        }
    }
}

//-----------------------------------------------------------------------------
// Name: WinInit()
// Desc: Init our game's window
//-----------------------------------------------------------------------------
HRESULT WinInit( HINSTANCE hInst, int nCmdShow, HWND *phWnd, HACCEL *phAccel )
{
    WNDCLASSEX wc;
    HWND       hWnd;
    HACCEL     hAccel;

    // Register the window class
    wc.cbSize        = sizeof(wc);
    wc.lpszClassName = TEXT("MY_WINDOWS_CLASS");
    wc.lpfnWndProc   = MainWndProc;
    wc.style         = CS_VREDRAW | CS_HREDRAW;
    wc.hInstance     = hInst;
    wc.hIcon         = LoadIcon( hInst, MAKEINTRESOURCE(IDI_MAIN_ICON) );
    wc.hIconSm       = LoadIcon( hInst, MAKEINTRESOURCE(IDI_MAIN_ICON) );
    wc.hCursor       = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU);
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;

    if( RegisterClassEx( &wc ) == 0 )
        return E_FAIL;

    // Load keyboard accelerators
    hAccel = LoadAccelerators( hInst, MAKEINTRESOURCE(IDR_MAIN_ACCEL) );

    // Calculate the proper size for the window given a client of 640x480
    DWORD dwFrameWidth    = GetSystemMetrics( SM_CXSIZEFRAME );
    DWORD dwFrameHeight   = GetSystemMetrics( SM_CYSIZEFRAME );
    DWORD dwMenuHeight    = GetSystemMetrics( SM_CYMENU );
    DWORD dwCaptionHeight = GetSystemMetrics( SM_CYCAPTION );
    DWORD dwWindowWidth   = SCREEN_WIDTH  + dwFrameWidth * 2;
    DWORD dwWindowHeight  = SCREEN_HEIGHT + dwFrameHeight * 2 + 
                            dwMenuHeight + dwCaptionHeight;

    // Create and show the main window
    DWORD dwStyle = WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX;
    hWnd = CreateWindowEx( 0, TEXT("MY_WINDOWS_CLASS"), 
                           TEXT("Direct3D (DX8) - 2D Demo Game"),
                           dwStyle, CW_USEDEFAULT, CW_USEDEFAULT,
  	                       dwWindowWidth, dwWindowHeight, NULL, NULL, hInst, NULL );
    if( hWnd == NULL )
    	return E_FAIL;

    ShowWindow( hWnd, nCmdShow );
    UpdateWindow( hWnd );

    // Save the window size/pos for switching modes
    GetWindowRect( hWnd, &g_rcWindow );

    *phWnd   = hWnd;
    *phAccel = hAccel;

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: MainWndProc()
// Desc: The main window procedure
//-----------------------------------------------------------------------------
LRESULT CALLBACK MainWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
                case IDM_TOGGLEFULLSCREEN:
                    // Toggle the fullscreen/window mode
                    if( g_bWindowed )
                        GetWindowRect( hWnd, &g_rcWindow );

                    g_bWindowed = !g_bWindowed;

                    if( FAILED( InitDirectDraw( hWnd, g_bWindowed ) ) )
                    {
                        SAFE_DELETE( g_pDisplay );

                        MessageBox( hWnd, TEXT("InitDirectDraw() failed. ")
                                    TEXT("The game will now exit. "), TEXT("TestGame"), 
                                    MB_ICONERROR | MB_OK );
                	    PostMessage( hWnd, WM_CLOSE, 0, 0 );
                    }

                    return 0L;

                case IDM_EXIT:
                    // Received key/menu command to exit app
            	    PostMessage( hWnd, WM_CLOSE, 0, 0 );
                    return 0L;
            }
            break;

        case WM_GETMINMAXINFO:
            {
                // Don't allow resizing in windowed mode.  
                // Fix the size of the window to 640x480 (client size)
                MINMAXINFO* pMinMax = (MINMAXINFO*) lParam;

                DWORD dwFrameWidth    = GetSystemMetrics( SM_CXSIZEFRAME );
                DWORD dwFrameHeight   = GetSystemMetrics( SM_CYSIZEFRAME );
                DWORD dwMenuHeight    = GetSystemMetrics( SM_CYMENU );
                DWORD dwCaptionHeight = GetSystemMetrics( SM_CYCAPTION );

                pMinMax->ptMinTrackSize.x = SCREEN_WIDTH  + dwFrameWidth * 2;
                pMinMax->ptMinTrackSize.y = SCREEN_HEIGHT + dwFrameHeight * 2 + 
                                            dwMenuHeight + dwCaptionHeight;

                pMinMax->ptMaxTrackSize.x = pMinMax->ptMinTrackSize.x;
                pMinMax->ptMaxTrackSize.y = pMinMax->ptMinTrackSize.y;
            }
            return 0L;

        case WM_MOVE:
            // Retrieve the window position after a move.  
            if( g_pDisplay )
                g_pDisplay->UpdateBounds();

            return 0L;

        case WM_SIZE:
            // Check to see if we are losing our window...
            if( SIZE_MAXHIDE == wParam || SIZE_MINIMIZED == wParam )
                g_bActive = FALSE;
            else
                g_bActive = TRUE;

            if( g_pDisplay )
                g_pDisplay->UpdateBounds();
            break;

        case WM_SETCURSOR:
            // Hide the cursor if in full screen 
            if( !g_bWindowed )
            {
                SetCursor( NULL );
                return TRUE;
            }
            break;

        case WM_QUERYNEWPALETTE:
            if( g_pDisplay && g_pDisplay->GetFrontBuffer() )            
            {
                // If we are in windowed mode with a desktop resolution in 8 bit 
                // color, then the palette we created during init has changed 
                // since then.  So get the palette back from the primary 
                // DirectDraw surface, and set it again so that DirectDraw 
                // realizes the palette, then release it again. 
                LPDIRECTDRAWPALETTE pDDPal = NULL; 
                g_pDisplay->GetFrontBuffer()->GetPalette( &pDDPal );
                g_pDisplay->GetFrontBuffer()->SetPalette( pDDPal );
                SAFE_RELEASE( pDDPal );
            }
            break;

        case WM_EXITMENULOOP:
            // Ignore time spent in menu
            g_dwLastTick = timeGetTime();
            break;

        case WM_EXITSIZEMOVE:
            // Ignore time spent resizing
            g_dwLastTick = timeGetTime();
            break;

        case WM_SYSCOMMAND:
            // Prevent moving/sizing and power loss in full screen mode
            switch( wParam )
            {
                case SC_MOVE:
                case SC_SIZE:
                case SC_MAXIMIZE:
                case SC_MONITORPOWER:
                    if( !g_bWindowed )
                        return TRUE;
            }
            break;
            
		case WM_ACTIVATE:
            if( WA_INACTIVE != wParam && g_pKeyboard )
            {
                // Make sure the device is acquired, if we are gaining focus.
                g_pKeyboard->Acquire();
				//g_bActive = true; //?
            }
			     
			/*
			if( WA_INACTIVE == wParam )
            {
				//Losing focus...
				g_bActive = false; //?
            }
			*/

            break;

        case WM_DESTROY:
            // Cleanup and close the app
            GameOver();
            PostQuitMessage( 0 );
            return 0L;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

//-----------------------------------------------------------------------------
// Name: GameStart()
// Desc: Initialize all DirectX objects to be used
//-----------------------------------------------------------------------------
HRESULT GameStart( HWND hWnd )
{
	HRESULT hr;

	// Initialize all the surfaces we need
    if( FAILED( hr = InitDirectDraw( hWnd, g_bWindowed ) ) )
        return hr;

	// Initialize all DirectInput
    if( FAILED( hr = InitDirectInput( hWnd ) ) )
        return hr;

	InitializeSprites();

	InitializeStars();

	InitializeExhaust();

    g_dwLastTick = timeGetTime();
	g_dwLastFireTick = timeGetTime();

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: GameMain()
// Desc: Move the sprites, blt them to the back buffer, then 
//       flip or blt the back buffer to the primary buffer
//-----------------------------------------------------------------------------
HRESULT GameMain( HWND hWnd )
{
    HRESULT hr;

	g_dAnimationTimer += g_dElpasedTime;

	if( g_dAnimationTimer >= 0.016 )
		g_dAnimationTimer = 0.0; // Target of 1/60th of a second (60 FPS) reached. Render a new frame.
	else
		return S_OK; // It's too early. Return now and render nothing.

    // Check the cooperative level before rendering
    if( FAILED( hr = g_pDisplay->GetDirectDraw()->TestCooperativeLevel() ) )
    {
        switch( hr )
        {
            case DDERR_EXCLUSIVEMODEALREADYSET:
            case DDERR_NOEXCLUSIVEMODE:
                // Do nothing because some other app has exclusive mode
                Sleep(10);
                return S_OK;

            case DDERR_WRONGMODE:
                // The display mode changed on us. Update the
                // DirectDraw surfaces accordingly
                return InitDirectDraw( hWnd, g_bWindowed );
        }
        return hr;
    }

	// Collect user input and move the player's ship
	MoveShip();

	// Move the game sprites
	MoveSprites();

	// Check to see who's hitting who and react to it
	CheckCollisions();

	// Assign the proper number sprites for the score
	ComputeScore();

    // Display the sprites on the screen
    if( FAILED( hr = DisplayFrame() ) )
    {
        if( hr != DDERR_SURFACELOST )
            return hr;

        // The surfaces were lost, so restore them 
        RestoreSurfaces();
    }

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: GameOver()
// Desc: Release all the DirectDraw objects
//-----------------------------------------------------------------------------
VOID GameOver()
{
	FreeDirectDraw();

	FreeDirectInput();

	for( g_sprite_i = g_SpriteList.begin(); g_sprite_i != g_SpriteList.end(); ++g_sprite_i )
		 g_sprite_i->releaseMemory();

	// Cleanup the STL generated linked list
	g_SpriteList.erase( g_SpriteList.begin(), g_SpriteList.end() );
}

//-----------------------------------------------------------------------------
// Name: InitDirectDraw()
// Desc: Called when the user wants to toggle between full-screen and windowed 
//       to create all the needed DDraw surfaces and set the coop level
//-----------------------------------------------------------------------------
HRESULT InitDirectDraw( HWND hWnd, BOOL bWindowed )
{
	HRESULT hr;

	FreeDirectDraw();

    // The back buffer and primary surfaces need to be created differently 
    // depending on if we are in full-screen or windowed mode
    g_pDisplay = new CDisplay();

    if( bWindowed )
    {
        g_pDisplay->CreateWindowedDisplay( hWnd, SCREEN_WIDTH, SCREEN_HEIGHT );

        // Add the system menu to the window's style
        DWORD dwStyle = GetWindowLong( hWnd, GWL_STYLE );
        dwStyle |= WS_SYSMENU;
        SetWindowLong( hWnd, GWL_STYLE, dwStyle );

        // Show the menu in windowed mode 
#ifdef _WIN64
        HINSTANCE hInst = (HINSTANCE) GetWindowLongPtr( hWnd, GWLP_HINSTANCE );
#else
        HINSTANCE hInst = (HINSTANCE) GetWindowLong( hWnd, GWL_HINSTANCE );
#endif
        HMENU hMenu = LoadMenu( hInst, MAKEINTRESOURCE( IDR_MENU ) );
        SetMenu( hWnd, hMenu );
    }
    else
    {
        g_pDisplay->CreateFullScreenDisplay( hWnd, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP );

		g_pDisplay->InitClipper();

        // Disable the menu in full-screen since we are 
        // using a palette and a menu would look bad 
        SetMenu( hWnd, NULL );

        // Remove the system menu from the window's style
        DWORD dwStyle = GetWindowLong( hWnd, GWL_STYLE );
        dwStyle &= ~WS_SYSMENU;
        SetWindowLong( hWnd, GWL_STYLE, dwStyle );       
    }

	if( FAILED( hr = g_pDisplay->CreateSurfaceFromBitmap( &g_pFighterBitmap, "fighter.bmp", 0, 0 ) ) )
    return hr;

	if( FAILED( hr = g_pDisplay->CreateSurfaceFromBitmap( &g_pNumbersBitmap, "numbers.bmp", 0, 0 ) ) )
    return hr;

	if( FAILED( hr = g_pDisplay->CreateSurfaceFromBitmap( &g_ExplosionBitmap, "explosion.bmp", 0, 0 ) ) )
    return hr;

	if( FAILED( hr = g_pDisplay->CreateSurfaceFromBitmap( &g_pMiscBitmap, "misc.bmp", 0, 0 ) ) )
    return hr;

    // Set the color key for the ship sprite to black
    if( FAILED( g_pFighterBitmap->SetColorKey( 0 ) ) )
        return E_FAIL;

    // Set the color key for the number sprite to black
    if( FAILED( g_pNumbersBitmap->SetColorKey( 0 ) ) )
        return E_FAIL;

    // Set the color key for the explode sprite to black
    if( FAILED( g_ExplosionBitmap->SetColorKey( 0 ) ) )
        return E_FAIL;

    // Set the color key for the shot sprite to black
    if( FAILED( g_pMiscBitmap->SetColorKey( 0 ) ) )
        return E_FAIL;

	RECT rcScreen = {0,0,SCREEN_WIDTH,SCREEN_HEIGHT};
	AttachClipper( &rcScreen );

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: FreeDirectDraw()
// Desc: Free all DirectDraw objects
//-----------------------------------------------------------------------------
void FreeDirectDraw(void)
{
    // Release all existing surfaces
	SAFE_DELETE( g_pFighterBitmap );
	SAFE_DELETE( g_pNumbersBitmap );
	SAFE_DELETE( g_ExplosionBitmap );
	SAFE_DELETE( g_pMiscBitmap );
    SAFE_DELETE( g_pDisplay );
}

//-----------------------------------------------------------------------------
// Name: RestoreSurfaces()
// Desc: Restore all the surfaces, and redraw the sprite surfaces.
//-----------------------------------------------------------------------------
HRESULT RestoreSurfaces(void)
{
	HRESULT hr;

	if( FAILED( hr = g_pDisplay->GetDirectDraw()->RestoreAllSurfaces() ) )
    return hr;

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: AttachClipper()
// Desc: Create a DirectDraw clipper and attach it to the back buffer.
//-----------------------------------------------------------------------------
HRESULT AttachClipper( LPRECT lpRect )
{                        
	LPDIRECTDRAWCLIPPER lpddClipper;
	LPRGNDATA lpRgnd;
	HRESULT hr;

	// first create the direct draw clipper
	if(FAILED( hr = g_pDisplay->GetDirectDraw()->CreateClipper( 0, &lpddClipper, NULL) ))
	   return hr;

	// Allocate memory for our region data
	lpRgnd = (LPRGNDATA)malloc( sizeof(RGNDATAHEADER) + sizeof(RECT) );

	// now copy the rect into region data
	memcpy( lpRgnd->Buffer, lpRect, sizeof(RECT) );

	// Fill in the region data header
	lpRgnd->rdh.dwSize          = sizeof(RGNDATAHEADER);
	lpRgnd->rdh.nRgnSize        = sizeof(RECT);
	lpRgnd->rdh.iType           = RDH_RECTANGLES;
	lpRgnd->rdh.nCount          = 1;
	lpRgnd->rdh.rcBound.left    = lpRect->left;
	lpRgnd->rdh.rcBound.top     = lpRect->top;
	lpRgnd->rdh.rcBound.right   = lpRect->right;
	lpRgnd->rdh.rcBound.bottom  = lpRect->bottom;

	// Set the clipping list
	if(FAILED( hr = lpddClipper->SetClipList( lpRgnd, 0 )))
	{
	   free(lpRgnd);
	   return hr;
	}

	// Attach the clipper to the surface
	if(FAILED( hr = g_pDisplay->GetBackBuffer()->SetClipper( lpddClipper )))
	{
	   free(lpRgnd);
	   return hr;
	}

	// Release allocated memory!
	free( lpRgnd );

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: InitDirectInput()
// Desc: Creates a keyboard device using DirectInput
//-----------------------------------------------------------------------------
HRESULT InitDirectInput( HWND hWnd )
{
	HRESULT hr;

    FreeDirectInput();

    // Create a DInput object
    if( FAILED( hr = DirectInput8Create( GetModuleHandle(NULL), DIRECTINPUT_VERSION, 
                                         IID_IDirectInput8, (VOID**)&g_lpdi, NULL ) ) )
        return hr;
    
    // Obtain an interface to the system keyboard device.
    if( FAILED( hr = g_lpdi->CreateDevice( GUID_SysKeyboard, &g_pKeyboard, NULL ) ) )
        return hr;
    
    // Set the data format to "keyboard format" - a predefined data format 
    //
    // A data format specifies which controls on a device we
    // are interested in, and how they should be reported.
    //
    // This tells DirectInput that we will be passing an array
    // of 256 bytes to IDirectInputDevice::GetDeviceState.
    if( FAILED( hr = g_pKeyboard->SetDataFormat( &c_dfDIKeyboard ) ) )
        return hr;
    
    // Set the cooperative level to let DirectInput know how
    // this device should interact with the system and with other
    // DirectInput applications.
    hr = g_pKeyboard->SetCooperativeLevel( hWnd, DISCL_NOWINKEY | DISCL_NONEXCLUSIVE | DISCL_FOREGROUND );
    if( hr == DIERR_UNSUPPORTED )
    {
        FreeDirectInput();
        MessageBox( hWnd, TEXT("SetCooperativeLevel() returned DIERR_UNSUPPORTED.\n")
                          TEXT("For security reasons, background exclusive keyboard\n")
                          TEXT("access is not allowed."), TEXT("TestGame"), MB_OK );
        return S_OK;
    }

    if( FAILED(hr) )
        return hr;

    // Acquire the newly created device
    g_pKeyboard->Acquire();

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: FreeDirectInput()
// Desc: Initialize the DirectInput variables.
//-----------------------------------------------------------------------------
void FreeDirectInput(void)
{
    // Unacquire the device one last time just in case 
    // the app tried to exit while the device is still acquired.
    if( g_pKeyboard ) 
        g_pKeyboard->Unacquire();
    
    // Release any DirectInput objects.
    SAFE_RELEASE( g_pKeyboard );
    SAFE_RELEASE( g_lpdi );
}

//-----------------------------------------------------------------------------
// Name: InitializeSprites()
// Desc: Creates each sprite, sets their properties, and then loads them 
//       into a linked list for future use.
//-----------------------------------------------------------------------------
void InitializeSprites(void)
{
	CSprite sprite;
	int i = 0;

	sprite.zeroSpriteValues();
	sprite.m_nID                = 555;
	strcpy( sprite.m_chType, "ship" );
	strcpy( sprite.m_chBitmap, "fighter.bmp" );
	sprite.m_dPosition_x        = 0;
	sprite.m_dPosition_y        = 250;
	sprite.m_bAutoAnimate       = false; // We'll control the animation or selves through incFrame() and decFrame()
	sprite.m_nFrameRateModifier = -2;
	sprite.m_nCurrentFrame      = 14;  // Point forward until further notice!
	sprite.m_nFrameWidth        = 144;
	sprite.m_nFrameHeight       = 108;
	sprite.m_nFramesAcross      = 8;
	sprite.m_nWidthScaling      = -30; // Shrink the sprite's width
	sprite.m_nHeightScaling     = -30; // Shrink the sprite's height
	sprite.m_bModifyCollision   = true;
	sprite.m_nCollisionRight    = -5;  // Reduce sprite 's collision area on the right side
	sprite.m_nCollisionLeft     = -15; // Reduce sprite 's collision area on the left side
	sprite.m_nCollisionBottom   = -50; // Reduce sprite 's collision area on the bottom
	sprite.m_nCollisionTop      = -20; // Reduce sprite 's collision area on the top
	sprite.loadAnimationString( 0, 
		"7 7 6 6 5 5 4 4 3 3 2 2 1 1 0 0 9 9 10 10 11 11 12 12 13 13 14 14 15 15", 
		CSprite::MAINTAIN_LAST_FRAME );

	g_SpriteList.push_back(sprite);

	// Create an animated sprite for engine exhaust
	sprite.zeroSpriteValues();
	strcpy( sprite.m_chType, "animeflame" );
	strcpy( sprite.m_chBitmap, "misc.bmp" );
	sprite.m_bActive            = false;
	sprite.m_dPosition_x        = 0;
	sprite.m_dPosition_y        = 0;
	sprite.m_nFrameRateModifier = -10;
	sprite.m_nFrameOffset_x     = 0;
	sprite.m_nFrameOffset_y     = 74;
	sprite.m_nFramesAcross      = 3;
	sprite.m_nFrameWidth        = 46;
	sprite.m_nFrameHeight       = 13;
	sprite.loadAnimation( 0, 0, 5, CSprite::MAINTAIN_LAST_FRAME );

	g_SpriteList.push_back(sprite);

	// Create a sprite for keeping track of the number 
	// of ships the player has left
	sprite.zeroSpriteValues();
	strcpy( sprite.m_chType, "shipcount" );
	strcpy( sprite.m_chBitmap, "misc.bmp" );
	sprite.m_bSingleFrame       = true;
	sprite.m_dPosition_x        = 375;
	sprite.m_dPosition_y        = 5;
	sprite.m_nFrameOffset_x     = 0;
	sprite.m_nFrameOffset_y     = 17;
	sprite.m_nFrameWidth        = 67;
	sprite.m_nFrameHeight       = 38;
	
	g_SpriteList.push_back(sprite);

	// Create a single number sprite for keeping track of 
	// the number of ships the player has left
	sprite.zeroSpriteValues();
	strcpy( sprite.m_chType, "shipnumber" );
	strcpy( sprite.m_chBitmap, "numbers.bmp" );
	sprite.m_dPosition_x        = 428;
	sprite.m_dPosition_y        = 28;
	sprite.m_bAutoAnimate       = false;
	sprite.m_nFrameWidth        = 17;
	sprite.m_nFrameHeight       = 17;
	sprite.m_nFramesAcross      = 10;
	sprite.m_nFrameOffset_x     =  9;
	sprite.m_nFrameOffset_y     =  5;
	sprite.loadAnimation( 0, 0, 9, CSprite::LOOP_ANIMATION );

	g_SpriteList.push_back(sprite);

	// Create power bar sprite
	sprite.zeroSpriteValues();
	strcpy( sprite.m_chType, "powerbar" );
	strcpy( sprite.m_chBitmap, "misc.bmp" );
	sprite.m_bActive            = false;
	sprite.m_bSingleFrame       = true;
	sprite.m_nFrameOffset_x     = 0;
	sprite.m_nFrameOffset_y     = 56;
	sprite.m_nFrameWidth        = 105;
	sprite.m_nFrameHeight       = 17;

	g_SpriteList.push_back(sprite);

	// Create a red hashmark for the power level display
	sprite.zeroSpriteValues();
	strcpy( sprite.m_chType, "redmark" );
	strcpy( sprite.m_chBitmap, "misc.bmp" );
	sprite.m_bActive            = false;
	sprite.m_bSingleFrame       = true;
	sprite.m_nFrameOffset_x     = 51;
	sprite.m_nFrameOffset_y     = 0;
	sprite.m_nFrameWidth        = 4;
	sprite.m_nFrameHeight       = 13;

	g_SpriteList.push_back(sprite);

	// Create a yellow hashmark for the power level display
	sprite.zeroSpriteValues();
	strcpy( sprite.m_chType, "yellowmark" );
	strcpy( sprite.m_chBitmap, "misc.bmp" );
	sprite.m_bActive            = false;
	sprite.m_bSingleFrame       = true;
	sprite.m_nFrameOffset_x     = 56;
	sprite.m_nFrameOffset_y     = 0;
	sprite.m_nFrameWidth        = 4;
	sprite.m_nFrameHeight       = 13;
	
	g_SpriteList.push_back(sprite);

	// Create a green hashmark for the power level display
	sprite.zeroSpriteValues();
	strcpy( sprite.m_chType, "greenmark" );
	strcpy( sprite.m_chBitmap, "misc.bmp" );
	sprite.m_bActive            = false;
	sprite.m_bSingleFrame       = true;
	sprite.m_nFrameOffset_x     = 61;
	sprite.m_nFrameOffset_y     = 0;
	sprite.m_nFrameWidth        = 4;
	sprite.m_nFrameHeight       = 13;

	g_SpriteList.push_back(sprite);

	// Create 4 numbers for keeping score
	for( i = 0; i < 4; i++ )
	{
		sprite.zeroSpriteValues();
		strcpy( sprite.m_chType, "numbers" );
		strcpy( sprite.m_chBitmap, "numbers.bmp" );
		sprite.m_nState             = i;
		sprite.m_dPosition_x        = 300 + (i * 17);
		sprite.m_dPosition_y        = 5;
		sprite.m_bAutoAnimate       = false;
		sprite.m_nFrameOffset_x     =  9;
		sprite.m_nFrameOffset_y     =  5;
		sprite.m_nFramesAcross      = 10;
		sprite.m_nFrameWidth        = 17;
		sprite.m_nFrameHeight       = 17;
		sprite.loadAnimationString( 0, "0 1 2 3 4 5 6 7 8 9", CSprite::LOOP_ANIMATION );

		g_SpriteList.push_back(sprite);
	}

	// Create 20 mines to shoot at!
	for( i = 0; i < 20; i++ )
	{
		sprite.zeroSpriteValues();
		strcpy( sprite.m_chType, "mine" );
		strcpy( sprite.m_chBitmap, "misc.bmp" );
		sprite.m_bSingleFrame      = true;
		sprite.m_dPosition_x       = 800;
		sprite.m_dPosition_y       = RandomInt( 0, 560 );
		sprite.m_dVelocity_x       = -(RandomInt( 1, 10 ));
		sprite.m_nFrameWidth       = 16;
		sprite.m_nFrameHeight      = 16;
		sprite.m_nFrameOffset_x    = 16;
		sprite.m_nFrameOffset_y    = 1;

		g_SpriteList.push_back(sprite);
	}

	// Create 20 laser shots for blowing up mines!
	for( i = 0; i < 20; i++ )
	{
		sprite.zeroSpriteValues();
		strcpy( sprite.m_chType, "shot" );
		strcpy( sprite.m_chBitmap, "misc.bmp" );
		sprite.m_bActive            = false;
		sprite.m_bSingleFrame       = true;
		sprite.m_nFrameOffset_x     = 32;
		sprite.m_nFrameOffset_y     = 0;
		sprite.m_nFrameWidth        = 18;
		sprite.m_nFrameHeight       = 3;
		
		g_SpriteList.push_back(sprite);
	}

	// Create 20 explosions to cover each possible laser impact with a mine!
	for( i = 0; i < 20; i++ )
	{
		sprite.zeroSpriteValues();
		strcpy( sprite.m_chType, "explosion" );
		strcpy( sprite.m_chBitmap, "explosion.bmp" );
		sprite.m_bActive            = false;
		sprite.m_nFramesAcross      = 7;
		sprite.m_nFrameWidth        = 42;
		sprite.m_nFrameHeight       = 36;
		sprite.m_nFrameRateModifier = -3;
		sprite.loadAnimation( 0, 0, 13, CSprite::GO_INACTIVE );

		g_SpriteList.push_back(sprite);
	}
}

//-----------------------------------------------------------------------------
// Name: InitializeStars()
// Desc: Give each star a random starting position and velocity
//-----------------------------------------------------------------------------
void InitializeStars(void)
{
	for( int i = 0; i < TOTAL_STARS; i++ )
	{
		stars[i].x = rand()%SCREEN_WIDTH;
		stars[i].y = rand()%SCREEN_HEIGHT;
		stars[i].nVelocity = 1 + rand()%16;
		stars[i].color = RGB(255,255,255);
	}
}

//-----------------------------------------------------------------------------
// Name: InitializeExhaust()
// Desc: Initialize all exhaust particles
//-----------------------------------------------------------------------------
void InitializeExhaust(void)
{
	for( int i = 0; i < TOTAL_PARTICLES; i++ )
	{
		exhaust[i].x = 0;
		exhaust[i].y = 0;
		exhaust[i].nVelocity = 1 + rand()%16;
		exhaust[i].color = 700;
		exhaust[i].bVisible = false;
	}
}

//-----------------------------------------------------------------------------
// Name: MoveShip()
// Desc: Collects user input and acts on it by modifying the ship sprite
//-----------------------------------------------------------------------------
void MoveShip(void)
{
	char buffer[256];
	int  nShips_x = 0;
	int  nShips_y = 0;
	bool bFlameOn = false;

	// Get keyboard input 
	g_pKeyboard->GetDeviceState(sizeof(buffer),(LPVOID)&buffer); 

	// Find the ship and modify it...
	for( g_sprite_i = g_SpriteList.begin(); g_sprite_i != g_SpriteList.end(); ++g_sprite_i )
	{
		if( !lstrcmp(g_sprite_i->m_chType, "ship") )
		{
			if( KEYDOWN(buffer, DIK_UP) )
			{
				if( g_sprite_i->m_dPosition_y > 0 )
					g_sprite_i->m_dPosition_y -= SHIP_SPEED;

				if( g_sprite_i->m_nCurrentFrame < 28 )
					g_sprite_i->incFrame();
			}
			else if( KEYDOWN(buffer, DIK_DOWN) )
			{
				if( g_sprite_i->m_dPosition_y < 520 )
					g_sprite_i->m_dPosition_y += SHIP_SPEED;

				if( g_sprite_i->m_nCurrentFrame > 0 )
					g_sprite_i->decFrame();
			}
			else
			{
				if( g_sprite_i->m_nCurrentFrame > 14)
					g_sprite_i->decFrame();

				if( g_sprite_i->m_nCurrentFrame < 14)
					g_sprite_i->incFrame();
			}

			if( KEYDOWN(buffer, DIK_LEFT) )
			{
				if( g_sprite_i->m_dPosition_x > 0 )
					g_sprite_i->m_dPosition_x -= SHIP_SPEED;
			}

			if( KEYDOWN(buffer, DIK_RIGHT) )
			{
				if( g_sprite_i->m_dPosition_x < (SCREEN_WIDTH - 144))
					g_sprite_i->m_dPosition_x += SHIP_SPEED;

				nShips_x = g_sprite_i->m_dPosition_x;
				nShips_y = g_sprite_i->m_dPosition_y;

				bFlameOn = true;
			}

			if( KEYDOWN(buffer, DIK_SPACE) )
			{
				///*
				// Single shot laser fire
				for( g_sprite_j = g_SpriteList.begin(); g_sprite_j != g_SpriteList.end(); ++g_sprite_j )
				{
					if( !lstrcmp(g_sprite_j->m_chType, "shot") && g_sprite_j->m_bActive == false )
					{
						g_sprite_j->m_dPosition_x = ( g_sprite_i->m_dPosition_x + 90 );
						g_sprite_j->m_dPosition_y = ( g_sprite_i->m_dPosition_y + 35 );
						g_sprite_j->m_dVelocity_x = 20;
						g_sprite_j->m_bActive = true;
						break;
					}
				}
				//*/

				/*
				// Triple shot laser fire

				// Figure how much time has passed since the last tri-shot
				DWORD dwCurrTick = timeGetTime();
				DWORD dwTickDiff = dwCurrTick - g_dwLastFireTick;
				int nShots = 0;

				if( dwTickDiff >= 75 )
				{
					for( g_sprite_j = g_SpriteList.begin(); g_sprite_j != g_SpriteList.end(); ++g_sprite_j )
					{
						if( !lstrcmp(g_sprite_j->m_chType, "shot") && g_sprite_j->m_bActive == false )
						{
							g_sprite_j->m_dPosition_x = ( g_sprite_i->m_dPosition_x + 90 );
							g_sprite_j->m_dPosition_y = ( g_sprite_i->m_dPosition_y + 35 );
							g_sprite_j->m_bActive = true;

							if( nShots == 0 )
								g_sprite_j->dVelocity_x = 20;

							if( nShots == 1 )
							{
								g_sprite_j->m_dVelocity_x = 20;
								g_sprite_j->m_dVelocity_y = 2;
							}

							if( nShots == 2 )
							{
								g_sprite_j->m_dVelocity_x = 20;
								g_sprite_j->m_dVelocity_y = -2;
							}

							++nShots;
							if( nShots == 3 )
								break;
						}
					}

					g_dwLastFireTick = dwCurrTick;
				}
				*/
			}
			break;
			
		}
	}

	for( g_sprite_i = g_SpriteList.begin(); g_sprite_i != g_SpriteList.end(); ++g_sprite_i )
	{
		if( !lstrcmp(g_sprite_i->m_chType, "animeflame") )
		{
			if( bFlameOn == true )
			{
				g_sprite_i->m_bActive = true;
				g_sprite_i->m_dPosition_x = (nShips_x - 40);
				g_sprite_i->m_dPosition_y = (nShips_y + 30);

				for( int i = 0; i< TOTAL_PARTICLES; i++ )
				{
					if( exhaust[i].bVisible == false )
					{
						int nCenter = (nShips_y + 36);
						exhaust[i].x = nShips_x;
						exhaust[i].y = RandomInt( (nCenter - 10), (nCenter + 10) );
						exhaust[i].bVisible = true;
						break;
					}
				}

				bFlameOn = false;
			}
			else
			{
				g_sprite_i->m_bActive = false;
				g_sprite_i->m_nCurrentFrame = 0;

			}
			break;
		}
	}
}

//-----------------------------------------------------------------------------
// Name: MoveSprites()
// Desc: Identifies sprites in the linked list and moves each one accordingly
//-----------------------------------------------------------------------------
void MoveSprites()
{
	// Move all active player shots
	
	for( g_sprite_i = g_SpriteList.begin(); g_sprite_i != g_SpriteList.end(); ++g_sprite_i )
	{
		if( !lstrcmp(g_sprite_i->m_chType, "shot") && g_sprite_i->m_bActive == true )
		{
			if( g_sprite_i->m_dPosition_x < SCREEN_WIDTH )
			{
				g_sprite_i->m_dPosition_x += g_sprite_i->m_dVelocity_x;
				g_sprite_i->m_dPosition_y += g_sprite_i->m_dVelocity_y;
			}
			else
			{
				// Recycle player shots that have left the viewable area!
				g_sprite_i->m_bActive = false;
				g_sprite_i->m_dPosition_x = 0;
				g_sprite_i->m_dPosition_y = 0;
				g_sprite_i->m_dVelocity_x = 0;
				g_sprite_i->m_dVelocity_y = 0;
			}
		}

		if( !lstrcmp(g_sprite_i->m_chType, "mine")  )
		{
			g_sprite_i->m_dPosition_x += g_sprite_i->m_dVelocity_x;
			g_sprite_i->m_dPosition_y += g_sprite_i->m_dVelocity_y;

			if( g_sprite_i->m_dPosition_x > 800 )
				g_sprite_i->m_dPosition_x = 0;

			if( g_sprite_i->m_dPosition_x < -64 )
			{
				g_sprite_i->m_dPosition_x = 800;
				g_sprite_i->m_dPosition_y = RandomInt( 0, 560 );
				g_sprite_i->m_dVelocity_x = -(RandomInt( 1, 10 ));
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Name: MoveStars()
// Desc: Update each star's position according to its velocity
//-----------------------------------------------------------------------------
void MoveStars(void)
{
	// Scroll stars to the left
	for( int i = 0; i< TOTAL_STARS; i++ )
	{
		// Move the star
		stars[i].x -= stars[i].nVelocity;

		// If the star falls off the screen's edge, wrap it around
		if( stars[i].x <= 0 )
			stars[i].x = SCREEN_WIDTH;
	}
}

//-----------------------------------------------------------------------------
// Name: MoveExhaust()
// Desc: Move all visible exhaust particles to the right
//-----------------------------------------------------------------------------
void MoveExhaust(void)
{
	// Move particles to the left
	for( int i = 0; i < TOTAL_PARTICLES; i++ )
	{
		// Move the particle
		if( exhaust[i].bVisible == true )
		{
			exhaust[i].x -= exhaust[i].nVelocity;

			if( exhaust[i].x <= 0 )
			{
				exhaust[i].x = 1;
				exhaust[i].bVisible = false;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Name: CheckCollisions()
// Desc: Search the sprite linked-list for sprites that touch
//-----------------------------------------------------------------------------
void CheckCollisions(void)
{
	// Perfrom collision detection 
	int nCount1 = 0;
	int nCount2 = 0;
	int size = g_SpriteList.size();

	for( g_sprite_i = g_SpriteList.begin(); g_sprite_i != g_SpriteList.end(); ++g_sprite_i )
	{
		++nCount1; // Next sprite to process

		for( g_sprite_j = g_SpriteList.begin(); g_sprite_j != g_SpriteList.end(); ++g_sprite_j )
		{
			// Don't check g_sprite_i against sprites that it's already been checked against
			while( nCount2 != nCount1 )
			{
				++g_sprite_j; // Don't check sprites that've already been checked.
				++nCount2;    // Skip ahead...
			}

			// Don't bother checking the last sprite...
			if( nCount2 == size )
				break;

			if( g_sprite_i->m_bCollide == true &&
				g_sprite_j->m_bCollide == true &&
				g_sprite_i->m_bActive  == true &&
				g_sprite_j->m_bActive  == true)
			{
				if( SpriteCollisionTest( *g_sprite_i, *g_sprite_j ) )
				{
					// Has a mine hit a shot?
					if( !lstrcmp(g_sprite_i->m_chType, "mine") && !lstrcmp(g_sprite_j->m_chType, "shot") )
					{
						g_nScore += 5;

						// Kill shot
						g_sprite_j->m_bActive = false;

						// Find an explosion sprite to assign
						for( g_sprite_k = g_SpriteList.begin(); g_sprite_k != g_SpriteList.end(); ++g_sprite_k )
						{
							if( !lstrcmp(g_sprite_k->m_chType, "explosion") && g_sprite_k->m_bActive == false )
							{
								// Assign explosion to the collided mine's position and speed
								g_sprite_k->m_dPosition_x = (g_sprite_i->m_dPosition_x - 10);
								g_sprite_k->m_dPosition_y = (g_sprite_i->m_dPosition_y - 20);
								g_sprite_k->m_dVelocity_x = g_sprite_i->m_dVelocity_x;
								g_sprite_k->m_bActive = true;

								// Place the mine into a waiting off-screen until the script engine using it again
								g_sprite_i->m_dPosition_x = -100;
								g_sprite_i->m_dPosition_y = -100;
								g_sprite_i->m_dVelocity_x = 0;
								g_sprite_i->m_dVelocity_y = 0;
								g_sprite_i->m_bScripting = false;
								break;
							}
						}
					}

					// Has a shot hit a mine?
					if( !lstrcmp(g_sprite_i->m_chType, "shot") && !lstrcmp(g_sprite_j->m_chType, "mine") )
					{
						g_nScore += 5;

						// Kill shot
						g_sprite_i->m_bActive = false;

						// Find an explosion sprite to assign
						for( g_sprite_k = g_SpriteList.begin(); g_sprite_k != g_SpriteList.end(); ++g_sprite_k )
						{
							if( !lstrcmp(g_sprite_k->m_chType, "explosion") && g_sprite_k->m_bActive == false )
							{
								// Assign explosion to the collided mine's position and speed
								g_sprite_k->m_dPosition_x = (g_sprite_j->m_dPosition_x - 10);
								g_sprite_k->m_dPosition_y = (g_sprite_j->m_dPosition_y - 20);
								g_sprite_k->m_dVelocity_x = g_sprite_j->m_dVelocity_x;
								g_sprite_k->m_bActive = true;

								// Place the mine into a waiting off-screen
								g_sprite_j->m_dPosition_x = -100;
								g_sprite_j->m_dPosition_y = -100;
								g_sprite_j->m_dVelocity_x = 0;
								g_sprite_j->m_dVelocity_y = 0;
								break;
							}
						}
					}

					// Has the ship hit a mine
					if( !lstrcmp(g_sprite_i->m_chType, "ship") && !lstrcmp(g_sprite_j->m_chType, "mine") )
					{
						if( g_nPowerLevel != 0 )
							--g_nPowerLevel;

						if( g_nPowerLevel == 0 )
						{
							if( g_nShipsLeft != 0 )
								--g_nShipsLeft;

							g_nPowerLevel = 20;
						}

						// Find an explosion sprite to assign
						for( g_sprite_k = g_SpriteList.begin(); g_sprite_k != g_SpriteList.end(); ++g_sprite_k )
						{
							if( !lstrcmp(g_sprite_k->m_chType, "explosion") && g_sprite_k->m_bActive == false )
							{
								// Assign explosion to the collided mine's position and speed
								g_sprite_k->m_dPosition_x = (g_sprite_j->m_dPosition_x - 10);
								g_sprite_k->m_dPosition_y = (g_sprite_j->m_dPosition_y - 20);
								g_sprite_k->m_dVelocity_x = g_sprite_j->m_dVelocity_x;
								g_sprite_k->m_bActive = true;

								// Place the mine into a waiting off-screen
								g_sprite_j->m_dPosition_x = -100;
								g_sprite_j->m_dPosition_y = -100;
								g_sprite_j->m_dVelocity_x = 0;
								g_sprite_j->m_dVelocity_y = 0;
								break;
							}
						}
					}
				}
			}
		}

		nCount2 = 0;
	}
}

//-----------------------------------------------------------------------------
// Name: SpriteCollisionTest()
// Desc: Simple function wrapper that passes the proper sprite values 
//       to CollisionTest() for checking.
//-----------------------------------------------------------------------------
bool SpriteCollisionTest( CSprite sprite1, CSprite sprite2 )
{
	// This function passes sprite values to the CollisionTest() function
	// in several different ways depending on whether or not the sprites
	// involved are being scaled or have had there size properties adjusted
	// for the purposes of fine-tuning collision detection.

	bool bCollide = false;
	
	if( sprite1.m_bModifyCollision == true && sprite2.m_bModifyCollision == true )
	{
		// Both sprites require modification before sending.
		bCollide = CollisionTest( sprite1.m_dPosition_x  + sprite1.m_nCollisionLeft, 
								  sprite1.m_dPosition_y  - sprite1.m_nCollisionTop, 
								  sprite1.m_nFrameWidth  + sprite1.m_nWidthScaling  + sprite1.m_nCollisionRight, 
								  sprite1.m_nFrameHeight + sprite1.m_nHeightScaling + sprite1.m_nCollisionBottom, 
								  sprite2.m_dPosition_x  + sprite2.m_nCollisionLeft,
								  sprite2.m_dPosition_y  - sprite2.m_nCollisionTop,
								  sprite2.m_nFrameWidth  + sprite2.m_nWidthScaling  + sprite2.m_nCollisionRight, 
								  sprite2.m_nFrameHeight + sprite2.m_nHeightScaling + sprite2.m_nCollisionBottom);
	}
	else if( sprite1.m_bModifyCollision == true && sprite2.m_bModifyCollision == false )
	{
		// Only the first sprite requires modification before sending.
		bCollide = CollisionTest( sprite1.m_dPosition_x  + sprite1.m_nCollisionLeft,
								  sprite1.m_dPosition_y  - sprite1.m_nCollisionTop,
								  sprite1.m_nFrameWidth  + sprite1.m_nWidthScaling  + sprite1.m_nCollisionRight,
								  sprite1.m_nFrameHeight + sprite1.m_nHeightScaling + sprite1.m_nCollisionBottom,
								  sprite2.m_dPosition_x,
								  sprite2.m_dPosition_y,
								  sprite2.m_nFrameWidth,
								  sprite2.m_nFrameHeight);

	}
	else if( sprite1.m_bModifyCollision == false && sprite2.m_bModifyCollision == true )
	{
		// Only the second sprite requires modification before sending.
		bCollide = CollisionTest( sprite1.m_dPosition_x,
								  sprite1.m_dPosition_y,
								  sprite1.m_nFrameWidth,
								  sprite1.m_nFrameHeight,
								  sprite2.m_dPosition_x  + sprite2.m_nCollisionLeft,
								  sprite2.m_dPosition_y  - sprite2.m_nCollisionTop,
								  sprite2.m_nFrameWidth  + sprite2.m_nWidthScaling  + sprite2.m_nCollisionRight,
								  sprite2.m_nFrameHeight + sprite2.m_nHeightScaling + sprite2.m_nCollisionBottom);

	}
	else
	{
		// Neither of the sprites require modification before sending.
		bCollide = CollisionTest( sprite1.m_dPosition_x,
								  sprite1.m_dPosition_y,
								  sprite1.m_nFrameWidth,
								  sprite1.m_nFrameHeight,
								  sprite2.m_dPosition_x,
								  sprite2.m_dPosition_y,
								  sprite2.m_nFrameWidth, 
								  sprite2.m_nFrameHeight);


	}

	return(bCollide);
}

//-----------------------------------------------------------------------------
// Name: CollisionTest()
// Desc: Checks to see if two rectangular regions overlap or not.
//-----------------------------------------------------------------------------
bool CollisionTest(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) 
{
	// This function tests if the two rectangles overlap

	// Get the radi of each rect
	int width1  = (w1>>1) - (w1>>3);
	int height1 = (h1>>1) - (h1>>3);

	int width2  = (w2>>1) - (w2>>3);
	int height2 = (h2>>1) - (h2>>3);

	// Compute center of each rect
	int cx1 = x1 + width1;
	int cy1 = y1 + height1;

	int cx2 = x2 + width2;
	int cy2 = y2 + height2;

	// Compute deltas
	int dx = abs(cx2 - cx1);
	int dy = abs(cy2 - cy1);

	// Test if rects overlap
	if(dx < (width1+width2) && dy < (height1+height2))
		return true;
	else // Else no collision
		return false;
}

//-----------------------------------------------------------------------------
// Name: ComputeScore()
// Desc: Animates the number sprites according to the current score
//-----------------------------------------------------------------------------
void ComputeScore(void)
{
	// Calculate the frame for each number sprite based on current score

	int number4 = g_nScore%10;           // Ones Place
	int number3 = (g_nScore%100)/10;     // Tens Place
	int number2 = (g_nScore%1000)/100;   // Hundreds Place
	int number1 = (g_nScore%10000)/1000; // Thousands Place

	int ships1  = g_nShipsLeft%10;       // Ones Place

	// Go through the linked list looking for the number sprites and assign values
	for( g_sprite_i = g_SpriteList.begin(); g_sprite_i != g_SpriteList.end(); ++g_sprite_i )
	{
		if( !lstrcmp(g_sprite_i->m_chType, "numbers") )
		{
			if( g_sprite_i->m_nState == 0 )
				g_sprite_i->m_nCurrentFrame = number1;

			if( g_sprite_i->m_nState == 1 )
				g_sprite_i->m_nCurrentFrame = number2;

			if( g_sprite_i->m_nState == 2 )
				g_sprite_i->m_nCurrentFrame = number3;

			if( g_sprite_i->m_nState == 3 )
				g_sprite_i->m_nCurrentFrame = number4;
		}

		if( !lstrcmp(g_sprite_i->m_chType, "shipnumber") )
		{
			g_sprite_i->m_nCurrentFrame = ships1;
		}
	}
}

//-----------------------------------------------------------------------------
// Name: DisplayFrame()
// Desc: Blts the sprites to the back buffer, then it blts or flips the 
//       back buffer onto the primary buffer.
//-----------------------------------------------------------------------------
HRESULT DisplayFrame()
{
	HRESULT hr;

    // Fill the back buffer with black, ignoring errors until the flip
    g_pDisplay->Clear(0);

	DrawStars();

	DrawExhaust();

	DrawPowerBar();

	if( g_bWindowed )
	{
		WriteToSurface( "Press Escape to quit.  Press Alt-Enter to switch to Full-Screen mode.", 
			NULL, 5, (SCREEN_HEIGHT - 20), g_pDisplay->GetBackBuffer(), false);
	}
	else
	{
		WriteToSurface( "Press Escape to quit.  Press Alt-Enter to switch to Windowed mode.", 
			NULL, 5, (SCREEN_HEIGHT - 20), g_pDisplay->GetBackBuffer(), false);
	}

	// Loop through all the sprites in the list and draw each one!
	for( g_sprite_i = g_SpriteList.begin(); g_sprite_i != g_SpriteList.end(); ++g_sprite_i )
	{
		if( !lstrcmp(g_sprite_i->m_chBitmap, "misc.bmp") )
			g_sprite_i->drawSprite( g_pDisplay, g_pMiscBitmap );

		if( !lstrcmp(g_sprite_i->m_chBitmap, "fighter.bmp") )
			g_sprite_i->drawSprite( g_pDisplay, g_pFighterBitmap );

		if( !lstrcmp(g_sprite_i->m_chBitmap, "numbers.bmp") )
			g_sprite_i->drawSprite( g_pDisplay, g_pNumbersBitmap );

		if( !lstrcmp(g_sprite_i->m_chBitmap, "explosion.bmp") )
			g_sprite_i->drawSprite( g_pDisplay, g_ExplosionBitmap );
	}

    // Flip or blt the back buffer onto the primary buffer
    if( FAILED( hr = g_pDisplay->Present() ) )
        return hr;

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: DrawStars()
// Desc: Draws the stars as pixel points on the back buffer
//
// WARNING: Do not attempt to debug into this function! The Lock() method of
//          DirectX takes a Win16Mutex when locking a surface for direct pixel
//          manipulation and any attempt to examine memory with a debugger 
//          will cause the computer to crash with an access violation.
//
//-----------------------------------------------------------------------------
HRESULT DrawStars(void)
{
	DDSURFACEDESC2 ddsd;
	int linearPitch = 0;
	int nColorDepth = 0;
	int x = 0;
	int y = 0;
	HRESULT hr;

	// Move and draw the stars 
	MoveStars();

	ZeroMemory( &ddsd, sizeof(ddsd) );
    ddsd.dwSize = sizeof(ddsd);

	// Lock the surface to directly write to the surface memory 
    if( FAILED( hr = g_pDisplay->GetBackBuffer()->Lock( NULL, &ddsd, 
		DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL ) ) )
        return hr;

	 // Get the color depth of the back buffer
	nColorDepth   = ddsd.ddpfPixelFormat.dwRGBBitCount;
	// Get number of bytes for the linear pitch
	linearPitch = (int)ddsd.lPitch; 

	if( nColorDepth == 8 )
	{
		// Use a BYTE because 1 byte per pixel in 8 bit color
		BYTE *surfaceBuffer = static_cast<BYTE*>(ddsd.lpSurface);

		for( int i = 0; i < TOTAL_STARS; i++ )
		{
			x = stars[i].x;
			y = stars[i].y;

			// Plot the pixel
			surfaceBuffer[x+y*linearPitch] = stars[i].color;
		}
	}
	else if( nColorDepth == 16 )
	{
		// Use a USHORT because 2 bytes per pixel in 16 bit color
		USHORT *surfaceBuffer = static_cast<USHORT*>(ddsd.lpSurface);
		// And half the linear pitch because each pixel is now worth 2 bytes
		linearPitch = (linearPitch>>1);   

		for( int i = 0; i < TOTAL_STARS; i++ )
		{
			int x = stars[i].x;
			int y = stars[i].y;

			// Plot the pixel
			surfaceBuffer[x+y*linearPitch] = stars[i].color;
		} 
	}
	else if( nColorDepth == 32 )
	{
		// Use a UINT because 4 bytes per pixel in 32 bit color
		UINT *surfaceBuffer = static_cast<UINT*>(ddsd.lpSurface);
		// And half the linear pitch twice because each pixel is now worth 4 bytes
		linearPitch = (linearPitch>>2); 

		for( int i = 0; i < TOTAL_STARS; i++ )
		{
			int x = stars[i].x;
			int y = stars[i].y;

			// Plot the pixel
			surfaceBuffer[x+y*linearPitch] = stars[i].color;
		} 
	}

	if( FAILED( hr = g_pDisplay->GetBackBuffer()->Unlock( NULL ) ) )
		return hr;

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: DrawExhaust()
// Desc: Draw all visible exhaust particles
//
// WARNING: Do not attempt to debug into this function! The Lock() method of
//          DirectX takes a Win16Mutex when locking a surface for direct pixel
//          manipulation and any attempt to examine memory with a debugger 
//          will cause the computer to crash with an access violation.
//
//-----------------------------------------------------------------------------
HRESULT DrawExhaust(void)
{
	DDSURFACEDESC2 ddsd;
	int linearPitch = 0;
	int nColorDepth = 0;
	int x = 0;
	int y = 0;
	HRESULT hr;

	// Move and draw the particles 
	MoveExhaust();

	ZeroMemory( &ddsd, sizeof(ddsd) );
    ddsd.dwSize = sizeof(ddsd);

	// Lock the surface to directly write to the surface memory 
    if( FAILED( hr = g_pDisplay->GetBackBuffer()->Lock( NULL, &ddsd, 
		DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL ) ) )
        return hr;

	 // Get the color depth of the back buffer
	nColorDepth   = ddsd.ddpfPixelFormat.dwRGBBitCount;
	// Get number of bytes for the linear pitch
	linearPitch = (int)ddsd.lPitch; 

	if( nColorDepth == 8 )
	{
		// Use a BYTE because 1 byte per pixel in 8 bit color
		BYTE *surfaceBuffer = static_cast<BYTE*>(ddsd.lpSurface);

		for( int i = 0; i < TOTAL_PARTICLES; i++ )
		{
			if( exhaust[i].bVisible == true )
			{
				x = exhaust[i].x;
				y = exhaust[i].y;

				// Plot the center pixel
				surfaceBuffer[x+(y*linearPitch)]     = exhaust[i].color;
				// Plot the left pixel
				surfaceBuffer[(x+1)+(y*linearPitch)] = exhaust[i].color;
				// Plot the right pixel
				surfaceBuffer[(x-1)+(y*linearPitch)] = exhaust[i].color;
				// Plot the top pixel
				surfaceBuffer[x+((y-1)*linearPitch)] = exhaust[i].color;
				// Plot the bottom pixel
				surfaceBuffer[x+((y+1)*linearPitch)] = exhaust[i].color;
			}
		}
	}
	else if( nColorDepth == 16 )
	{
		// Use a USHORT because 2 bytes per pixel in 16 bit color
		USHORT *surfaceBuffer = static_cast<USHORT*>(ddsd.lpSurface);
		// And half the linear pitch because each pixel is now worth 2 bytes
		linearPitch = (linearPitch>>1);     

		for( int i = 0; i < TOTAL_PARTICLES; i++ )
		{
			if( exhaust[i].bVisible == true )
			{
				x = exhaust[i].x;
				y = exhaust[i].y;

				// Plot the center pixel
				surfaceBuffer[x+(y*linearPitch)]     = exhaust[i].color;
				// Plot the left pixel
				surfaceBuffer[(x+1)+(y*linearPitch)] = exhaust[i].color;
				// Plot the right pixel
				surfaceBuffer[(x-1)+(y*linearPitch)] = exhaust[i].color;
				// Plot the top pixel
				surfaceBuffer[x+((y-1)*linearPitch)] = exhaust[i].color;
				// Plot the bottom pixel
				surfaceBuffer[x+((y+1)*linearPitch)] = exhaust[i].color;
			}
		}
	}
	else if( nColorDepth == 32 )
	{
		// Use a UINT because 4 bytes per pixel in 32 bit color
		UINT *surfaceBuffer = static_cast<UINT*>(ddsd.lpSurface);
		// And half the linear pitch twice because each pixel is now worth 4 bytes
		linearPitch = (linearPitch>>2); 

		for( int i = 0; i < TOTAL_PARTICLES; i++ )
		{
			if( exhaust[i].bVisible == true )
			{
				x = exhaust[i].x;
				y = exhaust[i].y;

				// Plot the center pixel
				surfaceBuffer[x+(y*linearPitch)]     = exhaust[i].color;
				// Plot the left pixel
				surfaceBuffer[(x+1)+(y*linearPitch)] = exhaust[i].color;
				// Plot the right pixel
				surfaceBuffer[(x-1)+(y*linearPitch)] = exhaust[i].color;
				// Plot the top pixel
				surfaceBuffer[x+((y-1)*linearPitch)] = exhaust[i].color;
				// Plot the bottom pixel
				surfaceBuffer[x+((y+1)*linearPitch)] = exhaust[i].color;
			}
		}
	}

	g_pDisplay->GetBackBuffer()->Unlock( NULL );

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: DrawPowerBar()
// Desc: Draws the power bar and the colored hash mark sprites that make up  
//       the power bar indicator.
//-----------------------------------------------------------------------------
HRESULT DrawPowerBar(void)
{
	// Warning: This function assumes that the sprites that make up the power 
	// bar have been placed in the linked list in a certain order. If this 
	// order is changed, the power bar will be drawn incorrectly or not at all.

	int nStartingPosit_x = 450;
	int nStartingPosit_y = 10;
	int nMarkSpacing = 5;
	int nMarkCount   = 0;
	int nTotalRed    = 5;
	int nTotalYellow = 5;
	int nTotalGreen  = 10;
	int i = 0;

	for( g_sprite_i = g_SpriteList.begin(); g_sprite_i != g_SpriteList.end(); ++g_sprite_i )
	{
		if( !lstrcmp(g_sprite_i->m_chType, "powerbar") )
		{
			g_sprite_i->m_bActive = true;

			g_sprite_i->m_dPosition_x = nStartingPosit_x - 3;
			g_sprite_i->m_dPosition_y = nStartingPosit_y - 2;
			g_sprite_i->drawSprite( g_pDisplay, g_pMiscBitmap );

			g_sprite_i->m_bActive = false;
		}

		if( !lstrcmp(g_sprite_i->m_chType, "redmark") )
		{
			// Keep drawing red hash marks until we hit the limit or
			// they're no longer required.
			g_sprite_i->m_bActive = true;

			if( g_nPowerLevel > 0 )
			{
				for( i = 0; i < nTotalRed; ++i )
				{
					g_sprite_i->m_dPosition_x = nStartingPosit_x;
					g_sprite_i->m_dPosition_y = nStartingPosit_y;
					g_sprite_i->drawSprite( g_pDisplay, g_pMiscBitmap );

					nStartingPosit_x += nMarkSpacing;
					++nMarkCount;

					if( nMarkCount >= g_nPowerLevel )
						break;
				}
			}

			g_sprite_i->m_bActive = false;
		}

		if( !lstrcmp(g_sprite_i->m_chType, "yellowmark") )
		{
			// Keep drawing yellow hash marks until we hit the limit or
			// they're no longer required.
			g_sprite_i->m_bActive = true;

			if( g_nPowerLevel > nTotalYellow )
			{
				for( i = 0; i < nTotalYellow; ++i )
				{
					g_sprite_i->m_dPosition_x = nStartingPosit_x;
					g_sprite_i->m_dPosition_y = nStartingPosit_y;
					g_sprite_i->drawSprite( g_pDisplay, g_pMiscBitmap );

					nStartingPosit_x += nMarkSpacing;
					++nMarkCount;

					if( nMarkCount >= g_nPowerLevel )
						break;
				}
			}

			g_sprite_i->m_bActive = false;
		}

		if( !lstrcmp(g_sprite_i->m_chType, "greenmark") )
		{
			// Keep drawing green hash marks until we hit the limit or
			// they're no longer required.
			g_sprite_i->m_bActive = true;

			if( g_nPowerLevel > nTotalGreen )
			{

				for( i = 0; i < nTotalGreen; ++i )
				{
					g_sprite_i->m_dPosition_x = nStartingPosit_x;
					g_sprite_i->m_dPosition_y = nStartingPosit_y;
					g_sprite_i->drawSprite( g_pDisplay, g_pMiscBitmap );

					nStartingPosit_x += nMarkSpacing;
					++nMarkCount;

					if( nMarkCount >= g_nPowerLevel )
						break;
				}
			}
				
			g_sprite_i->m_bActive = false;
			break; // There's no reason to keep searching the linked list - Bail out!
		}
	}

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: WriteToSurface()
// Desc: Writes text and/or variable values to the DirectDraw surface passed in
//-----------------------------------------------------------------------------
void WriteToSurface( char *label, int value, int x, int y, 
					 LPDIRECTDRAWSURFACE7 lpdds, bool bValuePassed )
{
	// This function assists in debugging by writing an 
	// informative label and a variable value at the 
	// specified X/Y position on the DirectDraw surface 
	// passed in. If you wish to display text without
	// a value, pass in "false" for the last argument and
	// set value to anything.

	HDC  wdc;
	char valueBuffer[10] = { NULL };
	char textBuffer[200] = { NULL };

	_itoa( value, valueBuffer, 10 );

	strcat( textBuffer, label );

	if( bValuePassed )
		strcat( textBuffer, valueBuffer );

	if( FAILED( lpdds->GetDC(&wdc) ) )
	   return;

	SetTextColor( wdc, RGB(255,255,0) );
	SetBkMode( wdc, TRANSPARENT );
	TextOut( wdc, x, y, textBuffer,strlen(textBuffer) );

	lpdds->ReleaseDC(wdc);
}

//-----------------------------------------------------------------------------
// Name: RandomInt()
// Desc: Produces a random int value between some given range
//-----------------------------------------------------------------------------
int RandomInt( int nLow, int nHigh )
{
    int nRange = nHigh - nLow;
    int nNum = rand() % nRange;
    return( nNum + nLow );
}
