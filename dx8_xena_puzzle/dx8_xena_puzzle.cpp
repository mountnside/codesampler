//-----------------------------------------------------------------------------
//           Name: dx8_xena_puzzle.cpp
//         Author: Kevin Harris
//  Last Modified: 03/03/04
//    Description: Xena: The Warrior Princess Puzzle game for my wife.
//
//  Special Notes: You'll need to move or copy the "Images" folder into the
//                 "Debug" folder after compiling if you want to run the debug 
//                 executable.
//-----------------------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <windowsx.h>
#include <ddraw.h>
#include "ddutil.h"
#include "resource.h"

//-----------------------------------------------------------------------------
// SYMBOLIC CONSTANTS
//-----------------------------------------------------------------------------
const int FRAME_DELAY = 100;// Frame delay or animation speed

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
struct GAME_PIECE
{
    RECT rcFrameRect; // Source RECT for blitting
    int  nPosition;   // Postion on the game board
    bool bBlank;      // One sprite must be blank
};

struct GAME_BOARD
{
    RECT rcPositRect; // RECT for hit detection with mouse clicks
    int  nPosition;   // Postion on the game board
};

LPDIRECTDRAW        g_lpDD;                // DirectDraw object
LPDIRECTDRAWSURFACE g_lpDDSPrimary;        // DirectDraw primary
LPDIRECTDRAWSURFACE g_lpDDSBack;           // Fake back buffer surface
LPDIRECTDRAWSURFACE g_lpDDSXena;           // DirectDraw surface for current xena image
LPDIRECTDRAWSURFACE g_lpDDSCursor;         // DirectDraw surface for custom cursor image
LPDIRECTDRAWPALETTE g_lpDDPalette;         // DirectDraw palette
LPDIRECTDRAWCLIPPER g_lpDDClipper;         // DirectDraw clipper
bool                g_bActive;             // Application status
bool                g_bAnimate;            // Event driven animation state
bool                g_bCursorVisible;      // Custom sword cursor animation state
bool                g_bGalleryMode;        // Special mode for users that solve all puzzles
RECT                g_rcWindow;            // Current client area
HWND                g_hWndMain;            // Main window handle
DWORD               g_dwLastTickCount;     // Last frame time
DWORD               g_dwMouseX;            // Mouse pointer's x position in client area
DWORD               g_dwMouseY;            // Mouse pointer's y position in client area
DWORD               g_dwCurrentPuzzle = 1; // Which puzzle is being worked on
GAME_PIECE          g_gamePieceArray[9];   // Defines RECTs for blitting of game pieces
GAME_BOARD          g_gameBoardPosits[9];  // Defines RECTs for hit detection with the mouse

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
bool DoInit( HINSTANCE hInstance, int nCmdShow );
bool InitDDraw( HWND hWnd );
bool UpdateFrame( HWND hWnd );
void SetMenuItems( HWND hWnd );
void MoveGamePieces( int x, int y );
void MoveGamePiecesByPosit( int iPosit1, int iPosit2 );
void SwapBoardPosits( int iPosit1, int iPosit2 );
void ChangePuzzle( DWORD dwSelection );
void UnlockPuzzle( HWND hWnd );
static void ReleaseObjects( void );

//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: 
//-----------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE hInstance, 
                    HINSTANCE hPrevInstance, 
                    LPSTR     lpCmdLine, 
                    int       nCmdShow)
{
    MSG msg;

    memset(&msg,0,sizeof(msg));

    if( !DoInit( hInstance, nCmdShow ) )
    {
        return( FALSE );
    }

    while(1)
    {
        if( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
        {
            if ( !GetMessage( &msg, NULL, 0, 0 ) )
            {
                return msg.wParam;
            }

            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else if( g_bActive == true )
        {
            UpdateFrame( g_hWndMain );
        }
        else
        {
            WaitMessage();
        }
    }
}

//-----------------------------------------------------------------------------
// Name: WindowProc()
// Desc: 
//-----------------------------------------------------------------------------
long FAR PASCAL WindowProc( HWND   hWnd,
                            UINT   message,
                            WPARAM wParam,
                            LPARAM lParam )
{
    switch( message )
    {
         case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDM_ABOUT:
                    MessageBox( hWnd,
                        "This game is dedicated to Jewels, my beautiful "
                        "wife\nand the biggest Xena fan in the whole wide "
                        "world!\n\nProgrammed by Kevin Harris using "
                        "DirectX 7.0.\nCopyright (c) 2001 All Rights "
                        "Reserved.\n\nkevin@codesampler.com", 
                        "About Xena", MB_ICONINFORMATION | MB_OK );
                    break;

                case IDM_SELECTION1:
                    ChangePuzzle(1);
                    break;

                case IDM_SELECTION2:
                    ChangePuzzle(2);
                    break;

                case IDM_SELECTION3:
                    ChangePuzzle(3);
                    break;

                case IDM_SELECTION4:
                    ChangePuzzle(4);
                    break;

                case IDM_SELECTION5:
                    ChangePuzzle(5);
                    break;

                case IDM_SELECTION6:
                    ChangePuzzle(6);
                    break;

                case IDM_SELECTION7:
                    ChangePuzzle(7);
                    break;

                case IDM_SELECTION8:
                    ChangePuzzle(8);
                    break;

                case IDM_SELECTION9:
                    ChangePuzzle(9);
                    break;

                case IDM_SELECTION10:
                    ChangePuzzle(10);
                    break;

                case IDM_SELECTION11:
                    ChangePuzzle(11);
                    break;

                case IDM_SELECTION12:
                    ChangePuzzle(12);
                    break;

                case IDM_GALLERY_MODE:
                    HMENU hMenu;
                    hMenu = GetMenu( hWnd );

                    if( g_bGalleryMode == true )
                    {
                        CheckMenuItem( hMenu, IDM_GALLERY_MODE, MF_UNCHECKED );
                        g_bGalleryMode = false;
                    }
                    else
                    {
                        CheckMenuItem( hMenu, IDM_GALLERY_MODE, MF_CHECKED );
                        g_bGalleryMode = true;
                    }

                    ChangePuzzle( g_dwCurrentPuzzle );
                    break;
            }
            break;

        case WM_MOVE:
            // Our window position has changed, so
            // get the client (drawing) rectangle.
            GetClientRect( hWnd, &g_rcWindow );
            // Convert the coordinates from client relative
            // to screen relative.
            ClientToScreen( hWnd, (LPPOINT)&g_rcWindow );
            ClientToScreen( hWnd, (LPPOINT)&g_rcWindow + 1 );
            g_bAnimate = true;
            return 0;
            break;

        case WM_SIZE:
            // Our window size is fixed, so this could
            // only be a minimize or maximize.
            if( wParam == SIZE_MINIMIZED ) 
            {
                // We've been minimized, no need to
                // redraw the screen.
                InvalidateRect( hWnd, NULL, TRUE );
                g_bActive = false;
            }
            else
            {
                g_bActive = true;
            }
            return 0;
            break;

        case WM_PALETTECHANGED:
            // First check to see if we caused this message.
            if( ( HWND )wParam != hWnd ) 
            {
                // We didn't cause it, so we have lost the palette.
                // Realize our palette.
                g_lpDDSPrimary->SetPalette( g_lpDDPalette );
                // Convert the sprite image to the new palette.
                DDReLoadBitmap( g_lpDDSXena, "Images\\xena1.bmp" );
                g_bAnimate = true;
            }
            break;

        case WM_QUERYNEWPALETTE:
            // We have control of the palette.
            g_lpDDSPrimary->SetPalette( g_lpDDPalette );
            // Convert the sprite image to the new palette.
            DDReLoadBitmap( g_lpDDSXena, "Images\\xena1.bmp" );
            g_bAnimate = true;
            break;

        case WM_KEYDOWN:
            switch( wParam )
            {
                case VK_ESCAPE:
                    PostMessage( hWnd, WM_CLOSE, 0, 0 );
                    break;
            }
            break;

        case WM_MOUSEMOVE:
            g_dwMouseX = LOWORD(lParam);
            g_dwMouseY = HIWORD(lParam);
            g_bAnimate = true;
            g_bCursorVisible = true;
            break;

        case WM_NCMOUSEMOVE:
            g_bCursorVisible = false;
            break;

        case WM_LBUTTONDOWN:
            MoveGamePieces( LOWORD(lParam), HIWORD(lParam) );
            g_bAnimate = true;
            break;

        case WM_DESTROY:
            ReleaseObjects();
            PostQuitMessage( 0 );
            break;
        }

    return DefWindowProc( hWnd, message, wParam, lParam );
}

//-----------------------------------------------------------------------------
// Name: DoInit()
// Desc: 
//-----------------------------------------------------------------------------
bool DoInit( HINSTANCE hInstance, int nCmdShow )
{
    HWND     hWnd;
    WNDCLASS wc;
    RECT     g_rcWindowRec;
    int      i = 0;
    int      j = 0;

    // Set up and register window class.
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc = WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_XENA_ICON));
    wc.hCursor = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_BLANK_CURSOR));
    wc.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU);
    wc.lpszClassName = "MY_WINDOWS_CLASS";
    RegisterClass( &wc );

    // Create a window.
    hWnd = CreateWindowEx( 0, "MY_WINDOWS_CLASS", 
                           "Direct3D (DX8) - Xena Puzzle Game",
                           WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX,
                           0, 0, 0, 0, NULL, NULL, hInstance, NULL );

    if( !hWnd )
        return false;

    g_hWndMain = hWnd; // Save the window handle.

    SetRect( &g_rcWindowRec, 0, 0, 324, 324 );

    // Adjust that to a size that includes the border, etc.
    AdjustWindowRectEx( &g_rcWindowRec,
        GetWindowStyle( hWnd ),     // Style of our main window.
        GetMenu( hWnd ) != NULL,    // Does the window have a menu?
        GetWindowExStyle( hWnd ) ); // Extended style of the main window.

    // Adjust the window to the new size.
    MoveWindow( hWnd, 0, 0, 
                g_rcWindowRec.right - g_rcWindowRec.left, 
                g_rcWindowRec.bottom - g_rcWindowRec.top,  
                FALSE );

    // Create all our DirectDraw objects.
    if( !InitDDraw( hWnd ) ) 
        return false;

    // Populate the RECTs being held by each SPRITE in the array
    // with frame postions to be used later with the surface where we're 
    // storing the xena bitmap.
    for( i = 0; i < 9; ++i )
    {
        g_gamePieceArray[i].rcFrameRect.top    = ( ( i / 3 ) * 108 );
        g_gamePieceArray[i].rcFrameRect.left   = ( ( i % 3 ) * 108 );
        g_gamePieceArray[i].rcFrameRect.bottom = g_gamePieceArray[i].rcFrameRect.top + 108;
        g_gamePieceArray[i].rcFrameRect.right  = g_gamePieceArray[i].rcFrameRect.left + 108;
        g_gamePieceArray[i].nPosition          = i + 1;
        g_gamePieceArray[i].bBlank             = false;

        // We can also use these same calculations to assign rectangular areas for hit detection
        // These will match up mouse clicks with the game board position selected for moving.
        g_gameBoardPosits[i].rcPositRect.top    = ( ( i / 3 ) * 108 );
        g_gameBoardPosits[i].rcPositRect.left   = ( ( i % 3 ) * 108 );
        g_gameBoardPosits[i].rcPositRect.bottom = g_gameBoardPosits[i].rcPositRect.top + 108;
        g_gameBoardPosits[i].rcPositRect.right  = g_gameBoardPosits[i].rcPositRect.left + 108;
        g_gameBoardPosits[i].nPosition          = i + 1;
    }

    // One square must be blank for game piece movement!
    g_gamePieceArray[8].bBlank = true;

    // Reverse order of game pieces for initial scramble!
    j = 9;
    for( i = 0; i < 9; ++i )
    {
        g_gamePieceArray[i].nPosition = j;
        --j;
    }

    // Final scramble moves place blank 
    // spot in bottom right corner
    MoveGamePiecesByPosit(1,4);
    MoveGamePiecesByPosit(4,5);
    MoveGamePiecesByPosit(5,6);
    MoveGamePiecesByPosit(6,9);

    SetMenuItems( hWnd );

    ShowWindow( hWnd, nCmdShow );

    return true;
}

//-----------------------------------------------------------------------------
// Name: InitDDraw()
// Desc: 
//-----------------------------------------------------------------------------
bool InitDDraw( HWND hWnd )
{
    DDSURFACEDESC  ddsd; // Surface description structure.
    DDPIXELFORMAT  ddpf; // Surface format structure.
    DDCOLORKEY     ddck; // Used to set color key

    //// Create the DirectDraw object.
    DirectDrawCreate( NULL, &g_lpDD, NULL );

    // Set normal cooperative level.
    g_lpDD->SetCooperativeLevel( hWnd, DDSCL_NORMAL );

    // We need to set a few values in the surface description structure 
    // before we can create the primary surface.
    ddsd.dwSize = sizeof( ddsd );
    ddsd.dwFlags = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
    // Create the primary surface.
    g_lpDD->CreateSurface( &ddsd, &g_lpDDSPrimary, NULL );

    ddsd.dwSize         = sizeof( ddsd );
    ddsd.dwFlags        = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
    ddsd.dwHeight       = 324;
    ddsd.dwWidth        = 324;

    // Create an off-screen surface for building on. 
    // This is not a true back buffer, so it can't page flip!
    g_lpDD->CreateSurface( &ddsd, &g_lpDDSBack, NULL );

    ddsd.dwSize         = sizeof( ddsd );
    ddsd.dwFlags        = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
    ddsd.dwHeight       = 32;
    ddsd.dwWidth        = 32;

    // Create an off-screen surface for storing the cursor image
    g_lpDD->CreateSurface( &ddsd, &g_lpDDSCursor, NULL );

    // Create a clipper and attach it to the primary surface.
    g_lpDD->CreateClipper( 0, &g_lpDDClipper, NULL );

    // Associate our clipper with our hWnd so it will be updated by Windows.
    g_lpDDClipper->SetHWnd( 0, hWnd );

    // Associate our clipper with the primary surface, so the Blt()
    // function will use it when we blit the bitmap data to the primary surface.
    g_lpDDSPrimary->SetClipper( g_lpDDClipper );

    g_lpDDClipper->Release();

    // Get ready to check the surface format.
    ddpf.dwSize = sizeof( ddpf );
    // Fetch the surface information
    g_lpDDSPrimary->GetPixelFormat( &ddpf );

    if( ddpf.dwFlags & DDPF_PALETTEINDEXED8 )
    {
        // If the desktop is palettized, create a 
        // palette and attach it to the primary surface.
        g_lpDDPalette = DDLoadPalette( g_lpDD, "Images\\xena1.bmp" );
        g_lpDDSPrimary->SetPalette( g_lpDDPalette );
    }

    // Create a surface and load the bitmap containing our future sprite frames into it.
    g_lpDDSXena = DDLoadBitmap( g_lpDD, "Images\\xena1.bmp", 0, 0 );
    g_lpDDSCursor = DDLoadBitmap( g_lpDD, "Images\\cursor.bmp", 0, 0 );

    // Set the color key's low/high values to color 0 for transparent blitting
    // This means that pure black pixels will not be blitted from the source 
    // surface to the dest surface.
    ddck.dwColorSpaceLowValue  = 0;
    ddck.dwColorSpaceHighValue = 0;

    // Now set the color key for 'source' blitting
    g_lpDDSCursor->SetColorKey(DDCKEY_SRCBLT, &ddck);

    g_bAnimate = true;

    return true;
}

//-----------------------------------------------------------------------------
// Name: UpdateFrame()
// Desc: 
//-----------------------------------------------------------------------------
bool UpdateFrame( HWND hWnd )
{
    RECT    rcPrimaryRec;
    RECT    rcBackDest;
    RECT    rcMouseDestRec;
    RECT    rcMouseSrcRec;
    DWORD   dwTickCount;
    DDBLTFX ddbltfx;
    bool    bSolutionFound = true;
    int     i = 0;

    if( g_bAnimate == false )
    {
        // Even if no event has forced a redraw, check for the timing interval
        // we can't let the client area go unrefreshed for too long.
        dwTickCount = GetTickCount();
        if( ( dwTickCount - g_dwLastTickCount ) <= FRAME_DELAY )
        {
            return true;
        }
        g_dwLastTickCount = dwTickCount;
    }

    // Check for solution state
    // Assume solved, then test assumption by double checking it!
    for( i = 0; i <= 8; ++i )
    {
        if( g_gamePieceArray[i].nPosition != g_gameBoardPosits[i].nPosition )
        {
            bSolutionFound = false;
        }
    }

    if( bSolutionFound == true )
    {
        // Solution found!
        // Unblank the missing piece to complete the chPuzzle's image
        for( i = 0; i <= 8; ++i )
        {
            if( g_gamePieceArray[i].bBlank == true )
            {
                g_gamePieceArray[i].bBlank = false;

                UnlockPuzzle(hWnd);
            }
        }

        g_bAnimate = true;
    }

    // Clear the back buffer to black using the blitter.
    ddbltfx.dwSize = sizeof( ddbltfx );
    ddbltfx.dwFillColor = 0; //Black
    g_lpDDSBack->Blt( NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx );

    for( i = 0; i < 9; ++i )
    {
        if( g_gamePieceArray[i].nPosition == 1 && 
            g_gamePieceArray[i].bBlank != true )
        {
            // Game board Position 1
            rcBackDest.top    = 0;
            rcBackDest.left   = 0;
            rcBackDest.bottom = 108;
            rcBackDest.right  = 108;

            g_lpDDSBack->Blt( &rcBackDest, g_lpDDSXena, 
                &g_gamePieceArray[i].rcFrameRect, DDBLT_WAIT, NULL );
        }

        if( g_gamePieceArray[i].nPosition == 2 && 
            g_gamePieceArray[i].bBlank != true )
        {
            // Game board Position 2
            rcBackDest.top    = 0;
            rcBackDest.left   = 108;
            rcBackDest.bottom = 108;
            rcBackDest.right  = 216;

            g_lpDDSBack->Blt( &rcBackDest, g_lpDDSXena, 
                &g_gamePieceArray[i].rcFrameRect, DDBLT_WAIT, NULL );
        }

        if( g_gamePieceArray[i].nPosition == 3 && 
            g_gamePieceArray[i].bBlank != true )
        {
            // Game board Position 3
            rcBackDest.top    = 0;
            rcBackDest.left   = 216;
            rcBackDest.bottom = 108;
            rcBackDest.right  = 324;

            g_lpDDSBack->Blt( &rcBackDest, g_lpDDSXena, 
                &g_gamePieceArray[i].rcFrameRect, DDBLT_WAIT, NULL );
        }

        if( g_gamePieceArray[i].nPosition == 4 && 
            g_gamePieceArray[i].bBlank != true )
        {
            // Game board Position 4
            rcBackDest.top    = 108;
            rcBackDest.left   = 0;
            rcBackDest.bottom = 216;
            rcBackDest.right  = 108;

            g_lpDDSBack->Blt( &rcBackDest, g_lpDDSXena, 
                &g_gamePieceArray[i].rcFrameRect, DDBLT_WAIT, NULL );
        }

        if( g_gamePieceArray[i].nPosition == 5 && 
            g_gamePieceArray[i].bBlank != true )
        {
            // Game board Position 5
            rcBackDest.top    = 108;
            rcBackDest.left   = 108;
            rcBackDest.bottom = 216;
            rcBackDest.right  = 216;

            g_lpDDSBack->Blt( &rcBackDest, g_lpDDSXena, 
                &g_gamePieceArray[i].rcFrameRect, DDBLT_WAIT, NULL );
        }

        if( g_gamePieceArray[i].nPosition == 6 && 
            g_gamePieceArray[i].bBlank != true )
        {
            // Game board Position 6
            rcBackDest.top    = 108;
            rcBackDest.left   = 216;
            rcBackDest.bottom = 216;
            rcBackDest.right  = 324;

            g_lpDDSBack->Blt( &rcBackDest, g_lpDDSXena, 
                &g_gamePieceArray[i].rcFrameRect, DDBLT_WAIT, NULL );
        }

        if( g_gamePieceArray[i].nPosition == 7 && 
            g_gamePieceArray[i].bBlank != true )
        {
            // Game board Position 7
            rcBackDest.top    = 216;
            rcBackDest.left   = 0;
            rcBackDest.bottom = 324;
            rcBackDest.right  = 108;

            g_lpDDSBack->Blt( &rcBackDest, g_lpDDSXena, 
                &g_gamePieceArray[i].rcFrameRect, DDBLT_WAIT, NULL );
        }

        if( g_gamePieceArray[i].nPosition == 8 && 
            g_gamePieceArray[i].bBlank != true )
        {
            // Game board Position 8
            rcBackDest.top    = 216;
            rcBackDest.left   = 108;
            rcBackDest.bottom = 324;
            rcBackDest.right  = 216;

            g_lpDDSBack->Blt( &rcBackDest, g_lpDDSXena, 
                &g_gamePieceArray[i].rcFrameRect, DDBLT_WAIT, NULL );
        }

        if( g_gamePieceArray[i].nPosition == 9 && 
            g_gamePieceArray[i].bBlank != true )
        {
            // Game board Position 9
            rcBackDest.top    = 216;
            rcBackDest.left   = 216;
            rcBackDest.bottom = 324;
            rcBackDest.right  = 324;

            g_lpDDSBack->Blt( &rcBackDest, g_lpDDSXena, 
                &g_gamePieceArray[i].rcFrameRect, DDBLT_WAIT, NULL );
        }
    }

    // Where the mouse cursor will be blitted to
    rcMouseDestRec.top    = g_dwMouseY;
    rcMouseDestRec.left   = g_dwMouseX;
    rcMouseDestRec.bottom = rcMouseDestRec.top  + 32;
    rcMouseDestRec.right  = rcMouseDestRec.left + 32;

    // Where the cursor will blitted from
    // Normal source rect with no manual clipping for the 
    // sword pointer being half in and half out of the client area.
    rcMouseSrcRec.top    = 0;
    rcMouseSrcRec.left   = 0;
    rcMouseSrcRec.bottom = 32;
    rcMouseSrcRec.right  = 32;

    if( rcMouseDestRec.bottom > 324) 
    {
        // Oops... the sword pointer is not completely inside the client area
        // We need to draw only part of the sword!
        rcMouseSrcRec.bottom = rcMouseSrcRec.bottom - ( rcMouseDestRec.bottom - 324 );
        rcMouseDestRec.bottom = 324;
    }

    if( rcMouseDestRec.right > 324)
    {
        // Same as above, but we now need to clip our sword cursor to the 
        // right edge of the client area this time!
        rcMouseSrcRec.right = rcMouseSrcRec.right - ( rcMouseDestRec.right - 324 );
        rcMouseDestRec.right = 324;
    }

    if( g_bCursorVisible == true )
    {
        g_lpDDSBack->Blt( &rcMouseDestRec, g_lpDDSCursor, &rcMouseSrcRec, DDBLT_WAIT | DDBLT_KEYSRC, NULL );
    }

    rcPrimaryRec.top    = (g_rcWindow.top);
    rcPrimaryRec.left   = (g_rcWindow.left);
    rcPrimaryRec.bottom = (g_rcWindow.top  + 324);
    rcPrimaryRec.right  = (g_rcWindow.left + 324);

    g_lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN,0);
    g_lpDDSPrimary->Blt( &rcPrimaryRec, g_lpDDSBack, NULL, DDBLT_WAIT, NULL );

    return true;
}

//-----------------------------------------------------------------------------
// Name: MoveGamePieces()
// Desc: 
//-----------------------------------------------------------------------------
void MoveGamePieces( int x, int y )
{
    int iClickedPosit = 0;
    int iPositHolder  = 0;
    int i = 0;
    int j = 0;
    int k = 0;

    for( k = 0; k  < 9; ++k )
    {
        // Which position was clicked?
        if( g_gameBoardPosits[k].rcPositRect.left   < x &&
            g_gameBoardPosits[k].rcPositRect.right  > x &&
            g_gameBoardPosits[k].rcPositRect.top    < y &&
            g_gameBoardPosits[k].rcPositRect.bottom > y    )
        {
            // Ok we clicked here!
            iClickedPosit = g_gameBoardPosits[k].nPosition;

            for( i = 0; i < 9; ++i )
            {
                if( g_gamePieceArray[i].nPosition == iClickedPosit && 
                    g_gamePieceArray[i].bBlank == false )
                {
                    // They clicked on a valid game piece that isn't the blank!
                    // Now, make sure it's next to the blank space, then move it there!
                    if( g_gamePieceArray[i].nPosition == 1 )
                    {
                        // This piece can only be moved to positions 2 or 4
                        // Check those postions for the blank!
                        for( j = 0; j < 9; ++j )
                        {
                            if( ( g_gamePieceArray[j].nPosition == 2 || 
                                  g_gamePieceArray[j].nPosition == 4 ) && 
                                  g_gamePieceArray[j].bBlank == true )
                            {
                                SwapBoardPosits( i, j );
                                goto bailOut;
                            }
                        }
                        
                        goto bailOut;
                    }

                    if( g_gamePieceArray[i].nPosition == 2 )
                    {
                        // This piece can only be moved to positions 1 or 3 or 5
                        // Check those postions for the blank!
                        for( int j = 0; j < 9; ++j )
                        {
                            if( ( g_gamePieceArray[j].nPosition == 1 || 
                                  g_gamePieceArray[j].nPosition == 3 || 
                                  g_gamePieceArray[j].nPosition == 5 ) && 
                                  g_gamePieceArray[j].bBlank == true )
                            {
                                SwapBoardPosits( i, j );
                                goto bailOut;
                            }
                        }

                        goto bailOut;
                    }

                    if( g_gamePieceArray[i].nPosition == 3 )
                    {
                        // This piece can only be moved to positions 2 or 6
                        // Check those postions for the blank!
                        for( int j = 0; j < 9; ++j )
                        {
                            if( ( g_gamePieceArray[j].nPosition == 2 || 
                                  g_gamePieceArray[j].nPosition == 6 ) && 
                                  g_gamePieceArray[j].bBlank == true )
                            {
                                SwapBoardPosits( i, j );
                                goto bailOut;
                            }
                        }

                        goto bailOut;
                    }

                    if( g_gamePieceArray[i].nPosition == 4 )
                    {
                        // This piece can only be moved to positions 1 or 5 or 7
                        // Check those postions for the blank!
                        for( int j = 0; j < 9; ++j )
                        {
                            if( ( g_gamePieceArray[j].nPosition == 1 || 
                                  g_gamePieceArray[j].nPosition == 5 || 
                                  g_gamePieceArray[j].nPosition == 7 ) && 
                                  g_gamePieceArray[j].bBlank == true )
                            { 
                                SwapBoardPosits( i, j );
                                goto bailOut;
                            }
                        }

                        goto bailOut;
                    }

                    if( g_gamePieceArray[i].nPosition == 5 )
                    {
                        // This piece can only be moved to positions 2 or 4 or 6 or 8
                        // Check those postions for the blank!
                        for( int j = 0; j < 9; ++j )
                        {
                            if( ( g_gamePieceArray[j].nPosition == 2 || 
                                  g_gamePieceArray[j].nPosition == 4 || 
                                  g_gamePieceArray[j].nPosition == 6 || 
                                  g_gamePieceArray[j].nPosition == 8 ) && 
                                  g_gamePieceArray[j].bBlank == true )
                            {
                                SwapBoardPosits( i, j );
                                goto bailOut;
                            }
                        }

                        goto bailOut;
                    }

                    if( g_gamePieceArray[i].nPosition == 6 )
                    {
                        // This piece can only be moved to positions 3 or 5 or 9
                        // Check those postions for the blank!
                        for( int j = 0; j < 9; ++j )
                        {
                            if( ( g_gamePieceArray[j].nPosition == 3 || 
                                  g_gamePieceArray[j].nPosition == 5 || 
                                  g_gamePieceArray[j].nPosition == 9 ) && 
                                  g_gamePieceArray[j].bBlank == true )
                            {
                                SwapBoardPosits( i, j );
                                goto bailOut;
                            }
                        }

                        goto bailOut;
                    }

                    if( g_gamePieceArray[i].nPosition == 7 )
                    {
                        // This piece can only be moved to positions 4 or 8
                        // Check those postions for the blank!
                        for( int j = 0; j < 9; ++j )
                        {
                            if( ( g_gamePieceArray[j].nPosition == 4 || 
                                  g_gamePieceArray[j].nPosition == 8 ) && 
                                  g_gamePieceArray[j].bBlank == true )
                            {
                                SwapBoardPosits( i, j );
                                goto bailOut;
                            }
                        }

                        goto bailOut;
                    }

                    if( g_gamePieceArray[i].nPosition == 8 )
                    {
                        // This piece can only be moved to positions 7 or 5 or 9
                        // Check those postions for the blank!
                        for( int j = 0; j < 9; ++j )
                        {
                            if( ( g_gamePieceArray[j].nPosition == 7 || 
                                  g_gamePieceArray[j].nPosition == 5 || 
                                  g_gamePieceArray[j].nPosition == 9 ) && 
                                  g_gamePieceArray[j].bBlank == true )
                            {
                                SwapBoardPosits( i, j );
                                goto bailOut;
                            }
                        }

                        goto bailOut;
                    }

                    if( g_gamePieceArray[i].nPosition == 9 )
                    {
                        // This piece can only be moved to positions 6 or 8
                        // Check those postions for the blank!
                        for( int j = 0; j < 9; ++j )
                        {
                            if( ( g_gamePieceArray[j].nPosition == 6 || 
                                  g_gamePieceArray[j].nPosition == 8 ) && 
                                  g_gamePieceArray[j].bBlank == true )
                            { 
                                SwapBoardPosits( i, j );
                                goto bailOut;
                            }
                        }

                        goto bailOut;
                    }
                }
            }
        }
    }

    bailOut:;
}

//-----------------------------------------------------------------------------
// Name: MoveGamePiecesByPosit()
// Desc: 
//-----------------------------------------------------------------------------
void MoveGamePiecesByPosit( int iPosit1, int iPosit2 )
{
    int i = 0;
    int j = 0;

    for( i = 0; i < 9; ++i )
    {
        if( g_gamePieceArray[i].nPosition == iPosit1 )
        {
            for( j = 0; j < 9; ++j )
            {
                if( g_gamePieceArray[j].nPosition == iPosit2 )
                    SwapBoardPosits(i,j);
            }
        }
    }                   
}

//-----------------------------------------------------------------------------
// Name: SwapBoardPosits()
// Desc: 
//-----------------------------------------------------------------------------
void SwapBoardPosits( int iPosit1, int iPosit2 )
{
    int iPositHolder = 0;

    iPositHolder = g_gamePieceArray[iPosit1].nPosition;
    g_gamePieceArray[iPosit1].nPosition = g_gamePieceArray[iPosit2].nPosition;
    g_gamePieceArray[iPosit2].nPosition = iPositHolder;
}

//-----------------------------------------------------------------------------
// Name: ChangePuzzle()
// Desc: 
//-----------------------------------------------------------------------------
void ChangePuzzle( DWORD dwSelection )
{
    char chValueBuffer[10] = {NULL};
    char chPath[100] = "Images\\xena";
    int  i = 0;
    int  j = 0;

    _itoa( dwSelection, chValueBuffer, 10 );

    strcat( chPath, chValueBuffer );
    strcat( chPath, ".bmp" );

    g_lpDDSXena = DDLoadBitmap( g_lpDD, chPath, 0, 0 );

    g_dwCurrentPuzzle = dwSelection;

    for( i = 0; i < 9; ++i )
    {
        g_gamePieceArray[i].rcFrameRect.top    = ( ( i / 3 ) * 108 );
        g_gamePieceArray[i].rcFrameRect.left   = ( ( i % 3 ) * 108 );
        g_gamePieceArray[i].rcFrameRect.bottom = g_gamePieceArray[i].rcFrameRect.top + 108;
        g_gamePieceArray[i].rcFrameRect.right  = g_gamePieceArray[i].rcFrameRect.left + 108;
        g_gamePieceArray[i].nPosition          = i + 1;
        g_gamePieceArray[i].bBlank             = false;
    }

    if( g_bGalleryMode == false )
    {
        // One square must be blank for game piece movement!
        g_gamePieceArray[8].bBlank = true;

        // Reverse order of game pieces for initial scramble!
        j = 9;
        for( i = 0; i < 9; ++i )
        {
            g_gamePieceArray[i].nPosition = j;
            --j;
        }

        // Final scramble moves place blank 
        // spot in bottom right corner
        MoveGamePiecesByPosit(1,4);
        MoveGamePiecesByPosit(4,5);
        MoveGamePiecesByPosit(5,6);
        MoveGamePiecesByPosit(6,9);
    }
}

//-----------------------------------------------------------------------------
// Name: UnlockPuzzle()
// Desc: 
//-----------------------------------------------------------------------------
void UnlockPuzzle( HWND hWnd )
{
    HMENU hMenu;
    char  chValueBuffer[10] = {NULL};
    char  chPuzzle[100]     = "PUZZLE";
 
    // Make the next chPuzzle available!
    hMenu = GetMenu( hWnd );

    // Ungray the next menu!
    if( g_dwCurrentPuzzle == 1 )  EnableMenuItem( hMenu, IDM_SELECTION2,  MF_ENABLED );
    if( g_dwCurrentPuzzle == 2 )  EnableMenuItem( hMenu, IDM_SELECTION3,  MF_ENABLED );
    if( g_dwCurrentPuzzle == 3 )  EnableMenuItem( hMenu, IDM_SELECTION4,  MF_ENABLED );
    if( g_dwCurrentPuzzle == 4 )  EnableMenuItem( hMenu, IDM_SELECTION5,  MF_ENABLED );
    if( g_dwCurrentPuzzle == 5 )  EnableMenuItem( hMenu, IDM_SELECTION6,  MF_ENABLED );
    if( g_dwCurrentPuzzle == 6 )  EnableMenuItem( hMenu, IDM_SELECTION7,  MF_ENABLED );
    if( g_dwCurrentPuzzle == 7 )  EnableMenuItem( hMenu, IDM_SELECTION8,  MF_ENABLED );
    if( g_dwCurrentPuzzle == 8 )  EnableMenuItem( hMenu, IDM_SELECTION9,  MF_ENABLED );
    if( g_dwCurrentPuzzle == 9 )  EnableMenuItem( hMenu, IDM_SELECTION10, MF_ENABLED );
    if( g_dwCurrentPuzzle == 10 ) EnableMenuItem( hMenu, IDM_SELECTION11, MF_ENABLED );
    if( g_dwCurrentPuzzle == 11 ) EnableMenuItem( hMenu, IDM_SELECTION12, MF_ENABLED );
    if( g_dwCurrentPuzzle == 12 ) EnableMenuItem( hMenu, IDM_GALLERY_MODE, MF_ENABLED );

    // log the correct access code to the proper key of the xena.ini file
    _itoa( (g_dwCurrentPuzzle + 1), chValueBuffer, 10 );
    strcat( chPuzzle, chValueBuffer );

    if( g_dwCurrentPuzzle == 1 )  ::WritePrivateProfileString( "XENA CODES", chPuzzle, "3698", "xena.ini" );
    if( g_dwCurrentPuzzle == 2 )  ::WritePrivateProfileString( "XENA CODES", chPuzzle, "5874", "xena.ini" );
    if( g_dwCurrentPuzzle == 3 )  ::WritePrivateProfileString( "XENA CODES", chPuzzle, "2547", "xena.ini" );
    if( g_dwCurrentPuzzle == 4 )  ::WritePrivateProfileString( "XENA CODES", chPuzzle, "9852", "xena.ini" );
    if( g_dwCurrentPuzzle == 5 )  ::WritePrivateProfileString( "XENA CODES", chPuzzle, "3652", "xena.ini" );
    if( g_dwCurrentPuzzle == 6 )  ::WritePrivateProfileString( "XENA CODES", chPuzzle, "8436", "xena.ini" );
    if( g_dwCurrentPuzzle == 7 )  ::WritePrivateProfileString( "XENA CODES", chPuzzle, "8547", "xena.ini" );
    if( g_dwCurrentPuzzle == 8 )  ::WritePrivateProfileString( "XENA CODES", chPuzzle, "5614", "xena.ini" );
    if( g_dwCurrentPuzzle == 9 )  ::WritePrivateProfileString( "XENA CODES", chPuzzle, "8649", "xena.ini" );
    if( g_dwCurrentPuzzle == 10 ) ::WritePrivateProfileString( "XENA CODES", chPuzzle, "6287", "xena.ini" );
    if( g_dwCurrentPuzzle == 11 ) ::WritePrivateProfileString( "XENA CODES", chPuzzle, "7468", "xena.ini" );
    if( g_dwCurrentPuzzle == 12 ) ::WritePrivateProfileString( "XENA CODES", "COMPLETE", "0702", "xena.ini" );
}

//-----------------------------------------------------------------------------
// Name: SetMenuItems()
// Desc: 
//-----------------------------------------------------------------------------
void SetMenuItems( HWND hWnd )
{
    HMENU hMenu;
    char  chValueBuffer[10] = {NULL};
    char  chTextBuffer[100] = {NULL};
    char  chPuzzle[100]     = {NULL};

    hMenu = GetMenu( hWnd );

    for( int i = 1; i <= 12; ++i )
    {
        // xena.ini must be in the Windows folder for this next calls to work properly
        strset( chPuzzle, NULL );
        strcat( chPuzzle, "PUZZLE" );
        _itoa( i, chValueBuffer, 10 );
        strcat( chPuzzle, chValueBuffer );

        ::GetPrivateProfileString( "XENA CODES",chPuzzle, chTextBuffer, chTextBuffer, 100,"xena.ini" );

        if( strcmp( chTextBuffer,"3698" ) == 0 ) EnableMenuItem( hMenu, IDM_SELECTION2,  MF_ENABLED );
        if( strcmp( chTextBuffer,"5874" ) == 0 ) EnableMenuItem( hMenu, IDM_SELECTION3,  MF_ENABLED );
        if( strcmp( chTextBuffer,"2547" ) == 0 ) EnableMenuItem( hMenu, IDM_SELECTION4,  MF_ENABLED );
        if( strcmp( chTextBuffer,"9852" ) == 0 ) EnableMenuItem( hMenu, IDM_SELECTION5,  MF_ENABLED );
        if( strcmp( chTextBuffer,"3652" ) == 0 ) EnableMenuItem( hMenu, IDM_SELECTION6,  MF_ENABLED );
        if( strcmp( chTextBuffer,"8436" ) == 0 ) EnableMenuItem( hMenu, IDM_SELECTION7,  MF_ENABLED );
        if( strcmp( chTextBuffer,"8547" ) == 0 ) EnableMenuItem( hMenu, IDM_SELECTION8,  MF_ENABLED );
        if( strcmp( chTextBuffer,"5614" ) == 0 ) EnableMenuItem( hMenu, IDM_SELECTION9,  MF_ENABLED );
        if( strcmp( chTextBuffer,"8649" ) == 0 ) EnableMenuItem( hMenu, IDM_SELECTION10, MF_ENABLED );
        if( strcmp( chTextBuffer,"6287" ) == 0 ) EnableMenuItem( hMenu, IDM_SELECTION11, MF_ENABLED );
        if( strcmp( chTextBuffer,"7468" ) == 0 ) EnableMenuItem( hMenu, IDM_SELECTION12, MF_ENABLED );
    }

    ::GetPrivateProfileString( "XENA CODES", "COMPLETE", chTextBuffer, chTextBuffer, 100,"xena.ini" );

    if( strcmp( chTextBuffer,"0702" ) == 0 ) EnableMenuItem( hMenu, IDM_GALLERY_MODE, MF_ENABLED );
}

//-----------------------------------------------------------------------------
// Name: ReleaseObjects()
// Desc: 
//-----------------------------------------------------------------------------
static void ReleaseObjects( void )
{
    // ReleaseObjects is responsible for cleaning up DirectX
    // after the application is finshed.
    if ( g_lpDD != NULL )
    {
        if( g_lpDDSPrimary != NULL ) 
        {
            g_lpDDSPrimary->Release(); // Clipper is also released here.
            g_lpDDSPrimary = NULL;
        }

        if( g_lpDDSBack != NULL ) 
        {
            g_lpDDSBack->Release();
            g_lpDDSBack = NULL;
        }

        if( g_lpDDSXena != NULL ) 
        {
            g_lpDDSXena->Release();
            g_lpDDSXena = NULL;
        }

        if( g_lpDDSCursor != NULL ) 
        {
            g_lpDDSCursor->Release();
            g_lpDDSCursor = NULL;
        }

        if ( g_lpDDPalette != NULL ) 
        {
            g_lpDDPalette->Release();
            g_lpDDPalette = NULL;
        }

        g_lpDD->Release();
        g_lpDD = NULL;
    }
}