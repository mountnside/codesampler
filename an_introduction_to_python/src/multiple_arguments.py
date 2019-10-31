#-------------------------------------------------------------------------------
# Define a function that takes two arguments so we can customize our
# "Game Over!" message.
#-------------------------------------------------------------------------------

def printGameOver( playersName, totalKills ):
    print( "Game Over... " + playersName + "!" )
    print( "Total Kills: " + str( totalKills ) + "\n" )

# Now, lets call our new function and pass in some player names along with
# their total kills...

printGameOver( "camper_boy", 15 )
printGameOver( "Haxxor", 27 )
printGameOver( "c1pher", 51 )

# Note how argument order matters. Uncomment the next line to see an error.
#print(GameOver( 12 "killaHurtz" )

input( "\nPress Enter to exit..." )
