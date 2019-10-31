#-------------------------------------------------------------------------------
# Define a function that takes one argument so we can customize our
# "Game Over!" message.
#-------------------------------------------------------------------------------

def printGameOver( playersName ):
    print( "Game Over... " + playersName + "!" )

# Now, lets call our new function and pass in some player names...

printGameOver( "camper_boy" )
printGameOver( "Haxxor" )
printGameOver( "c1pher" )

input( "\nPress Enter to exit..." )
