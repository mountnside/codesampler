//-----------------------------------------------------------------------------
//           Name: w32_dll_source.h
//         Author: Kevin Harris
//  Last Modified: 09/25/04
//    Description: This is the DLLs header file for functions that I plan on 
//                 exporting or making available to other applications!
//
//                 To export functions, the __declspec(dllexport) keyword 
//                 must appear to the left of the function, class, or class 
//                 member function. If you do not place this keyword in front 
//                 of the function, the function can only be used locally 
//                 within the DLL and is not exported for external use by 
//                 other applications. 
//
//                 Also, if you use the __declspec(dllexport), you do not need 
//                 a .DEF file for exports.
//-----------------------------------------------------------------------------

#include <string>
using namespace std;

__declspec(dllexport) int StringToNumber( string *str );
__declspec(dllexport) void NumberToString( int value, string *str );
