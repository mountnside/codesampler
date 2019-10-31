#-------------------------------------------------------------------------------
# Note how the argument "playersName" has a default value assigned to it.
# This will allow us to call the function even when have no player name to pass
# in to it.
#-------------------------------------------------------------------------------

def printGameOver( playersName="Guest" ):
    print( "Game Over... " + playersName + "!" )

# Now, lets call our new function and pass in some player names...

printGameOver( "camper_boy" )
printGameOver( "Haxxor" )
printGameOver( "c1pher" )
printGameOver()            # This call passes no name... so it will use "Guest"

input( "\nPress Enter to exit..." )
