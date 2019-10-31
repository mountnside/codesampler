//-----------------------------------------------------------------------------
//           Name: w32_perfmon_leak_test.cpp
//         Author: Kevin Harris
//  Last Modified: 09/25/04
//    Description: This application demonstrates how to track a memory leak
//                 using PerfMon and the GlobalAlloc and GlobalFree functions
//                 of the Win32 SDK. Of course, most programmers don't use
//                 GlobalAlloc to allocate memory but it's possible to setup
//                 a conditional build define to switch between your preferred
//                 style of allocation and GlobalAlloc.
//
//  1. Once this test application comes up, launch PerfMon by going to the
//     Windows "Start" button and select "Run" from the main menu. When the
//     "Run" dialog pops up, type "perfmon" in the edit box and click "OK".
//
//  2. Once Perfmon is up and running, right click anywhere in its empty 
//     line-graphing area and select "Add Counters..." from the context menu.
//
//  3. Pull down the "Performance Object:" list box and select "Process".
//
//  4. Find and select "Private Bytes" from the counter list box.
//
//  5. Find and select "w32_perfmon_leak_test" from the instance list box.
//
//  6. Click the "Add" button and close the "Add Counters" dialog box.
//
//  7. Use the "Test" menu of the application to start the leak by selecting 
//    "Leak Memory".
//
//  8. Note the leak in PerfMon as the "Private Bytes" counter continues to
//     climb with each memory allocation.
//
//  9. Use the "Test" menu of the application to stop and clean up the 
//     leak by selecting "Free Memory".
//
//-----------------------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include "resource.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
struct _MEM_BLOCK
{
   struct _MEM_BLOCK *pNext;
};

typedef _MEM_BLOCK  MEM_BLOCK;
typedef _MEM_BLOCK* PTR_MEM_BLOCK;

const int ALLOCATION_SIZE = 4096 * 10;
const int TIME_INTERVAL   = 100;
const int LEAK_TIMER      = 13;

unsigned long g_timerID = 0;
bool g_bTimerRunning = false;
MEM_BLOCK g_memLeakListHead = {NULL};

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void FreeAllocatedMemory(void);

//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: The application's entry point.
//-----------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR     lpCmdLine,
                    int       nCmdShow )
{
	WNDCLASSEX winClass; 
	HWND       hwnd;
	MSG        uMsg;

    memset(&uMsg,0,sizeof(uMsg));

	winClass.lpszClassName = "MY_WINDOWS_CLASS";
	winClass.cbSize        = sizeof(WNDCLASSEX);
	winClass.style         = CS_HREDRAW | CS_VREDRAW;
	winClass.lpfnWndProc   = WindowProc;
	winClass.hInstance     = hInstance;
	winClass.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
	winClass.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);
	winClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	winClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	winClass.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU);
	winClass.cbClsExtra    = 0;
	winClass.cbWndExtra    = 0;

	if( RegisterClassEx( &winClass) == 0 )
		return E_FAIL;

	hwnd = CreateWindowEx( NULL, "MY_WINDOWS_CLASS", 
		                   "Tracking Memory Leaks With PerfMon",
						   WS_OVERLAPPEDWINDOW | WS_VISIBLE,
					       0, 0, 400, 200, NULL, NULL, hInstance, NULL );

	if( hwnd == NULL )
		return E_FAIL;

	ShowWindow( hwnd, nCmdShow );
    UpdateWindow( hwnd );

	while( uMsg.message != WM_QUIT )
	{
        GetMessage( &uMsg, NULL, 0, 0 );
		TranslateMessage( &uMsg );
		DispatchMessage( &uMsg );
	}

	UnregisterClass( "MY_WINDOWS_CLASS", winClass.hInstance );

	return uMsg.wParam;
}

//-----------------------------------------------------------------------------
// Name: WindowProc()
// Desc: 
//-----------------------------------------------------------------------------
LRESULT CALLBACK WindowProc( HWND   hwnd,
							 UINT   uMsg,
							 WPARAM wParam,
							 LPARAM lParam )
{
	switch( uMsg )
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

		case WM_COMMAND:
		{
            switch( LOWORD(wParam) )
            {
                case ID_TEST_LEAKMEMORY:
				{
					if( g_bTimerRunning == false )
					{
						g_timerID = SetTimer( hwnd, LEAK_TIMER, TIME_INTERVAL, NULL );

						if( g_timerID != 0 )
							g_bTimerRunning = true;
						else
							MessageBeep( MB_ICONEXCLAMATION );
					}
				}
				break;

                case ID_TEST_FREEMEMORY:
				{
					KillTimer( hwnd, LEAK_TIMER );
					g_timerID = 0;
					g_bTimerRunning = false;
					FreeAllocatedMemory();
				}
				break;

            }
            break;
		}

		break;

		case WM_TIMER:
		{
			PTR_MEM_BLOCK pMemBlock;
			PTR_MEM_BLOCK pNewMemBlock;

			pNewMemBlock = (PTR_MEM_BLOCK)GlobalAlloc( GPTR, ALLOCATION_SIZE );

			if( pNewMemBlock != NULL )
			{
				// Save this pointer
				pNewMemBlock->pNext = NULL;

				if( g_memLeakListHead.pNext == NULL )
				{
					// This is the first entry
					g_memLeakListHead.pNext = pNewMemBlock;
				}
				else
				{
					// Go to the end of the list
					pMemBlock = g_memLeakListHead.pNext;

					while( pMemBlock->pNext != NULL ) 
						pMemBlock = pMemBlock->pNext;

					pMemBlock->pNext = pNewMemBlock;
				}
			}
		}
		break;

		case WM_CLOSE:
		{
			DestroyWindow( hwnd );
		}
		break;

		case WM_DESTROY: 
		{
			FreeAllocatedMemory();
			PostQuitMessage(0);
		} 
		break;

		default:
		{
			return DefWindowProc( hwnd, uMsg, wParam, lParam );
		}
		break;
	}

	return 0;
}

//-----------------------------------------------------------------------------
// Name: FreeAllocatedMemory()
// Desc: 
//-----------------------------------------------------------------------------
void FreeAllocatedMemory( void )
{
	PTR_MEM_BLOCK pNextMemBlock;
	PTR_MEM_BLOCK pMemBlock;

	pMemBlock = g_memLeakListHead.pNext;

	while( pMemBlock != NULL )
	{
		pNextMemBlock = pMemBlock->pNext;
		GlobalFree( pMemBlock );
		pMemBlock = pNextMemBlock;
	}

	g_memLeakListHead.pNext = NULL;
}