//-----------------------------------------------------------------------------
//           Name: dx8_bitmap.cpp
//         Author: Kevin Harris
//  Last Modified: 03/03/04
//    Description: Sample loads a bitmap into a DirectDraw surface for viewing.
//
//           Note: DirectX 8.1 still uses 7.0 COM interfaces for DirectDraw!
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
const int SCREEN_WIDTH  = 800;
const int SCREEN_HEIGHT = 600;
const int COLOR_DEPTH   = 16;

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
LPDIRECTDRAW7        g_lpdd7;        // DirectDraw object
LPDIRECTDRAWSURFACE7 g_lpddsPrimary; // DirectDraw surface
DDSURFACEDESC2       g_ddsd;         // DirectDraw surface description structure

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
LRESULT CALLBACK WindowProc( HWND hWnd, 
							 UINT msg, 
							 WPARAM wParam, 
							 LPARAM lParam )
{
	switch( msg )
	{
        case WM_SETCURSOR:
		{
            SetCursor( NULL );
            return 1;
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
			shutDown();
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
					HINSTANCE hPrevinstance,
					LPSTR lpCmdLine,
					int nCmdShow )
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
						   "Direct3D (DX8) - Bitmap Loading",
                           WS_POPUP | WS_VISIBLE,
					 	   0, 0, 640, 480, NULL, NULL, hInstance, NULL);

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
	// Create the main DirectDraw object
	DirectDrawCreateEx( NULL, (LPVOID*)&g_lpdd7, IID_IDirectDraw7, NULL);

    // Set our cooperative level
    g_lpdd7->SetCooperativeLevel( hWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN );
  
    // Set the display mode
    g_lpdd7->SetDisplayMode( SCREEN_WIDTH, SCREEN_HEIGHT, COLOR_DEPTH, 0, 0 );

    // Set up the surface descriptor for creating a primary surface
	ZeroMemory( &g_ddsd, sizeof(g_ddsd) );
    g_ddsd.dwSize = sizeof( DDSURFACEDESC2 );
    g_ddsd.dwFlags = DDSD_CAPS; // Tell DirectDraw which flags are valid
    g_ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE; // Create a primary surface

	// Create the surface
    g_lpdd7->CreateSurface( &g_ddsd, &g_lpddsPrimary, NULL );

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: Draw all 3D Objects to the screen...
//-----------------------------------------------------------------------------
HRESULT render( void )
{
    HBITMAP hBMP;
    HRESULT hr;
	HDC     hDCImage;
    HDC     hDC;
    BITMAP         bmp;
    DDSURFACEDESC2 ddsd;

    if( g_lpddsPrimary == NULL )
        return E_INVALIDARG;

    hBMP = (HBITMAP) LoadImage( NULL, "ghost.bmp", IMAGE_BITMAP, 
                                640, 480, LR_LOADFROMFILE | LR_CREATEDIBSECTION );
    if( hBMP == NULL )
        return E_FAIL;

    if( hBMP == NULL || g_lpddsPrimary == NULL )
        return E_INVALIDARG;

    // Make sure this surface is restored.
    if( FAILED( hr = g_lpddsPrimary->Restore() ) )
        return hr;

    // Get the surface.description
    ddsd.dwSize  = sizeof(ddsd);
    g_lpddsPrimary->GetSurfaceDesc( &ddsd );

    if( ddsd.ddpfPixelFormat.dwFlags == DDPF_FOURCC )
        return E_NOTIMPL;

    // Select bitmap into a memoryDC so we can use it.
    hDCImage = CreateCompatibleDC( NULL );
    if( NULL == hDCImage )
        return E_FAIL;

    SelectObject( hDCImage, hBMP );

    // Get size of the bitmap
    GetObject( hBMP, sizeof(bmp), &bmp );

    // Stretch the bitmap to cover this surface
    if( FAILED( hr = g_lpddsPrimary->GetDC( &hDC ) ) )
        return hr;

    StretchBlt( hDC, 0, 0, 
                ddsd.dwWidth, ddsd.dwHeight, 
                hDCImage, 0, 0,
                bmp.bmWidth,  bmp.bmHeight, SRCCOPY );

    if( FAILED( hr = g_lpddsPrimary->ReleaseDC( hDC ) ) )
        return hr;

    DeleteDC( hDCImage );
    DeleteObject( hBMP );

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: Release and cleanup everything.
//-----------------------------------------------------------------------------
void shutDown( void )
{
    SAFE_RELEASE( g_lpddsPrimary );
    SAFE_RELEASE( g_lpdd7 );
}
