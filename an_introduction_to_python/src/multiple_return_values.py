#-------------------------------------------------------------------------------
# Define a function which takes a list of numbers and returns its average and 
# how many numbers were in the list.
#-------------------------------------------------------------------------------

def average( numberList ):
    numCount = 0
    runningTotal = 0
    
    for n in numberList:
        numCount = numCount + 1
        runningTotal = runningTotal + n

    # Return the average and the number count
    return runningTotal / numCount, numCount


# Test the average() function...

myNumbers = [5.0, 7.0, 8.0, 2.0]

theAverage, numberCount = average( myNumbers )

print( "The average of the list is " + str( theAverage ) + "." )
print( "The list contained " + str( numberCount ) + " numbers." )

input( "\nPress Enter to exit..." )
