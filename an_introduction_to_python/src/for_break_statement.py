numbers = [100, 25, 125, 50, 150, 75, 175]

for x in numbers:
    print( x )
    # As soon as we find 50 - stop the search!
    if x == 50:
        print( "Found It!" )
        break;

input( "\nPress Enter to exit..." )
