//-----------------------------------------------------------------------------
//           Name: w32_inline_assembly.cpp
//         Author: Kevin Harris
//  Last Modified: 09/25/04
//    Description: Demonstrates how to use Visual Studio's ability to intermix 
//                 assembly code with your own C++ code.
//-----------------------------------------------------------------------------

#include <iostream>
using namespace std;

#pragma warning( disable : 4035 )  // Disable the warning message produced by the power2 function

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------

char msg1[]   = "Hello from";
char msg2[]   = "inline Assembly!";
char format[] = "%s %s\n";

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------

int power2( int num, int power );
void floatToInt(int *int_pointer, float f);


//-----------------------------------------------------------------------------
// Name: main()
// Desc: 
//-----------------------------------------------------------------------------
int main( void )
{
    //-------------------------------------------------------------------------
	// Example 1:
	// An __asm block can call C functions, including C library routines. 
	// The following example calls the printf library routine:
    //-------------------------------------------------------------------------

	__asm
	{
		mov  eax, offset msg2    ; Specify the offset of the second message string
		push eax                 ; Place the offset of msg1 on the stack
		mov  eax, offset msg1    ; Specify the offset of the first message string
		push eax                 ; Place the offset of msg2 on the stack
		mov  eax, offset format  ; Specify the offset of the format string
		push eax                 ; Place the offset of the formatting instructions on the stack
		call printf              ; Call the printf function of the C library.
		// Clean up the stack so main can exit cleanly!
		// Use the unused register EBX to do the cleanup.
		pop  ebx
		pop  ebx
		pop  ebx
	}


    //-------------------------------------------------------------------------
	// Example 2: 
	// Calling a function that contains Assembly code and then returns a 
    // value in a register
    //-------------------------------------------------------------------------

    cout << endl;
    cout << "3 times 2 to the power of 5 is " << power2(3,5) << endl;
    cout << endl;

    //-------------------------------------------------------------------------
    // Example 3: 
	// Another example of calling a function that contains Assembly code 
    //-------------------------------------------------------------------------

    int   i = 0;
    float f = 1234.121234f;

    floatToInt(&i, f);

    cout << "float passed in = " << f << endl;
    cout << "int passed out  = " << i << endl;
    cout << endl;


	return 0;
}

//-----------------------------------------------------------------------------
// Name: power2()
// Desc: 
// Note: This is how the power2() function above would look if it was coded in 
//       regular assembly instead of inline assembly.
//
//  ; POWER.ASM
//  ; Compute the power of an integer
//  ;
//         PUBLIC _power2
//  _TEXT SEGMENT WORD PUBLIC 'CODE'
//  _power2 PROC
//  
//          push ebp         ; Save EBP
//          mov ebp, esp     ; Move ESP into EBP so we can refer
//                           ;   to arguments on the stack
//          mov eax, [ebp+4] ; Get first argument
//          mov ecx, [ebp+6] ; Get second argument
//          shl eax, cl      ; EAX = EAX * ( 2 ^ CL )
//          pop ebp          ; Restore EBP
//          ret              ; Return with sum in EAX
//  
//  _power2 ENDP
//  _TEXT   ENDS
//          END
//
//-----------------------------------------------------------------------------
int power2( int num, int power )
{
    __asm
    {
        mov eax, num    ; Get first argument
        mov ecx, power  ; Get second argument
        shl eax, cl     ; EAX = EAX * ( 2 to the power of CL )
    }

    // NOTE: The return value for this function will be in the EAX register.
    //       This would normally generate a warning message but I have it 
    //       currently disabled. (see pragma at top of file)
}



//-----------------------------------------------------------------------------
// Name: floatToInt()
// Desc: 
// Note: At the assembly level the recommended workaround for the second FIST  
//       bug is the same for the first; inserting the FRNDINT instruction 
//       immediately preceding the FIST instruction.
//-----------------------------------------------------------------------------
void floatToInt( int *int_pointer, float f ) 
{
    __asm  
    {
        fld f
        mov edx, int_pointer
        FRNDINT
        fistp dword ptr [edx]
    }
}
