//-----------------------------------------------------------------------------
//           Name: dx8_stars.cpp
//         Author: Kevin Harris
//  Last Modified: 03/03/04
//    Description: Star field simulation.
//
//           Note: DirectX 8.0 still uses 7.0 interfaces for DirectDraw!
//                 Use arrow keys to change direction of star scrolling
//-----------------------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <ddraw.h>

//-----------------------------------------------------------------------------
// MACROS
//-----------------------------------------------------------------------------
#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

//-----------------------------------------------------------------------------
// SYMBOLIC CONSTANTS
//-----------------------------------------------------------------------------
const int TOTAL_STARS   = 250; // Density of star field
const int FRAME_DELAY   = 15;  // Frame delay or animation speed.
const int SCREEN_WIDTH  = 640;
const int SCREEN_HEIGHT = 480;
const int COLOR_DEPTH   = 8;

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
LPDIRECTDRAW7        g_lpdd7;        // DirectDraw object
LPDIRECTDRAWSURFACE7 g_lpddsPrimary; // DirectDraw primary surface
LPDIRECTDRAWSURFACE7 g_lpddsBack;    // DirectDraw back surface
bool                 g_bScrollLeft;  // Scrolling direction

struct STAR
{
    int x;          // Star posit x
	int y;          // Star posit y
    int velocity;   // Star velocity
};

STAR stars[TOTAL_STARS]; // Starfield array

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
HRESULT init( HWND hWnd );
HRESULT render( void );
void    shutDown( void );
void ScrollStars();
void InitializeStars();



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
				case VK_LEFT:
					g_bScrollLeft = true;
					break;

				case VK_RIGHT:
					g_bScrollLeft = false;
					break;

				case VK_ESCAPE:
					PostQuitMessage( 0 );
					break;
			}
		}
        break;

		case WM_SETCURSOR:
		{
            SetCursor( NULL );
            return(1);
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
						   "Direct3D (DX8) - Sprite Animation",
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

void InitializeStars()
{
	for( int i = 0; i < TOTAL_STARS; i++ )
	{
		stars[i].x = rand()%SCREEN_WIDTH;
		stars[i].y = rand()%SCREEN_HEIGHT;
		stars[i].velocity = 1 + rand()%16;
	}
}

void ScrollStars()
{
	if( g_bScrollLeft == true )
	{
		// Scroll stars to the left
		for( int i = 0; i< TOTAL_STARS; i++ )
		{
			// Move the star
			stars[i].x -= stars[i].velocity;

			// If the star falls off the screen's edge, wrap it around
			if( stars[i].x <= 0 )
				stars[i].x = SCREEN_WIDTH;
		}
	}
	else
	{
		// Scroll stars to the right
		for( int i = 0; i < TOTAL_STARS; i++ )
		{
			// Move the star
			stars[i].x += stars[i].velocity;

			// If the star falls off the screen's edge, wrap it around
			if( stars[i].x >= SCREEN_WIDTH )
				stars[i].x -= SCREEN_WIDTH;
		}
	}
}

//-----------------------------------------------------------------------------
// Name: init()
// Desc: Initializes everything.
//-----------------------------------------------------------------------------
HRESULT init( HWND hWnd )
{
	DDSURFACEDESC2 ddsd;    // DirectDraw surface description structure
	DDSCAPS2       ddscaps; // DirectDraw surface capabilities structure.

	// Create the main DirectDraw object
	DirectDrawCreateEx( NULL, (LPVOID*)&g_lpdd7, IID_IDirectDraw7, NULL);

    // Set our cooperative level
    g_lpdd7->SetCooperativeLevel( hWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN );
  
    // Set the display mode
    g_lpdd7->SetDisplayMode( SCREEN_WIDTH, SCREEN_HEIGHT, COLOR_DEPTH, 0, 0 );

    // Create the primary surface with one back buffer.
	ZeroMemory( &ddsd, sizeof(ddsd) );
    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | 
                          DDSCAPS_COMPLEX |DDSCAPS_VIDEOMEMORY;
    ddsd.dwBackBufferCount = 1;
	ddsd.dwWidth  = SCREEN_WIDTH;
	ddsd.dwHeight = SCREEN_HEIGHT;
  
    g_lpdd7->CreateSurface( &ddsd, &g_lpddsPrimary, NULL );

    // Get a pointer to the back buffer.
	ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
	ddscaps.dwCaps2 = 0;
	ddscaps.dwCaps3 = 0;
	ddscaps.dwCaps4 = 0;
	g_lpddsPrimary->GetAttachedSurface( &ddscaps, &g_lpddsBack );

	InitializeStars();
	g_bScrollLeft = true;

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: Draw all 3D Objects to the screen...
//-----------------------------------------------------------------------------
HRESULT render( void )
{
	DDBLTFX  ddbltfx;
    double   dTickCount;
	DDSURFACEDESC2 ddsd;
	int linearPitch = 0;
	int nColorDepth = 0;
	int x = 0;
	int y = 0;

	static double dLastTickCount; // Last frame time.
	static int    nFrame;         // Current sprite frame.
	static int    nPosition = 0;  // Current position of sprite.

	dTickCount = GetTickCount();

    if( ( dTickCount - dLastTickCount ) <= FRAME_DELAY )
		return S_OK;

    dLastTickCount = dTickCount;

    // Clear the back buffer with black using the hardware blitter.
    ddbltfx.dwSize = sizeof( ddbltfx );
    ddbltfx.dwFillColor = 0;
    g_lpddsBack->Blt( NULL, NULL, NULL,DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx );

	// Move the stars
	ScrollStars();

	memset( &ddsd, 0, sizeof(ddsd) );
	ddsd.dwSize = sizeof(ddsd);

	if(FAILED(g_lpddsBack->Lock(NULL, &ddsd,DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT,NULL)))
		return S_FALSE;

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
			surfaceBuffer[x+y*linearPitch] = 255;
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
			surfaceBuffer[x+y*linearPitch] = 255;
		} 
	}
	else if( nColorDepth == 32 )
	{
		// Use a UINT because 4 bytes per pixel in 32 bit color
		UINT *surfaceBuffer = static_cast<UINT*>(ddsd.lpSurface);
		// NAnd half the linear pitch twice because each pixel is now worth 4 bytes
		linearPitch = (linearPitch>>2); 

		for( int i = 0; i < TOTAL_STARS; i++ )
		{
			int x = stars[i].x;
			int y = stars[i].y;

			// Plot the pixel
			surfaceBuffer[x+y*linearPitch] = 255;
		} 
	}

	// Now unlock the primary surface
	if(FAILED(g_lpddsBack->Unlock(NULL)))
	   return S_FALSE;

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
	SAFE_RELEASE( g_lpddsBack );
	SAFE_RELEASE( g_lpddsPrimary );
	SAFE_RELEASE( g_lpdd7 );
}