//------------------------------------------------------------------------------
//           Name: lua_calling_lua_function.cpp
//         Author: Kevin Harris (kevin@codesampler.com)
//  Last Modified: 12/12/07
//    Description: This sample demonstrates how to call a Lua function from a
//                 C/C++ application.
//------------------------------------------------------------------------------

#include <iostream>
#include <string>
using namespace std;

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

//------------------------------------------------------------------------------
// Name: call_someLuaFunction()
// Desc: This is our helper or wrapper function, which makes it easier to call
//       our Lua function.
//------------------------------------------------------------------------------
int call_someLuaFunction( lua_State *pLuaState, int arg1, string arg2 ) 
{
	// Push the "someLuaFunction" function onto the stack
	lua_getglobal( pLuaState, "someLuaFunction" );

	// Push the first argument onto the stack
	lua_pushnumber( pLuaState, arg1 );

	// Push the second argument onto the stack
	lua_pushstring( pLuaState, arg2.c_str() );

	// Call the function with 2 arguments and 1 return value.
	lua_call( pLuaState, 2, 1 );

	// Fetch the result from the stack
	int nReturnValue = (int)lua_tonumber( pLuaState, -1 );

	// Restore the stack
	lua_pop( pLuaState, 1 );

	return nReturnValue;
}

//------------------------------------------------------------------------------
// Name: main()
// Desc: 
//------------------------------------------------------------------------------
void main( void )
{
	// Initialize Lua
	lua_State *pLuaState = luaL_newstate();

	// Load Lua libraries
	luaL_openlibs( pLuaState );

	// Execute the script 
	luaL_dofile( pLuaState, "test_function.lua" );

	// Call our special wrapper function, which will look-up and call the Lua 
	// function for us.
	int nReturnValue = call_someLuaFunction( pLuaState, 5, "test string" );

	// Output the return value returned by our Lua function
	cout << "The return value passed back from \"someLuaFunction\" is: " << nReturnValue << endl;

	// Cleanup Lua
	lua_close( pLuaState );
}
