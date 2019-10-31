//-----------------------------------------------------------------------------
//           Name: dx8_joystick.cpp
//         Author: Kevin Harris
//  Last Modified: 03/03/04
//    Description: Joystick sample.
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
//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
LPDIRECTDRAW7        g_lpdd7;        // DirectDraw object
LPDIRECTDRAWSURFACE7 g_lpddsPrimary; // DirectDraw primary surface
LPDIRECTDRAWSURFACE7 g_lpddsBack;    // DirectDraw back buffer surface
LPDIRECTDRAWSURFACE7 g_lpddsDonut;   // DirectDraw off-screen surface for the donut bitmap.
LPDIRECTINPUT8       g_lpdi8;        // DirectInput object
LPDIRECTINPUTDEVICE8 g_lpdiJoystick; // DirectInput device

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

BOOL CALLBACK EnumJoysticksCallback( const DIDEVICEINSTANCE *pdidInstance, 
									 void *pContext );

BOOL CALLBACK EnumAxesCallback( const DIDEVICEOBJECTINSTANCE *pdidoi, 
							    void *pContext );

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

	if( !RegisterClassEx(&winClass) )
		return(0);

	hWnd = CreateWindowEx( NULL,"MY_WINDOWS_CLASS",
						   "Direct3D (DX8) - Joystick Sample",
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

	/////////////////////////////////////////////////////////////////////////////////////////

	// Create a DInput object
    DirectInput8Create( GetModuleHandle(NULL), DIRECTINPUT_VERSION, 
                        IID_IDirectInput8, (VOID**)&g_lpdi8, NULL );
    
    // Look for a simple joystick we can use for this sample.
    g_lpdi8->EnumDevices( DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback, 
		                NULL, DIEDFL_ATTACHEDONLY );
 
	g_lpdiJoystick->SetDataFormat( &c_dfDIJoystick );

    // Set the cooperative level to let DInput know how this device should
    // interact with the system and with other DInput applications.
    g_lpdiJoystick->SetCooperativeLevel(  hWnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND );

    // Enumerate the axes of the joystick and set the range of each axis found. 
	g_lpdiJoystick->EnumObjects( EnumAxesCallback, (VOID*)hWnd, DIDFT_AXIS );

	g_lpdiJoystick->Acquire();

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


	// We'll now poll the joystick for its current state and 
	// then retrieve the state data by passing in a DIJOYSTATE 
	// data structure into the GetDeviceState() function.

	// Once we have the data back, we can use the 
	// WriteToSurface() helper function to display each of
	// fields stored in the returned DIJOYSTATE structure.


	DIJOYSTATE  js; // DInput joystick state data-structure
	
	g_lpdiJoystick->Poll();
	g_lpdiJoystick->GetDeviceState( sizeof(DIJOYSTATE), &js );

	// We'll start off by examining each axis of the analog stick!
	WriteToSurface( "X Axis = ", js.lX, 0,  0, g_lpddsBack );
	WriteToSurface( "Y Axis = ", js.lY, 0, 20, g_lpddsBack );
	WriteToSurface( "Z Axis = ", js.lZ, 0, 40, g_lpddsBack );

	// Now let's check to see if we have any rotation controls...
	WriteToSurface( "X Axis Rotation = ", js.lRx, 0,  60, g_lpddsBack );
	WriteToSurface( "X Axis Rotation = ", js.lRy, 0,  80, g_lpddsBack );
	WriteToSurface( "Z Axis Rotation = ", js.lRz, 0, 100, g_lpddsBack );

	// Do we have any hat-switches or Point-of-View buttons?
	WriteToSurface( "POV 1 = ", js.rgdwPOV[0], 0, 120, g_lpddsBack );
	WriteToSurface( "POV 2 = ", js.rgdwPOV[1], 0, 140, g_lpddsBack );
	WriteToSurface( "POV 3 = ", js.rgdwPOV[2], 0, 160, g_lpddsBack );
	WriteToSurface( "POV 4 = ", js.rgdwPOV[3], 0, 180, g_lpddsBack );

	// Don't forget slider controls...
	WriteToSurface( "Slider 1 = ", js.rglSlider[0], 0, 200, g_lpddsBack );
	WriteToSurface( "Slider 2 = ", js.rglSlider[1], 0, 220, g_lpddsBack );

	// And last, but not least, we'll check the joystick's buttons...
	WriteToSurface( "Button 1 = ", js.rgbButtons[0], 0, 240, g_lpddsBack );
	WriteToSurface( "Button 2 = ", js.rgbButtons[1], 0, 260, g_lpddsBack );
	WriteToSurface( "Button 3 = ", js.rgbButtons[2], 0, 280, g_lpddsBack );
	WriteToSurface( "Button 4 = ", js.rgbButtons[3], 0, 300, g_lpddsBack );
	WriteToSurface( "Button 5 = ", js.rgbButtons[4], 0, 320, g_lpddsBack );
	WriteToSurface( "Button 6 = ", js.rgbButtons[5], 0, 340, g_lpddsBack );
	WriteToSurface( "Button 7 = ", js.rgbButtons[6], 0, 360, g_lpddsBack );
	WriteToSurface( "Button 8 = ", js.rgbButtons[7], 0, 380, g_lpddsBack );

	// There can acually be up to 32 buttons, 
	// but I'll spare you the details!

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
	if( g_lpdiJoystick != NULL ) 
	{
		g_lpdiJoystick->Unacquire();
		SAFE_RELEASE( g_lpdiJoystick );
	}

	// Release any DirectInput objects.
	SAFE_RELEASE( g_lpdi8 );
}

BOOL CALLBACK EnumJoysticksCallback( const DIDEVICEINSTANCE *pdidInstance, 
									 void *pContext )
{
	// NOTE: To keep things simple, this call back function 
	// simply grabs the first joystick found attached to 
	// the computer and kills the enumeration by returning
	// DIENUM_STOP. If you wanted to find every 
	// joystick the user had attached, you would store the 
	// enumeration results and continue the search by
	// returning DIENUM_CONTINUE instead. This would allow
	// you to offer the user his or her selection of available 
	// joysticks.

    // Obtain an interface to the enumerated joystick.
    g_lpdi8->CreateDevice(pdidInstance->guidInstance, &g_lpdiJoystick, NULL);
    
    return DIENUM_STOP;

	// If your computer has more than one joystick and this 
	// call back function keeps retrieving the wrong one, 
	// replace the code in EnumJoysticksCallback() with the 
	// code below to get the second joystick instead!
	/*
	static int count = 0;
	++count;
	if( count == 2 )
	{
	    // Obtain an interface to the enumerated joystick.
		lpdi8->CreateDevice(pdidInstance->guidInstance, &lpdiJoystick, NULL);
		return DIENUM_STOP;
	}
    return DIENUM_CONTINUE;
	*/
}

BOOL CALLBACK EnumAxesCallback( const DIDEVICEOBJECTINSTANCE *pdidoi, 
							    void *pContext )
{
	// For each axis enumrated, this function will set 
	// the minimum and maximum range values for it.

    DIPROPRANGE diprg;

    diprg.diph.dwSize       = sizeof(DIPROPRANGE);
    diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    diprg.diph.dwHow        = DIPH_BYOFFSET;
    diprg.diph.dwObj        = pdidoi->dwOfs; // Specify the enumerated axis
    diprg.lMin              = -100;
    diprg.lMax              = +100;
    
    // Set the range for the axis
    if( FAILED( g_lpdiJoystick->SetProperty( DIPROP_RANGE, &diprg.diph ) ) )
        return DIENUM_STOP;

    return DIENUM_CONTINUE;
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