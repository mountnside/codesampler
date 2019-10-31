def printGameOver( playersName, totalKills ):
    print( "Game Over... " + playersName + "!" )
    print( "Total Kills: " + str( totalKills ) + "\n" )

# Now, lets call our new function and pass in some player names along with
# their total kills...

printGameOver( "camper_boy", 15 )
printGameOver( "Haxxor", 27 )
printGameOver( "c1pher", 51 )

#-------------------------------------------------------------------------------
# If we call the function using keyword arguemnts, order doesn't matter.
#-------------------------------------------------------------------------------

printGameOver( totalKills=12, playersName="killaHurtz" )

input( "\nPress Enter to exit..." )
