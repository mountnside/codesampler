//-----------------------------------------------------------------------------
//           Name: dx8_sprite_test.h
//         Author: Kevin Harris
//  Last Modified: 03/03/04
//    Description: Test program for my sprite class
//
//           Note: Some of the sprites move or activate by playing with the
//                 arrow-keys.
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
// MACROS
//-----------------------------------------------------------------------------
#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }
#define KEYDOWN(name,key) (name[key] & 0x80)

//-----------------------------------------------------------------------------
// SYMBOLIC CONSTANTS
//-----------------------------------------------------------------------------
const int SCREEN_WIDTH  = 800;
const int SCREEN_HEIGHT = 600;
const int SCREEN_BPP    = 16;

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
CDisplay *g_pDisplay   = NULL;
CSurface *g_pDonut     = NULL;  // Off-screen surface for the donut bitmap.
CSurface *g_pNumbers   = NULL;  // Off-screen surface for the numbers bitmap.
CSurface *g_pFly       = NULL;  // Off-screen surface for the fly bitmap.
CSurface *g_pShips     = NULL;  // Off-screen surface for the ships bitmap.
CSurface *g_pExplosion = NULL;  // Off-screen surface for the explosion bitmap.

LPDIRECTINPUT8       g_lpdi;      // DirectInput object
LPDIRECTINPUTDEVICE8 g_pKeyboard; // DirectInput device

float  g_fElpasedTime;
double g_dCurTime;
double g_dLastTime;
float  g_fAnimationTimer = 0.0f;

RECT  g_rcWindow;
RECT  g_rcViewport;
RECT  g_rcScreen;
BOOL  g_bWindowed   = true;
BOOL  g_bActive     = false;
DWORD g_dwLastTick  = 0;

// Create a linked list with STL to hold and manage CSprite objects
typedef list<CSprite> SPRITELIST;

SPRITELIST g_SpriteList;
SPRITELIST::iterator g_sprite_i;
SPRITELIST::iterator g_sprite_j;
SPRITELIST::iterator g_sprite_k;

//-----------------------------------------------------------------------------
// Function-prototypes
//-----------------------------------------------------------------------------
LRESULT CALLBACK MainWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
HRESULT WinInit( HINSTANCE hInst, int nCmdShow, HWND* phWnd, HACCEL* phAccel );
HRESULT GameStart( HWND hWnd );
HRESULT GameMain( HWND hWnd );
void    GameOver(void);
HRESULT InitDirectDraw( HWND hWnd, BOOL bWindowed );
void    FreeDirectDraw(void);
HRESULT RestoreSurfaces(void);
HRESULT AttachClipper( LPRECT lpRect );
HRESULT InitDirectInput( HWND hWnd );
void    FreeDirectInput(void);
void    InitializeSprites(void);
void    MoveSprites(void);
HRESULT DisplayFrame(void);
void    WriteToSurface( char *label, int value, int x, int y, 
					    LPDIRECTDRAWSURFACE7 lpdds, bool bValuePassed );

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

                if( FAILED( GameMain( hWnd ) ) )
                {
                    SAFE_DELETE( g_pDisplay );

                    MessageBox( hWnd, TEXT("GameMain() failed. ")
                                TEXT("The game will now exit. "), TEXT("Sprite Test"), 
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
HRESULT WinInit( HINSTANCE hInst, 
				 int       nCmdShow, 
				 HWND     *phWnd, 
				 HACCEL   *phAccel )
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
                           TEXT("Direct3D (DX8) - 2D Sprite Test",),
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
LRESULT CALLBACK MainWndProc( HWND   hWnd,
							  UINT   msg, 
							  WPARAM wParam, 
							  LPARAM lParam )
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
                                    TEXT("The game will now exit. "), TEXT("Sprite Test"), 
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
            }

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

    g_dwLastTick = timeGetTime();

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

    // Figure how much time has passed since the last time
    DWORD dwCurrTick = timeGetTime();
    DWORD dwTickDiff = dwCurrTick - g_dwLastTick;

    // Don't update if no time has passed 
    if( dwTickDiff == 0 )
        return S_OK;

    g_dwLastTick = dwCurrTick;

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

	g_dCurTime     = timeGetTime();
	g_fElpasedTime = (float)((g_dCurTime - g_dLastTime) * 0.001);
	g_dLastTime    = g_dCurTime;

	g_fAnimationTimer += g_fElpasedTime;

	if( g_fAnimationTimer >= 0.016f )
		g_fAnimationTimer = 0.0f; // Target of 1/60th of a second (60 FPS) reached. Render a new frame.
	else
		return S_OK; // It's too early. Return now and render nothing.

	// Move the game sprites
	MoveSprites();

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

	if( FAILED( hr = g_pDisplay->CreateSurfaceFromBitmap( &g_pDonut, "donut.bmp", 0, 0 ) ) )
    return hr;

	if( FAILED( hr = g_pDisplay->CreateSurfaceFromBitmap( &g_pNumbers, "numbers.bmp", 0, 0 ) ) )
    return hr;

	if( FAILED( hr = g_pDisplay->CreateSurfaceFromBitmap( &g_pShips, "ships.bmp", 0, 0 ) ) )
    return hr;

	if( FAILED( hr = g_pDisplay->CreateSurfaceFromBitmap( &g_pFly, "fly.bmp", 0, 0 ) ) )
    return hr;

	if( FAILED( hr = g_pDisplay->CreateSurfaceFromBitmap( &g_pExplosion, "explosion.bmp", 0, 0 ) ) )
    return hr;
	

    // Set the color key for the ship sprite to black
    if( FAILED( g_pDonut->SetColorKey( 0 ) ) )
        return E_FAIL;

    // Set the color key for the number sprite to black
    if( FAILED( g_pNumbers->SetColorKey( 0 ) ) )
        return E_FAIL;

    // Set the color key for the explode sprite to black
    if( FAILED( g_pFly->SetColorKey( 0 ) ) )
        return E_FAIL;

    // Set the color key for the shot sprite to black
    if( FAILED( g_pShips->SetColorKey( 0 ) ) )
        return E_FAIL;

	// Set the color key for the shot sprite to black
    if( FAILED( g_pExplosion->SetColorKey( 0 ) ) )
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
	SAFE_DELETE( g_pDonut );
	SAFE_DELETE( g_pNumbers );
	SAFE_DELETE( g_pFly );
	SAFE_DELETE( g_pShips );
	SAFE_DELETE( g_pExplosion );
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
                          TEXT("access is not allowed."), TEXT("Sprite Test"), MB_OK );
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

	sprite.zeroSpriteValues();
	strcpy( sprite.m_chType, "donut" );
	strcpy( sprite.m_chName, "Donut1" );
	strcpy( sprite.m_chBitmap, "donut.bmp" );
	sprite.m_dPosition_x        = 250;
	sprite.m_dPosition_y        = 250;
	sprite.m_dVelocity_x        = 5;
	sprite.m_dVelocity_y        = 5;
	sprite.m_nFrameWidth        = 64;
	sprite.m_nFrameHeight       = 64;
	sprite.m_nFramesAcross      = 5;
	sprite.loadAnimation( 0, 0, 29, CSprite::GOTO_NEXT_ANIMATION, 1 );
	sprite.loadAnimation( 1, 29, 0, CSprite::GOTO_NEXT_ANIMATION, 0 );

	g_SpriteList.push_back(sprite);

	sprite.zeroSpriteValues();
	strcpy( sprite.m_chType, "donut" );
	strcpy( sprite.m_chName, "Donut2" );
	strcpy( sprite.m_chBitmap, "donut.bmp" );
	sprite.m_dPosition_x        = 350;
	sprite.m_dPosition_y        = 250;
	sprite.m_dVelocity_x        = -1;
	sprite.m_dVelocity_y        = -1;
	sprite.m_nFrameWidth        = 64;
	sprite.m_nFrameHeight       = 64;
	sprite.m_nFramesAcross      = 5;
	sprite.loadAnimation( 0, 0, 29, CSprite::LOOP_ANIMATION );

	g_SpriteList.push_back(sprite);

	sprite.zeroSpriteValues();
	strcpy( sprite.m_chType, "donut" );
	strcpy( sprite.m_chName, "Donut3" );
	strcpy( sprite.m_chBitmap, "donut.bmp" );
	sprite.m_dPosition_x        = 75;
	sprite.m_dPosition_y        = 250;
	sprite.m_nFrameWidth        = 64;
	sprite.m_nFrameHeight       = 64;
	sprite.m_nFramesAcross      = 5;
	sprite.loadAnimation( 0, 0, 29, CSprite::MAINTAIN_LAST_FRAME );
	sprite.loadAnimation( 1, 29, 0, CSprite::MAINTAIN_LAST_FRAME );

	g_SpriteList.push_back(sprite);

	sprite.zeroSpriteValues();
	strcpy( sprite.m_chType, "donut" );
	strcpy( sprite.m_chName, "BigDonut" );
	strcpy( sprite.m_chBitmap, "donut.bmp" );
	sprite.m_dPosition_x        = 330;
	sprite.m_dPosition_y        = 0;
	sprite.m_nFrameWidth        = 64;
	sprite.m_nFrameHeight       = 64;
	sprite.m_nFramesAcross      = 5;
	sprite.m_nWidthScaling      = 400;
	sprite.m_nHeightScaling     = 400;
	sprite.m_bAutoAnimate       = false;
	sprite.m_nFrameRateModifier = -10;
	sprite.loadAnimation( 0, 0, 29, CSprite::LOOP_ANIMATION );

	g_SpriteList.push_back(sprite);

	sprite.zeroSpriteValues();
	strcpy( sprite.m_chType, "fly" );
	strcpy( sprite.m_chName, "Fly1" );
	strcpy( sprite.m_chBitmap, "fly.bmp" );
	sprite.m_dPosition_x        = 0;
	sprite.m_dPosition_y        = 0;
	sprite.m_bCollide           = false;
	sprite.m_nFrameWidth        = 320;
	sprite.m_nFrameHeight       = 200;
	sprite.m_nFramesAcross      = 2;
	sprite.loadAnimation( 0, 0, 2, CSprite::LOOP_ANIMATION );

	g_SpriteList.push_back(sprite);

	sprite.zeroSpriteValues();
	strcpy( sprite.m_chType, "ship" );
	strcpy( sprite.m_chName, "YellowShip" );
	strcpy( sprite.m_chBitmap, "ships.bmp" );
	sprite.m_dPosition_x        = 100;
	sprite.m_dPosition_y        = 500;
	sprite.m_bAutoAnimate       = false;
	sprite.m_nFrameWidth        = 32;
	sprite.m_nFrameHeight       = 32;
	sprite.m_nFramesAcross      = 10;
	sprite.loadAnimation( 0, 0, 39, CSprite::LOOP_ANIMATION );

	g_SpriteList.push_back(sprite);

	sprite.zeroSpriteValues();
	strcpy( sprite.m_chType, "ship" );
	strcpy( sprite.m_chName, "RedShip" );
	strcpy( sprite.m_chBitmap, "ships.bmp" );
	sprite.m_dPosition_x        = 200;
	sprite.m_dPosition_y        = 500;
	sprite.m_nFrameWidth        = 32;
	sprite.m_nFrameHeight       = 32;
	sprite.m_nFramesAcross      = 10;
	sprite.loadAnimation( 0, 40, 79, CSprite::LOOP_ANIMATION );

	g_SpriteList.push_back(sprite);

	sprite.zeroSpriteValues();
	strcpy( sprite.m_chType, "ship" );
	strcpy( sprite.m_chName, "GreenShip" );
	strcpy( sprite.m_chBitmap, "ships.bmp" );
	sprite.m_dPosition_x        = 300;
	sprite.m_dPosition_y        = 500;
	sprite.m_nFrameWidth        = 32;
	sprite.m_nFrameHeight       = 32;
	sprite.m_nFramesAcross      = 10;
	sprite.loadAnimation( 0, 80, 119, CSprite::GOTO_NEXT_ANIMATION, 1 );
	sprite.loadAnimation( 1, 119, 80, CSprite::GOTO_NEXT_ANIMATION, 0 );

	g_SpriteList.push_back(sprite);

	sprite.zeroSpriteValues();
	strcpy( sprite.m_chType, "ship" );
	strcpy( sprite.m_chName, "BlueShip" );
	strcpy( sprite.m_chBitmap, "ships.bmp" );
	sprite.m_dPosition_x        = 400;
	sprite.m_dPosition_y        = 500;
	sprite.m_nFrameWidth        = 32;
	sprite.m_nFrameHeight       = 32;
	sprite.m_nFramesAcross      = 10;
	sprite.loadAnimation( 0, 159, 120, CSprite::GOTO_NEXT_ANIMATION, 1 );
	sprite.loadAnimation( 1, 120, 159, CSprite::GOTO_NEXT_ANIMATION, 0 );
	
	g_SpriteList.push_back(sprite);

	sprite.zeroSpriteValues();
	strcpy( sprite.m_chType, "numbers" );
	strcpy( sprite.m_chName, "Numbers1" );
	strcpy( sprite.m_chBitmap, "numbers.bmp" );
	sprite.m_dPosition_x        = 200;
	sprite.m_dPosition_y        = 300;
	sprite.m_bAutoAnimate       = false;
	sprite.m_nFrameWidth        = 17;
	sprite.m_nFrameHeight       = 17;
	sprite.m_nFramesAcross      = 10;
	sprite.m_nFrameOffset_x     = 20;
	sprite.m_nFrameOffset_y     = 50;
	sprite.loadAnimationString( 0, "0 1 2 3 4 5 6 7 8 9", CSprite::LOOP_ANIMATION );

	g_SpriteList.push_back(sprite);

	sprite.zeroSpriteValues();
	strcpy( sprite.m_chType, "explosion" );
	strcpy( sprite.m_chBitmap, "explosion.bmp" );
	sprite.m_dPosition_x        = 250;
	sprite.m_dPosition_y        = 250;
	sprite.m_nFramesAcross      = 7;
	sprite.m_nFrameWidth        = 42;
	sprite.m_nFrameHeight       = 36;
	sprite.m_nFrameRateModifier = -3;
	sprite.loadAnimation( 0, 0, 13, CSprite::GO_INACTIVE );

	g_SpriteList.push_back(sprite);
}

//-----------------------------------------------------------------------------
// Name: MoveSprites()
// Desc: Identifies sprites in the linked list and moves each one accordingly
//-----------------------------------------------------------------------------
void MoveSprites()
{
	char buffer[256];

	// Get keyboard input 
	g_pKeyboard->GetDeviceState(sizeof(buffer),(LPVOID)&buffer); 

	for( g_sprite_i = g_SpriteList.begin(); g_sprite_i != g_SpriteList.end(); ++g_sprite_i )
	{
		// Move donut sprites according to their velocities
		if( !lstrcmp(g_sprite_i->m_chType, "donut" ) )
		{
			g_sprite_i->m_dPosition_x += g_sprite_i->m_dVelocity_x;
			g_sprite_i->m_dPosition_y += g_sprite_i->m_dVelocity_y;

			// Keep them on screen by wrapping their movement
			// to the other side...
			if( g_sprite_i->m_dPosition_x > 800 )
				g_sprite_i->m_dPosition_x = 0;

			if( g_sprite_i->m_dPosition_y > 600 )
				g_sprite_i->m_dPosition_y = 0;

			if( g_sprite_i->m_dPosition_x < 0 )
				g_sprite_i->m_dPosition_x = 800;

			if( g_sprite_i->m_dPosition_y < 0 )
				g_sprite_i->m_dPosition_y = 600;
		}

		// Update the third donut sprite according to keyboard input!
		if( !lstrcmp(g_sprite_i->m_chName, "Donut3" ) )
		{
			if( KEYDOWN(buffer, DIK_UP) )
			{
				g_sprite_i->m_nCurrentAnimation = 0;
				g_sprite_i->m_nCurrentFrame = 0;
			}

			if( KEYDOWN(buffer, DIK_DOWN) )
			{
				g_sprite_i->m_nCurrentAnimation = 1;
				g_sprite_i->m_nCurrentFrame = 0;
			}
		}

		// Update the third donut sprite according to keyboard input!
		if( !lstrcmp(g_sprite_i->m_chType, "explosion" ) )
		{
			if( KEYDOWN(buffer, DIK_UP) )
			{
				g_sprite_i->m_bActive = true;
			}
		}

		// Update the fourth donut sprite according to keyboard input!
		if( !lstrcmp(g_sprite_i->m_chName, "BigDonut" ) )
		{
			if( KEYDOWN(buffer, DIK_UP) )
			{
				//g_sprite_i->m_incFrame(false);
				g_sprite_i->incFrame(true);
			}

			if( KEYDOWN(buffer, DIK_DOWN) )
			{
				//g_sprite_i->m_decFrame(false);
				g_sprite_i->decFrame(true);
			}

			/*
			g_sprite_i->m_nWidthScaling   -= 5;
			g_sprite_i->m_nHeightScaling  -= 5;

			if( g_sprite_i->m_nWidthScaling < -64 )
			{
				g_sprite_i->m_nWidthScaling = 400;
				g_sprite_i->m_nHeightScaling = 400;
			}
			//*/
		}

		// Update the YellowShip sprite according to keyboard input!
		if( !lstrcmp(g_sprite_i->m_chName, "YellowShip" ) )
		{
			if( KEYDOWN(buffer, DIK_LEFT) )
			{
				g_sprite_i->decFrame();
			}

			if( KEYDOWN(buffer, DIK_RIGHT) )
			{
				g_sprite_i->incFrame();
			}
		}

		// Update the Numbers1 sprite according to keyboard input!
		if( !lstrcmp(g_sprite_i->m_chName, "Numbers1" ) )
		{
			if( KEYDOWN(buffer, DIK_LEFT) )
			{
				g_sprite_i->decFrame();
			}

			if( KEYDOWN(buffer, DIK_RIGHT) )
			{
				g_sprite_i->incFrame();
			}
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
		if( !lstrcmp(g_sprite_i->m_chBitmap, "donut.bmp" ) )
			g_sprite_i->drawSprite( g_pDisplay ,g_pDonut );

		if( !lstrcmp(g_sprite_i->m_chBitmap, "fly.bmp" ) )
			g_sprite_i->drawSprite( g_pDisplay ,g_pFly );

		if( !lstrcmp(g_sprite_i->m_chBitmap, "ships.bmp" ) )
			g_sprite_i->drawSprite( g_pDisplay ,g_pShips );

		if( !lstrcmp(g_sprite_i->m_chBitmap, "numbers.bmp" ) )
			g_sprite_i->drawSprite( g_pDisplay ,g_pNumbers );

		if( !lstrcmp(g_sprite_i->m_chBitmap, "explosion.bmp" ) )
			g_sprite_i->drawSprite( g_pDisplay ,g_pExplosion );
	}

    // Flip or blt the back buffer onto the primary buffer
    if( FAILED( hr = g_pDisplay->Present() ) )
        return hr;

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
