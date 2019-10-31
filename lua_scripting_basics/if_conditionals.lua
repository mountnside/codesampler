--------------------------------------------------------------------------------
--           Name: if_statements.lua
--         Author: Kevin Harris (kevin@codesampler.com)
--  Last Modified: 12/12/07
--    Description: This Lua script demonstrates how to use "if", "if-else", and
--                 "if-elseif-else" conditionals.
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
-- A simple "if" conditional:
--------------------------------------------------------------------------------

health = 0

if health <= 0 then
    print( "You're dead!" )
end

--------------------------------------------------------------------------------
-- An "if-else" conditional:
--------------------------------------------------------------------------------

health = 75

if health <= 0 then
    print( "You're dead!" )
else
    print( "You're alive!" )
end

--------------------------------------------------------------------------------
-- An "if-elseif-else" conditional:
--------------------------------------------------------------------------------

health = 24

if health <= 0 then
    print( "You're dead!" )
elseif health < 25 then
    print( "You're alive - but badly wounded!" )
else
    print( "You're alive!" )
end
