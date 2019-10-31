//-----------------------------------------------------------------------------
//           Name: dx8_mode_switch.cpp
//         Author: Kevin Harris
//  Last Modified: 03/03/04
//    Description: An empty rendering loop capable of switching between windowed 
//                 and full-screen exclusive mode for debugging purposes.
//-----------------------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#define STRICT

#include <windows.h>
#include <mmsystem.h>
#include <ddraw.h>
#include "ddutil.h"
#include "resource.h"

//-----------------------------------------------------------------------------
// DEFINES / MACROS
//-----------------------------------------------------------------------------
#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

#define WINDOWED_HELPTEXT   TEXT("Press Escape to quit.  Press Alt-Enter to switch to Full-Screen mode.")
#define FULLSCREEN_HELPTEXT TEXT("Press Escape to quit.  Press Alt-Enter to switch to Windowed mode.")

//-----------------------------------------------------------------------------
// SYMBOLIC CONSTANTS
//-----------------------------------------------------------------------------
const int SCREEN_WIDTH  = 640;
const int SCREEN_HEIGHT = 480;
const int COLOR_DEPTH   = 16;

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
CDisplay *g_pDisplay     = NULL;
CSurface *g_pTextSurface = NULL;
CSurface *g_pBitmap      = NULL;
BOOL      g_bWindowed    = TRUE;
BOOL      g_bActive      = FALSE;
DWORD     g_dwLastTick   = 0;
RECT      g_rcWindow;

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
HRESULT init( HWND hWnd );
HRESULT render( HWND hWnd );
void    shutDown();

HRESULT initDirectDrawMode( HWND hWnd, BOOL bWindowed );
HRESULT restoreSurfaces();

//-----------------------------------------------------------------------------
// Name: WindowProc()
// Desc: The main window procedure
//-----------------------------------------------------------------------------
LRESULT CALLBACK WindowProc( HWND   hWnd, 
							 UINT   msg, 
							 WPARAM wParam, 
							 LPARAM lParam )
{
     switch (msg)
    {
        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
                case IDM_TOGGLEFULLSCREEN:
                    // Toggle the fullscreen/window mode
                    if( g_bWindowed )
                        GetWindowRect( hWnd, &g_rcWindow );

                    g_bWindowed = !g_bWindowed;

                    if( FAILED( initDirectDrawMode( hWnd, g_bWindowed ) ) )
                    {
                        SAFE_DELETE( g_pDisplay );

                        MessageBox( hWnd, TEXT("initDirectDrawMode() failed. ")
                                    TEXT("The sample will now exit. "), TEXT("Mode Switch"), 
                                    MB_ICONERROR | MB_OK );
                	    PostMessage( hWnd, WM_CLOSE, 0, 0 );
                    }

                    return 0L;

                case IDM_EXIT:
                    // Received key/menu command to exit app
            	    PostMessage( hWnd, WM_CLOSE, 0, 0 );
                    return 0L;
            }
            break; // Continue with default processing

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
            // Hide the cursor if in fullscreen 
            if( !g_bWindowed )
            {
                SetCursor( NULL );
                return TRUE;
            }
            break; // Continue with default processing

        case WM_QUERYNEWPALETTE:
            if( g_pDisplay && g_pDisplay->GetFrontBuffer() )            
            {
                // If we are in windowed mode with a desktop resolution in 8 bit 
                // color, then the palette we created during init has changed 
                // since then.  So get the palette back from the primary 
                // DirectDraw surface, and set it again so that DirectDraw 
                // realises the palette, then release it again. 
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
            // Prevent moving/sizing and power loss in fullscreen mode
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
            
        case WM_DESTROY:
            // Cleanup and close the app
            shutDown();
            PostQuitMessage( 0 );
            return 0L;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point to the program. Initializes everything and calls
//       UpdateFrame() when idle from the message pump.
//-----------------------------------------------------------------------------
int WINAPI WinMain(	HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPSTR     lpCmdLine,
					int       nCmdShow )
{
	WNDCLASSEX winClass; 
	HWND       hWnd;
	MSG        msg;
    HACCEL     hAccel;

	winClass.lpszClassName	= "MY_WINDOWS_CLASS";
	winClass.cbSize         = sizeof(WNDCLASSEX);
	winClass.style			= CS_DBLCLKS | CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	winClass.lpfnWndProc	= WindowProc;
	winClass.hInstance		= hInstance;
	winClass.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
	winClass.hIconSm        = LoadIcon(NULL, IDI_APPLICATION);
	winClass.hCursor		= LoadCursor(NULL, IDC_ARROW);
	winClass.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
	winClass.lpszMenuName	= MAKEINTRESOURCE(IDR_MENU);
	winClass.cbClsExtra		= 0;
	winClass.cbWndExtra		= 0;

    if( RegisterClassEx( &winClass ) == 0 )
        return E_FAIL;

    // Load keyboard accelerators
    hAccel = LoadAccelerators( hInstance, MAKEINTRESOURCE(IDR_MAIN_ACCEL) );

    // Calculate the proper size for the window given a client of 640x480
    DWORD dwFrameWidth    = GetSystemMetrics( SM_CXSIZEFRAME );
    DWORD dwFrameHeight   = GetSystemMetrics( SM_CYSIZEFRAME );
    DWORD dwMenuHeight    = GetSystemMetrics( SM_CYMENU );
    DWORD dwCaptionHeight = GetSystemMetrics( SM_CYCAPTION );
    DWORD dwWindowWidth   = SCREEN_WIDTH  + dwFrameWidth * 2;
    DWORD dwWindowHeight  = SCREEN_HEIGHT + dwFrameHeight * 2 + 
                            dwMenuHeight + dwCaptionHeight;

    // Create and show the main window
	hWnd = CreateWindowEx( NULL, "MY_WINDOWS_CLASS", 
                           "Direct3D (DX8) - Mode Switch",
						   WS_OVERLAPPEDWINDOW | WS_VISIBLE,
					       0, 0, 640, 480, NULL, NULL, hInstance, NULL );
    if( hWnd == NULL )
    	return E_FAIL;

	ShowWindow( hWnd, nCmdShow );
    UpdateWindow( hWnd );

    // Save the window size/pos for switching modes
    GetWindowRect( hWnd, &g_rcWindow );

	init( hWnd );

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
            if( TranslateAccelerator( hWnd, hAccel, &msg ) == 0 )
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

                if( FAILED( render( hWnd ) ) )
                {
                    SAFE_DELETE( g_pDisplay );

                    MessageBox( hWnd, TEXT("GameMain() failed. ")
                                TEXT("The sample will now exit. "), TEXT("Mode Switch"), 
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
	
	// Clean up and release Direct3D resources.
	shutDown();

    UnregisterClass( "MY_WINDOWS_CLASS", winClass.hInstance );

	return msg.wParam;
}

//-----------------------------------------------------------------------------
// Name: init()
// Desc: Initilialize all DirectX objects to be used
//-----------------------------------------------------------------------------
HRESULT init( HWND hWnd )
{
	HRESULT hr;

	// Initialize all the surfaces we need
    if( FAILED( hr = initDirectDrawMode( hWnd, g_bWindowed ) ) )
        return hr;

    g_dwLastTick = timeGetTime();

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: Move the sprites, blt them to the back buffer, then flip or blt the 
//       back buffer to the primary buffer
//-----------------------------------------------------------------------------
HRESULT render( HWND hWnd )
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
                return initDirectDrawMode( hWnd, g_bWindowed );
        }
        return hr;
    }

    // Display the sprites on the screen

    // Fill the back buffer with black, ignoring errors until the flip
    g_pDisplay->Clear(0);

	g_pDisplay->Blt( 0, 0, g_pBitmap, NULL );

    // Blt the help text on the backbuffer, ignoring errors until the flip
    g_pDisplay->Blt( 10, 10, g_pTextSurface, NULL );

    // Flip or blt the back buffer onto the primary buffer
    if( FAILED( hr = g_pDisplay->Present() ) )
	{
		if( hr != DDERR_SURFACELOST )
            return hr;

        // The surfaces were lost, so restore them 
        restoreSurfaces();
        return hr;
	}

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: Release all the DirectDraw objects
//-----------------------------------------------------------------------------
VOID shutDown()
{
	SAFE_DELETE( g_pBitmap );
    SAFE_DELETE( g_pTextSurface );
    SAFE_DELETE( g_pDisplay );
}


//-----------------------------------------------------------------------------
// Name: initDirectDrawMode()
// Desc: Called when the user wants to toggle between full-screen and windowed 
//       to create all the needed DDraw surfaces and set the coop level
//-----------------------------------------------------------------------------
HRESULT initDirectDrawMode( HWND hWnd, BOOL bWindowed )
{
    // Release all existing surfaces
	SAFE_DELETE( g_pBitmap );
    SAFE_DELETE( g_pTextSurface );
    SAFE_DELETE( g_pDisplay );

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
        g_pDisplay->CreateFullScreenDisplay( hWnd, SCREEN_WIDTH, 
			                                 SCREEN_HEIGHT, COLOR_DEPTH );

        // Disable the menu in full-screen since we are 
        // using a palette and a menu would look bad 
        SetMenu( hWnd, NULL );

        // Remove the system menu from the window's style
        DWORD dwStyle = GetWindowLong( hWnd, GWL_STYLE );
        dwStyle &= ~WS_SYSMENU;
        SetWindowLong( hWnd, GWL_STYLE, dwStyle );       
    }

    if( g_bWindowed )
    {
        // Create a surface, and draw text to it.  
        g_pDisplay->CreateSurfaceFromText( &g_pTextSurface, 
			                               NULL, WINDOWED_HELPTEXT, 
                                           RGB(0,0,0), RGB(255, 255, 0) );
    }
    else
    {
        // Create a surface, and draw text to it.  
        g_pDisplay->CreateSurfaceFromText( &g_pTextSurface, NULL, 
			                               FULLSCREEN_HELPTEXT, 
                                           RGB(0,0,0), RGB(255, 255, 0) );
    }

	g_pDisplay->CreateSurfaceFromBitmap( &g_pBitmap, "ghost.bmp", 640, 480 );

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: restoreSurfaces()
// Desc: Restore any lost surfaces, and redraw the sprite surfaces.
//-----------------------------------------------------------------------------
HRESULT restoreSurfaces()
{
	HRESULT hr;
 
    if( FAILED( hr = g_pDisplay->GetDirectDraw()->RestoreAllSurfaces() ) )
        return hr;

    // No need to re-create the surface, just re-draw it.
    if( FAILED( hr = g_pBitmap->DrawBitmap( "ghost.bmp",640, 480 ) ) )
        return hr;

    if( g_bWindowed )
    {
        // No need to re-create the surface, just re-draw it.
        if( FAILED( hr = g_pTextSurface->DrawText( NULL, WINDOWED_HELPTEXT, 
                                       0, 0, RGB(0,0,0), RGB(255, 255, 0) ) ) )
            return hr;
    }
    else
    {
        // No need to re-create the surface, just re-draw it.
        if( FAILED( hr = g_pTextSurface->DrawText( NULL, FULLSCREEN_HELPTEXT, 
                                       0, 0, RGB(0,0,0), RGB(255, 255, 0) ) ) )
            return hr;
    }

    return S_OK;
}






