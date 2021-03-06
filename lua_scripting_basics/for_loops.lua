--------------------------------------------------------------------------------
--           Name: for_loops.lua
--         Author: Kevin Harris (kevin@codesampler.com)
--  Last Modified: 12/12/07
--    Description: This Lua script demonstrates how to use for loops.
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
-- Count from 1 to 10
--------------------------------------------------------------------------------

for i = 1, 10 do
	print( i )
end

print( "\n" )

--------------------------------------------------------------------------------
-- Count backwards from 20 and stop at 10 by incrementing by -1.
--------------------------------------------------------------------------------

for i = 20, 10, -1 do
	print( i )
end

print( "\n" )

--------------------------------------------------------------------------------
-- Count from 0.0 to 2.0 and increment by 0.25 while doing it.
--------------------------------------------------------------------------------

for i = 0.0, 2.0, 0.25 do
	print( i )
end
