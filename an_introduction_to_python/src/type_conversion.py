#-------------------------------------------------------------------------------
# With out the explicit conversion, Python will automatically upgrade your
# addition to "floating-point" addition, which may not be what you want 
# especially if your intention was to drop the decimal places.
#-------------------------------------------------------------------------------

myInt    = 25
myFloat  = 25.12
myString = "The value of 'myInt' is "

print( myInt + myFloat )

# With conversion - to int
print( myInt + int( myFloat ) )

# With conversion - to string
print( myString + str( myInt ) )

input( "\nPress Enter to exit..." )
