//-----------------------------------------------------------------------------
//           Name: ogl_fullscreen.cpp
//         Author: Kevin Harris
//  Last Modified: 02/01/05
//    Description: This sample demonstrates how to probe the hardware for 
//                 specific Display Settings and Pixel Formats suitable for a 
//                 full-screen application with OpenGL.
//-----------------------------------------------------------------------------

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <GL/gl.h>
#include "resource.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND    g_hWnd = NULL;
HDC	    g_hDC  = NULL;
HGLRC   g_hRC  = NULL;
DEVMODE g_oldDevMode;

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void init(void);
void shutDown(void);
void render(void);

//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: The application's entry point
//-----------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR     lpCmdLine,
                    int       nCmdShow )
{
    WNDCLASSEX winClass; 
	MSG        uMsg;

    memset(&uMsg,0,sizeof(uMsg));
    
	winClass.lpszClassName = "MY_WINDOWS_CLASS";
	winClass.cbSize        = sizeof(WNDCLASSEX);
	winClass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	winClass.lpfnWndProc   = WindowProc;
	winClass.hInstance     = hInstance;
    winClass.hIcon	       = LoadIcon(hInstance, (LPCTSTR)IDI_OPENGL_ICON);
    winClass.hIconSm	   = LoadIcon(hInstance, (LPCTSTR)IDI_OPENGL_ICON);
	winClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	winClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	winClass.lpszMenuName  = NULL;
	winClass.cbClsExtra    = 0;
	winClass.cbWndExtra    = 0;

	if( !RegisterClassEx(&winClass) )
		return E_FAIL;

	g_hWnd = CreateWindowEx( NULL, "MY_WINDOWS_CLASS", 
                             "OpenGL - Full Screen Application",
						     WS_POPUP | WS_VISIBLE,
					         0, 0, 640, 480, NULL, NULL, hInstance, NULL );

	if( g_hWnd == NULL )
		return E_FAIL;

    ShowWindow( g_hWnd, nCmdShow );
    UpdateWindow( g_hWnd );

	init();

	while( uMsg.message != WM_QUIT )
	{
		if( PeekMessage( &uMsg, NULL, 0, 0, PM_REMOVE ) )
		{
			TranslateMessage( &uMsg );
			DispatchMessage( &uMsg );
		}
        else
			render();
	}

	shutDown();

    UnregisterClass( "MY_WINDOWS_CLASS", winClass.hInstance );

	return uMsg.wParam;
}

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

		case WM_CLOSE:
		{
			PostQuitMessage(0);	
		}

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
// Name: init()
// Desc: 
//-----------------------------------------------------------------------------
void init( void )
{
	// Cache the current display mode so we can switch back when done.
	EnumDisplaySettings( NULL, ENUM_CURRENT_SETTINGS, &g_oldDevMode );

	//
	// Enumerate Device modes...
	//

	int nMode = 0;
	DEVMODE devMode;
	bool bDesiredDevModeFound = false;

	while( EnumDisplaySettings( NULL, nMode++, &devMode ) )
	{
		// Does this device mode support a 640 x 480 setting?
		if( devMode.dmPelsWidth  != 640 || devMode.dmPelsHeight != 480)
			continue;

		// Does this device mode support 32-bit color?
		if( devMode.dmBitsPerPel != 32 )
			continue;

		// Does this device mode support a refresh rate of 75 MHz?
		if( devMode.dmDisplayFrequency != 75 )
			continue;

		// We found a match, but can it be set without rebooting?
		if( ChangeDisplaySettings( &devMode, CDS_TEST ) == DISP_CHANGE_SUCCESSFUL )
		{
			bDesiredDevModeFound = true;
			break;
		}
	}

	if( bDesiredDevModeFound == false )
	{
		// TO DO: Handle lack of support for desired mode...
		return;
	}

	//
	// Verify hardware support by enumerating pixel formats...
	//

	g_hDC = GetDC( g_hWnd );

	PIXELFORMATDESCRIPTOR pfd;

	int nTotalFormats = DescribePixelFormat( g_hDC, 1, sizeof(pfd), NULL );

	int nPixelFormat;

	for( nPixelFormat = 1; nPixelFormat <= nTotalFormats; ++nPixelFormat )
	{
		if( DescribePixelFormat( g_hDC, nPixelFormat, sizeof(pfd), &pfd ) == 0 )
		{
			DWORD dwErrorCode = GetLastError();
			// TO DO: Respond to failure of DescribePixelFormat
			return;
		}

		if( !(pfd.dwFlags & PFD_SUPPORT_OPENGL) )
			continue;

		if( !(pfd.dwFlags & PFD_DOUBLEBUFFER) )
			continue;

		if( pfd.iPixelType != PFD_TYPE_RGBA )
			continue;
		
		if( pfd.cColorBits != 32 )
			continue;

		if( pfd.cDepthBits != 16 )
			continue;

		// If we made it this far, we found a match!
		break;
	}

	//
	// Everything checks out - create the rendering context and 
	// switch the display settings with our new device mode...
	//

	if( SetPixelFormat( g_hDC, nPixelFormat, &pfd) == FALSE )
	{
		DWORD dwErrorCode = GetLastError();
		// TO DO: Respond to failure of SetPixelFormat
		return;
	}

	g_hRC = wglCreateContext( g_hDC );
	wglMakeCurrent( g_hDC, g_hRC );
	
	glClearColor( 0.0f, 1.0f, 0.0f, 1.0f );

	if( ChangeDisplaySettings( &devMode, CDS_FULLSCREEN ) != DISP_CHANGE_SUCCESSFUL )
	{
		// TO DO: Respond to failure of ChangeDisplaySettings
		return;
	}
}

//-----------------------------------------------------------------------------
// Name: shutDown()
// Desc: 
//-----------------------------------------------------------------------------
void shutDown( void )	
{
	// Restore original device mode...
	ChangeDisplaySettings( &g_oldDevMode, 0 );

	if( g_hRC != NULL )
	{
		wglMakeCurrent( NULL, NULL );
		wglDeleteContext( g_hRC );
		g_hRC = NULL;
	}

	if( g_hDC != NULL )
	{
		ReleaseDC( g_hWnd, g_hDC );
		g_hDC = NULL;
	}
}

//-----------------------------------------------------------------------------
// Name: render()
// Desc: 
//-----------------------------------------------------------------------------
void render( void )	
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	// Render geometry here...

	SwapBuffers( g_hDC );
}
