//------------------------------------------------------------------------------
//           Name: lua_calling_cpp_function.cpp
//         Author: Kevin Harris (kevin@codesampler.com)
//  Last Modified: 12/12/07
//    Description: This sample demonstrates how to register a C++ function with
//                 Lua so it can be called by an embedded Lua script.
//------------------------------------------------------------------------------

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

//------------------------------------------------------------------------------
// Name: computeAverageAndSum()
// Desc: This is the function we're going to expose to Lua.
//------------------------------------------------------------------------------
int computeAverageAndSum( lua_State *pLuaState )
{
	// Get number of arguments
	int nNumArgs = lua_gettop( pLuaState );
	double dSum = 0;

	// Loop through each argument
	for( int i = 1; i <= nNumArgs; ++i )
	{
		if( !lua_isnumber( pLuaState, i ) )
		{
			lua_pushstring( pLuaState, "Error - computeAverageAndSum passed a non-number!" );
			lua_error( pLuaState );
		}

		// Total the arguments
		dSum += lua_tonumber( pLuaState, i );
	}

	// Push the average as the first of two return results
	lua_pushnumber( pLuaState, dSum / nNumArgs );

	// Push the sum as the second of two return results
	lua_pushnumber( pLuaState, dSum );

	// Return the number of results
	return 2;
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

	// Register our function with Lua 
	lua_register( pLuaState, "computeAverageAndSum", computeAverageAndSum );

	// Run a test script to exercise our new function 
	luaL_dofile( pLuaState, "test_function.lua" );

	// Cleanup Lua 
	lua_close( pLuaState );
}
