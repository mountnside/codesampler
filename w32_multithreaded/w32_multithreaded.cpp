//-----------------------------------------------------------------------------
//           Name: w32_multithreaded.cpp
//         Author: Kevin Harris
//  Last Modified: 03/06/05
//    Description: Sample of spawning multiple threads under Win32.
//-----------------------------------------------------------------------------

#include <windows.h>

#include <iostream>
using namespace std;

// cout, as defined by <iostream>, is not thread-safe, so we need to
// use a critical section to make the cout call atomic.
CRITICAL_SECTION g_criticalSection;

//-----------------------------------------------------------------------------
// Note how functions must have a certain signature or prototype to be used 
// properly as a thread-friendly function under Win32:
//
// DWORD WINAPI <funcName>(LPVOID);
//
//-----------------------------------------------------------------------------
DWORD WINAPI genericFunc(LPVOID);
DWORD WINAPI printString(LPVOID);
DWORD WINAPI printNumber(LPVOID);

HANDLE g_handles[3]; // Storage for thread g_handles
DWORD  g_ids[3];     // Storage for thread g_ids


//-----------------------------------------------------------------------------
// Name: genericFunc()
// Desc: Sample thread function
//-----------------------------------------------------------------------------
DWORD WINAPI genericFunc( LPVOID n )
{
	for( int i = 0; i < 100; ++i )
    {
        EnterCriticalSection( &g_criticalSection );
		cout << "genericFunc - " << i << endl;
        LeaveCriticalSection( &g_criticalSection );
    }

	return (DWORD)n;
}

//-----------------------------------------------------------------------------
// Name: printString()
// Desc: Sample thread function
//-----------------------------------------------------------------------------
DWORD WINAPI printString( LPVOID n )
{
    char* str = (char*)n;

	for( int i = 0; i < 100; ++i )
    {
        EnterCriticalSection( &g_criticalSection );
		cout << "printString - " << str << " " << i << endl;
        LeaveCriticalSection( &g_criticalSection );
    }

	return (DWORD)n;
}

//-----------------------------------------------------------------------------
// Name: printNumber()
// Desc: Sample thread function
//-----------------------------------------------------------------------------
DWORD WINAPI printNumber( LPVOID n )
{
    int num = (int)n;

	for( int i = num; i < (num + 100); ++i )
    {
        EnterCriticalSection( &g_criticalSection );
		cout << "printNumber - " << i << endl;
        LeaveCriticalSection( &g_criticalSection );
    }

	return (DWORD)n;
}

//-----------------------------------------------------------------------------
// Name: main()
// Desc: 
//-----------------------------------------------------------------------------
int main( int argc, char *argv[] )
{
	char string[] = "String";
    int  number = 2000;

    InitializeCriticalSection( &g_criticalSection );

    // First, call Win32's CreateThread Function and store the returned handle
    // This call will create and begin execution of the thread while the handle 
    // will allow us to recall the thread later for termination.

	g_handles[0] = CreateThread( NULL, 0, genericFunc,         NULL,   NULL, &g_ids[0] );
	g_handles[1] = CreateThread( NULL, 0, printString, (LPVOID)string, NULL, &g_ids[1] );
	g_handles[2] = CreateThread( NULL, 0, printNumber, (LPVOID)number, NULL, &g_ids[2] );

    // If we need to stop our primary thread (or as we normally call it - the
    // "application") we'll need to notify our app to the existence of our 
    // worker threads by calling WaitForMultipleObjects(...) and passing in 
    // our saved g_handles. This will allow the primary thread or app to block 
    // until they're finished and allow for a proper cleanup.

	WaitForMultipleObjects( 3, g_handles, true, INFINITE );

    // When WaitForMultipleObjects(...) stops blocking and returns, we know 
    // that our three threads have finished their tasks, and are now ready to 
    // be cleaned-up by calling CloseHandle(...) on them.

    CloseHandle( g_handles[0] );
    CloseHandle( g_handles[1] );
    CloseHandle( g_handles[2] );

    DeleteCriticalSection( &g_criticalSection );

	return 0;
}
