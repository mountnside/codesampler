--------------------------------------------------------------------------------
--           Name: functions.lua
--         Author: Kevin Harris (kevin@codesampler.com)
--  Last Modified: 12/12/07
--    Description: This Lua script demonstrates how to create functions.
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
-- Create a function that writes the Fibonacci series up to n.
--------------------------------------------------------------------------------

function fibonacci( n )
	a, b = 0, 1
	while b < n do
        	print( b )
		a, b = b, a + b
	end
end

-- Now, call the function we just defined

fibonacci( 2000 )

print( "\n" )

--------------------------------------------------------------------------------
-- Define a function that adds two numbers together and returns the sum using
-- a single return value.
--------------------------------------------------------------------------------

function myAddFunc( a, b )
	return a + b
end

-- Now, call the function we just defined

sum = myAddFunc( 2, 2 )
print( sum )

sum = myAddFunc( 6, 4 )
print( sum )

print( "\n" )

--------------------------------------------------------------------------------
-- Define a function that takes two numbers and returns both their sum and 
-- product using two return values.
--------------------------------------------------------------------------------

function myAddAndMultiplyFunc( a, b )
	return a + b, a * b
end

-- Now, call the function we just defined

sum, product = myAddAndMultiplyFunc( 2, 5 )
print( sum )
print( product )

sum, product = myAddAndMultiplyFunc( 5, 5 )
print( sum )
print( product )

print( "\n" )

--------------------------------------------------------------------------------
-- Define a function that takes a variable argument list and simly prints
-- them out
--------------------------------------------------------------------------------

function varArgFunction(...)
	print( unpack( arg ) )
end

-- Now, call the function we just defined

varArgFunction( "A", "B", "C" )
varArgFunction( 1, 2, 3, 4, 5 )

