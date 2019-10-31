//-----------------------------------------------------------------------------
//           Name: dx8_pixels.cpp
//         Author: Kevin Harris
//  Last Modified: 03/03/04
//    Description: This sample shows how to lock a DirectDraw surface so you 
//                 can write directly to it.
//
//           Note: DirectX 8.0 still uses 7.0 interfaces for DirectDraw!
//                 Click and move the mouse to plot white pixels on the 
//                 surface.
//-----------------------------------------------------------------------------
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <windowsx.h>
#include <ddraw.h>

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
bool                 g_bMouseButtonDown = false;
int                  g_xPos = 0;
int                  g_yPos = 0;

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
			}
		}
        break;

		case WM_MOUSEMOVE: 
		{
			// The mouse moved, get its current postion and store it.
			g_xPos = GET_X_LPARAM(lParam);
			g_yPos = GET_Y_LPARAM(lParam);
			return(0);
		} 
		break;
		
		case WM_LBUTTONDOWN: 
		{
			// The left mouse button has been pressed!
			g_bMouseButtonDown = true;
			return(0);
		} 
		break;

		case WM_LBUTTONUP: 
		{
			// The left mouse button has been released!
			g_bMouseButtonDown = false;
			return(0);
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
						   "Direct3D (DX8) - Pixel Plotting",
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
	DDSURFACEDESC2 ddsd; // DirectDraw surface description structure

	// Create the main DirectDraw object
	DirectDrawCreateEx( NULL, (LPVOID*)&g_lpdd7, IID_IDirectDraw7, NULL);

    // Set our cooperative level
    g_lpdd7->SetCooperativeLevel( hWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN );

    // Set the display mode
    g_lpdd7->SetDisplayMode( SCREEN_WIDTH, SCREEN_HEIGHT, COLOR_DEPTH, 0, 0 );

    // Fill in the surface descriptor before creating the surface.
	ZeroMemory( &ddsd, sizeof(ddsd) );
    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    ddsd.dwFlags = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	// Create a primary surface...
    g_lpdd7->CreateSurface( &ddsd, &g_lpddsPrimary, NULL );

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: Draw all 3D Objects to the screen...
//-----------------------------------------------------------------------------
HRESULT render( void )
{
	if( g_bMouseButtonDown == true ) // If the mouse button is down, plot a pixel!
	{
		DDSURFACEDESC2 ddsd; // DirectDraw surface description structure
		int linearPitch = 0;
		int nColorDepth = 0;
		ZeroMemory( &ddsd, sizeof(ddsd) );
		ddsd.dwSize = sizeof(ddsd);

		// Lock the primary surface so we can access it at the pixel level.
		g_lpddsPrimary->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
			
		// Get the color depth of the back buffer
		nColorDepth = ddsd.ddpfPixelFormat.dwRGBBitCount;

		// Get number of bytes for the linear pitch
		linearPitch = (int)ddsd.lPitch; 

		if( nColorDepth == 8 )
		{
			// Use a BYTE because 1 byte per pixel in 8 bit color
			BYTE *surfaceBuffer = static_cast<BYTE*>(ddsd.lpSurface);

			surfaceBuffer[g_xPos+g_yPos*linearPitch] = 255;
		}
		else if( nColorDepth == 16 )
		{
			// Use a USHORT because 2 bytes per pixel in 16 bit color
			USHORT *surfaceBuffer = static_cast<USHORT*>(ddsd.lpSurface);
			// And half the linear pitch because each pixel is now worth 2 bytes
			linearPitch = (linearPitch>>1);   

			surfaceBuffer[g_xPos+g_yPos*linearPitch] = 255;
		}
		else if( nColorDepth == 32 )
		{
			// Use a UINT because 4 bytes per pixel in 32 bit color
			UINT *surfaceBuffer = static_cast<UINT*>(ddsd.lpSurface);
			// NAnd half the linear pitch twice because each pixel is now worth 4 bytes
			linearPitch = (linearPitch>>2); 

			surfaceBuffer[g_xPos+g_yPos*linearPitch] = 255;
		}

		// Now unlock the primary surface.
		g_lpddsPrimary->Unlock(NULL);
	}

	return S_OK;
}


//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: Release and cleanup everything.
//-----------------------------------------------------------------------------
void shutDown()
{
	SAFE_RELEASE( g_lpddsPrimary );
	SAFE_RELEASE( g_lpdd7 );
}
