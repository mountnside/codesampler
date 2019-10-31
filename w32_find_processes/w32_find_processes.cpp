//-----------------------------------------------------------------------------
//           Name: w32_find_processes.cpp
//         Author: Kevin Harris
//  Last Modified: 09/25/04
//    Description: How to find a running process using 
//                 CreateToolhelp32Snapshot, Process32First, and Process32Next.      
//-----------------------------------------------------------------------------

#include <windows.h>
#include <tlhelp32.h>

#include <iostream>
using namespace std;

bool getProcessList( void )
{
    HANDLE hProcessSnap = NULL;
    bool bRet = false; 
    PROCESSENTRY32 pe32 = {0};
 
    //  Take a snapshot of all processes in the system. 
    hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );

    if( hProcessSnap == INVALID_HANDLE_VALUE )
        return false;

    // Fill in the size of the structure before using it. 
    pe32.dwSize = sizeof(PROCESSENTRY32);
 
    //  Walk the snapshot of the processes, and for each process, 
    //  display information.

    if( Process32First( hProcessSnap, &pe32 ) )
    { 
        DWORD dwPriorityClass;
        BOOL  bGotModule = FALSE;

        do
        {
            HANDLE hProcess;

            hProcess = OpenProcess (PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID); 
            dwPriorityClass = GetPriorityClass( hProcess ); 
            CloseHandle( hProcess );

            // Print the process's information. 
            cout << endl; 
            cout << "Priority Class Base\t" << pe32.pcPriClassBase << endl; 
            cout << "PID\t\t\t"             << pe32.th32ProcessID  << endl;
            cout << "Thread Count\t\t"      << pe32.cntThreads     << endl;
            cout << "Exe File\t\t"          << pe32.szExeFile      << endl;
        }
        while( Process32Next( hProcessSnap, &pe32 ) ); 

        bRet = true;
    }
    else
        bRet = false; // Could not walk the list of processes 
 
    // Don't forget to clean up the snapshot object.

    CloseHandle( hProcessSnap );

    return bRet;
}

void main( int argc, char *argv[] )
{
    getProcessList();
}