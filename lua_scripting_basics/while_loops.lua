--------------------------------------------------------------------------------
--           Name: while_loops.lua
--         Author: Kevin Harris (kevin@codesampler.com)
--  Last Modified: 12/12/07
--    Description: This Lua script demonstrates how to use while loops.
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
-- While loop on a simple counter...
--------------------------------------------------------------------------------

i = 0

while i < 10 do
	io.write( i )
	io.write( "\n" )
	i = i + 1
end

--------------------------------------------------------------------------------
-- While loop through a list of strings...
--------------------------------------------------------------------------------

myList = { "Alpha", "Bravo", "Charlie", "Delta", "Echo", "Foxtrot" }
i = 1

while myList[i] do
	io.write( myList[i] )
	io.write( "\n" )
	i = i + 1
end
