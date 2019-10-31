//-----------------------------------------------------------------------------
//           Name: dx8_clipper.cpp
//         Author: Kevin Harris
//  Last Modified: 03/03/04
//    Description: This sample attaches a clipper to the back buffer so 
//                 sprites can move off-screen smoothly with out blinking
//                 out of existence suddenly.
//
//  Special Notes: DirectX 8.0 still uses 7.0 interfaces for DirectDraw!
//-----------------------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <ddraw.h>
#include "ddutil.h"

//-----------------------------------------------------------------------------
// MACROS
//-----------------------------------------------------------------------------
#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
LPDIRECTDRAW7        g_lpdd7;        // DirectDraw object
LPDIRECTDRAWSURFACE7 g_lpddsPrimary; // DirectDraw primary surface
LPDIRECTDRAWSURFACE7 g_lpddsBack;    // DirectDraw back buffer surface
LPDIRECTDRAWSURFACE7 g_lpddsDonut;   // DirectDraw off-screen surface for the donut bitmap.

//-----------------------------------------------------------------------------
// SYMBOLIC CONSTANTS
//-----------------------------------------------------------------------------
const int FRAME_DELAY   = 15;  // Frame delay or animation speed.
const int SCREEN_WIDTH  = 640;
const int SCREEN_HEIGHT = 480;
const int COLOR_DEPTH   = 8;

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
HRESULT init( HWND hWnd );
HRESULT render( void );
void    shutDown( void );

void AttachClipper( LPDIRECTDRAWSURFACE7 lpdds, LPRECT lpRect );

//-----------------------------------------------------------------------------
// Name: WindowProc()
// Desc: The window's message handler
//-----------------------------------------------------------------------------
LRESULT CALLBACK WindowProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{	
        case WM_SETCURSOR:
		{
            SetCursor( NULL );
            return(1);
		}
		break;
 
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
int WINAPI WinMain(	HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPSTR     lpCmdLine,
					int       nCmdShow )
{
	WNDCLASSEX winClass; 
	HWND       hWnd;
	MSG        msg;

    memset(&msg,0,sizeof(msg));

	winClass.lpszClassName	= "MY_WINDOWS_CLASS";
	winClass.cbSize         = sizeof(WNDCLASSEX);
	winClass.style			= CS_DBLCLKS | CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	winClass.lpfnWndProc	= WindowProc;
	winClass.hInstance		= hInstance;
	winClass.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
	winClass.hIconSm        = LoadIcon(NULL, IDI_APPLICATION);
	winClass.hCursor		= LoadCursor(NULL, IDC_ARROW);
	winClass.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
	winClass.lpszMenuName	= NULL;
	winClass.cbClsExtra		= 0;
	winClass.cbWndExtra		= 0;

	if( !RegisterClassEx(&winClass) )
		return(0);

	hWnd = CreateWindowEx( NULL,"MY_WINDOWS_CLASS",
						   "Direct3D (DX8) - Clipper Sample",
                           WS_POPUP | WS_VISIBLE,
					 	   0, 0, 640, 480, NULL, NULL, hInstance, NULL );

	if( hWnd == NULL )
		return E_FAIL;

    ShowWindow( hWnd, nCmdShow );
    UpdateWindow( hWnd );

	init( hWnd );

	while( msg.message != WM_QUIT )
	{
		if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{ 
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
        else
		    render();
	}

	shutDown();

    UnregisterClass( "MY_WINDOWS_CLASS", winClass.hInstance );

	return msg.wParam;
}

//-----------------------------------------------------------------------------
// Name: init()
// Desc: Initializes everything.
//-----------------------------------------------------------------------------
HRESULT init( HWND hWnd )
{
	DDSURFACEDESC2      ddsd;        // DirectDraw surface description structure
	DDSCAPS2            ddscaps;     // DirectDraw surface capabilities structure.
	LPDIRECTDRAWPALETTE lpddPalette; // DirectDraw palette.

	DirectDrawCreateEx( NULL, (LPVOID*)&g_lpdd7, IID_IDirectDraw7, NULL);

    g_lpdd7->SetCooperativeLevel( hWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN );
  
    g_lpdd7->SetDisplayMode( SCREEN_WIDTH, SCREEN_HEIGHT, COLOR_DEPTH, 0, 0 );

	ZeroMemory( &ddsd, sizeof(ddsd) );
    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | 
                          DDSCAPS_COMPLEX | DDSCAPS_VIDEOMEMORY;
    ddsd.dwBackBufferCount = 1;
	ddsd.dwWidth  = SCREEN_WIDTH;
	ddsd.dwHeight = SCREEN_HEIGHT;
  
    g_lpdd7->CreateSurface( &ddsd, &g_lpddsPrimary, NULL );

	ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
	ddscaps.dwCaps2 = 0;
	ddscaps.dwCaps3 = 0;
	ddscaps.dwCaps4 = 0;
	g_lpddsPrimary->GetAttachedSurface( &ddscaps, &g_lpddsBack );

    // Create and set the palette
    lpddPalette = DDLoadPalette( g_lpdd7, "donut.bmp" );
    g_lpddsPrimary->SetPalette(lpddPalette);
	
    g_lpddsDonut = DDLoadBitmap( g_lpdd7, "donut.bmp", 0, 0 );

	DDSetColorKey(g_lpddsDonut, CLR_INVALID);

	// Create a RECT that covers the whole screen so any sprite that
	// lies on the edge will be clipped properly. This will prevent
	// sprites from suddenly popping onto the screen!
	RECT rcScreen = {0,0,SCREEN_WIDTH,SCREEN_HEIGHT};

	// Use the AttachClipper() function to create a clipper that's defined 
	// by the RECT passed in and attach it to the surface specified.
	AttachClipper( g_lpddsBack, &rcScreen );

	return S_OK;
}


//-----------------------------------------------------------------------------
// Name: render()
// Desc: Draw all 3D Objects to the screen...
//-----------------------------------------------------------------------------
HRESULT render( void )
{
	DDBLTFX ddbltfx;
	RECT    rcFrame;
    RECT    rcDest;
    double  dTickCount;

	static double dLastTickCount; // Last frame time.
	static int    nFrame;         // Current sprite frame.
	static int    nPosition = -64;// Current position of sprite.

	dTickCount = GetTickCount();

    if( ( dTickCount - dLastTickCount ) <= FRAME_DELAY )
		return S_OK;

    dLastTickCount = dTickCount;

    ddbltfx.dwSize = sizeof( ddbltfx );
    ddbltfx.dwFillColor = 0;
    g_lpddsBack->Blt( NULL, NULL, NULL,DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx );

	// Calculate which donut frame to use next from the bitmap!
    rcFrame.top    = (( nFrame / 5 ) * 64);
    rcFrame.left   = (( nFrame % 5 ) * 64);
    rcFrame.bottom = rcFrame.top  + 64;
    rcFrame.right  = rcFrame.left + 64;

	++nFrame;

    if( nFrame > 29 )
		nFrame = 0;

	// Calculate the donuts new position as it moves across the screen
	nPosition += 1;
	
    if( nPosition > 704 )
		nPosition = -64;

    rcDest.left   = nPosition;
	rcDest.top    = 200;
	rcDest.bottom = rcDest.top  + 64;
	rcDest.right  = rcDest.left + 64;

	// Blit the donut's next frame from the off-screen surface called, lpDDSDonut
	// to the back buffer surface called, lpDDSBack!
	g_lpddsBack->Blt( &rcDest, g_lpddsDonut, &rcFrame, DDBLT_WAIT | DDBLT_KEYSRC, NULL );

    // Finally, perform the flip.
    g_lpddsPrimary->Flip( NULL, DDFLIP_WAIT );

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: Release and cleanup everything.
//-----------------------------------------------------------------------------
void shutDown()
{
	SAFE_RELEASE( g_lpddsDonut );
	SAFE_RELEASE( g_lpddsPrimary );
	SAFE_RELEASE( g_lpdd7 );
}

//-----------------------------------------------------------------------------
// Name: AttachClipper()
// Desc: Attaches a clipper to a DirectDraw surface
//-----------------------------------------------------------------------------
void AttachClipper( LPDIRECTDRAWSURFACE7 lpdds, LPRECT lpRect )
{                        
	LPDIRECTDRAWCLIPPER lpddClipper;
	LPRGNDATA           lpRgnd;

	// first create the direct draw clipper
	if(FAILED(g_lpdd7->CreateClipper( 0, &lpddClipper, NULL) ))
	   return;

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
	if(FAILED(lpddClipper->SetClipList( lpRgnd, 0 )))
	{
	   free(lpRgnd);
	   return;
	}

	// Attach the clipper to the surface
	if(FAILED(lpdds->SetClipper( lpddClipper )))
	{
	   free(lpRgnd);
	   return;
	}

	// Release allocated memory!
	free( lpRgnd );
}
