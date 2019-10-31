//-----------------------------------------------------------------------------
//           Name: w32_mini_dump.cpp
//         Author: Kevin Harris
//  Last Modified: 07/22/05
//    Description: This sample demonstrates how to make use of the mini-dump 
//                 feature in Windows 2000 and Windows Xp. 
//
//                 Here's what to do:
//
//                 1. Compile and run the test application to see it crash.
//
//                 2. Once it crashes, click "Ok" on the error message, which 
//                    prompts you to create a mini-dump file. The mini-dump 
//                    file will be created in the same directory as the 
//                    executable and will have name like similar to this:
//                    "w32_mini_dump.exe.1081442621.dmp".
//
//                 3. Double-click on the mini-dump file to load it into Visual
//                    Studio.
//
//                 4. Once it's loaded, hit F5 to execute it. You can now see 
//                    what caused your customer's crash.
//
// NOTE: If you can't compile the project because you don't have access to 
// "dbghelp.h", you'll probably need to download the latest Platform SDK from 
// Microsoft's MSDN site:
//
// http://www.microsoft.com/whdc/devtools/debugging/default.mspx
//
// Additional information concerning mini-dumps:
//
// http://www.codeproject.com/debug/postmortemdebug_standalone1.asp
// http://msdn.microsoft.com/msdnmag/issues/02/06/Bugslayer/default.aspx
// http://msdn.microsoft.com/msdnmag/issues/02/03/hood/default.aspx
//-----------------------------------------------------------------------------

#include "miniDumper.h"

miniDumper g_miniDumper( true );

void main( void )
{
    // Do something stupid so we can test the MiniDumper!
    int* i = NULL;
    *i = NULL;
}
