//------------------------------------------------------------------------------
//           Name: lua_simple.cpp
//         Author: Kevin Harris (kevin@codesampler.com)
//  Last Modified: 12/12/07
//    Description: This sample demonstrates the bare essentials of 
//                 running an embedded Lua script from a C/C++ application.
//------------------------------------------------------------------------------

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

int main( int argc, char **argv )
{
	// Initialize Lua
	lua_State *pLuaState = luaL_newstate();

	// Load the Lua libraries
	luaL_openlibs( pLuaState );

	// Use Lua to run a script
	luaL_dofile( pLuaState, "test.lua" );

	// That's it... cleanup after Lua
	lua_close( pLuaState );

	return 0;
}
