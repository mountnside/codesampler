#-------------------------------------------------------------------------------
# The variable "myNumber" is being used to hold an integer value of 25
#-------------------------------------------------------------------------------

myNumber = 25
print( myNumber )
print( type( myNumber ) )
print( "" )

#-------------------------------------------------------------------------------
# The variable "myBigNumber" is being used to hold a long integer value of
# 4294967296L. Note how it ends with a capital 'L'.
#-------------------------------------------------------------------------------

myBigNumber = 4294967296L
print( myBigNumber )
print( type( myBigNumber ) )
print( "" )

#-------------------------------------------------------------------------------
# The variable "pi" is being used to hold a float value of 3.141592653589793.
#
# NOTE: The Python variable types "int" and "float" have limited precision,
# which means there is a limit on how big or small the number can be.
# Run the script and note how "pi" gets chopped off and rounded up.
#-------------------------------------------------------------------------------

pi = 3.141592653589793
print( pi )
print( type( pi ) )
print( "" )

#-------------------------------------------------------------------------------
# The variable "myBoolean" is being used to hold the Boolean value of True
#-------------------------------------------------------------------------------

myBoolean = True
print( myBoolean )
print( type( myBoolean ) )
print( "" )

#-------------------------------------------------------------------------------
# The variable "myString" is being used to hold the string value
# "This is my string!"
#-------------------------------------------------------------------------------

myString = "This is my string!"
print( myString )
print( type( myString ) )
print( "" )

#-------------------------------------------------------------------------------
# The variable "myList" is being used to hold several integer values.
#-------------------------------------------------------------------------------

myList = [25, 50, 75, 100]

# You can print( each integer value by accessing its index
print( myList[0] )
print( myList[1] )
print( myList[2] )
print( myList[3] )
print( type( myList ) )

input( "\nPress Enter to exit..." )
