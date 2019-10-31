//-----------------------------------------------------------------------------
//           Name: dx8_mouse.cpp
//         Author: Kevin Harris
//  Last Modified: 03/03/04
//    Description: Mouse Sample.
//
//           Note: DirectX 8.0 still uses 7.0 interfaces for DirectDraw!
//-----------------------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#define INITGUID

#include <windows.h>
#include <ddraw.h>
#include <dinput.h>

//-----------------------------------------------------------------------------
// MACROS
//-----------------------------------------------------------------------------
#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }
#define KEYDOWN(name,key) (name[key] & 0x80) 

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
LPDIRECTDRAW7        g_lpdd7;        // DirectDraw object
LPDIRECTDRAWSURFACE7 g_lpddsPrimary; // DirectDraw primary surface
LPDIRECTDRAWSURFACE7 g_lpddsBack;    // DirectDraw back buffer surface
LPDIRECTDRAWSURFACE7 g_lpddsDonut;   // DirectDraw off-screen surface for the donut bitmap.
LPDIRECTINPUT8       g_lpdi8;        // DirectInput object
LPDIRECTINPUTDEVICE8 g_lpdiMouse;    // DirectInput device

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
void WriteToSurface( char *label, int value, int x, int y, 
					 LPDIRECTDRAWSURFACE7 lpdds, bool valuePresent = 1 );


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

	if(!RegisterClassEx( &winClass) )
		return(0);

	hWnd = CreateWindowEx( NULL,"MY_WINDOWS_CLASS",
						   "Direct3D (DX8) - Mouse Sample",
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
	DDSURFACEDESC2 ddsd;    // DirectDraw surface description structure
	DDSCAPS2       ddscaps; // DirectDraw surface capabilities structure.

	DirectDrawCreateEx( NULL, (LPVOID*)&g_lpdd7, IID_IDirectDraw7, NULL);
    g_lpdd7->SetCooperativeLevel( hWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN );
    g_lpdd7->SetDisplayMode( SCREEN_WIDTH, SCREEN_HEIGHT, COLOR_DEPTH, 0, 0 );

	ZeroMemory( &ddsd, sizeof(ddsd) );
    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | 
                          DDSCAPS_COMPLEX |DDSCAPS_VIDEOMEMORY;
    ddsd.dwBackBufferCount = 1;
	ddsd.dwWidth  = SCREEN_WIDTH;
	ddsd.dwHeight = SCREEN_HEIGHT;

    g_lpdd7->CreateSurface( &ddsd, &g_lpddsPrimary, NULL );

	ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
	ddscaps.dwCaps2 = 0;
	ddscaps.dwCaps3 = 0;
	ddscaps.dwCaps4 = 0;
	g_lpddsPrimary->GetAttachedSurface( &ddscaps, &g_lpddsBack );

	/////////////////////////////////////////////////////////////////////////////////////////////

	// Create a DInput object
    DirectInput8Create( GetModuleHandle(NULL), DIRECTINPUT_VERSION, 
                        IID_IDirectInput8, (VOID**)&g_lpdi8, NULL );
    
    // Obtain an interface to the system keyboard device.
    g_lpdi8->CreateDevice( GUID_SysMouse, &g_lpdiMouse, NULL );


	g_lpdiMouse->SetDataFormat( &c_dfDIMouse );

	// Uncomment the following to see another way of reading mouse data
	/*
	DIPROPHEADER dih;
	dih.dwHeaderSize = sizeof(DIPROPHEADER);
	dih.dwSize       = sizeof(DIPROPDWORD);
	dih.dwHow        = DIPH_DEVICE;
	dih.dwObj        = 0;

	DIPROPDWORD dip;
	dip.dwData = DIPROPAXISMODE_ABS;
	dip.diph = dih;

	g_lpdiMouse->SetProperty( DIPROP_AXISMODE, &dip.diph );
	//*/

	g_lpdiMouse->SetCooperativeLevel( hWnd, 
		                              DISCL_NONEXCLUSIVE | DISCL_FOREGROUND );

	g_lpdiMouse->Acquire();
	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: Draw all 3D Objects to the screen...
//-----------------------------------------------------------------------------
HRESULT render( void )
{
	DDBLTFX ddbltfx;
    double  dTickCount;

	static double dLastTickCount; // Last frame time.
	static int    nFrame;         // Current sprite frame.
	static int    nPosition = 0;  // Current position of sprite.

	dTickCount = GetTickCount();

    if( ( dTickCount - dLastTickCount ) <= FRAME_DELAY )
		return S_OK;

    dLastTickCount = dTickCount;

    // Clear out the back buffer
    ddbltfx.dwSize = sizeof( ddbltfx );
    ddbltfx.dwFillColor = 0;
    g_lpddsBack->Blt( NULL, NULL, NULL,DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx );

	DIMOUSESTATE2 ms; // DirectInput mouse state structure

	// Get the input's device state, and put the state in ms
	g_lpdiMouse->GetDeviceState( sizeof(DIMOUSESTATE), &ms );

	WriteToSurface( "X Position = ", ms.lX, 0,  0, g_lpddsBack ); // X postion
	WriteToSurface( "Y Position = ", ms.lY, 0, 20, g_lpddsBack ); // Y postion
	WriteToSurface( "Z Position = ", ms.lZ, 0, 40, g_lpddsBack ); // Mouse wheel position

	WriteToSurface( "Mouse Button 1 = ", ms.rgbButtons[0], 0,  60, g_lpddsBack );
	WriteToSurface( "Mouse Button 2 = ", ms.rgbButtons[1], 0,  80, g_lpddsBack );
	WriteToSurface( "Mouse Button 3 = ", ms.rgbButtons[2], 0, 100, g_lpddsBack );
	WriteToSurface( "Mouse Button 4 = ", ms.rgbButtons[3], 0, 120, g_lpddsBack );

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

	// Unacquire and release any DirectInputDevice objects.
	if( g_lpdiMouse != NULL ) 
	{
		g_lpdiMouse->Unacquire();
		SAFE_RELEASE( g_lpdiMouse );
	}

	// Release any DirectInput objects.
	SAFE_RELEASE( g_lpdi8 );
}


//-----------------------------------------------------------------------------
// Name: WriteToSurface()
// Desc: This function assists in debugging by writing an informative label 
//       and a variable value at the specifeid X/Y postion on the DirectDraw 
//       surface passed in. If you wish to display text without a value, pass 
//       in "false" for the last arguement.
//-----------------------------------------------------------------------------
void WriteToSurface( char *label, int value, int x, int y, 
					 LPDIRECTDRAWSURFACE7 lpdds, bool valuePresent )
{
	HDC  wdc;
	char valueBuffer[10] = { NULL };
	char textBuffer[200] = { NULL };

	_itoa( value, valueBuffer, 10 );

	strcat( textBuffer, label );

	if( valuePresent )
		strcat( textBuffer, valueBuffer );

	if( FAILED( lpdds->GetDC(&wdc) ) )
	   return;

	SetTextColor( wdc, RGB(255,255,255) );
	SetBkMode( wdc, TRANSPARENT );
	TextOut( wdc, x, y, textBuffer,strlen(textBuffer) );

	lpdds->ReleaseDC(wdc);
}