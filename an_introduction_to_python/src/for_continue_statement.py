numbers = [100, 25, 125, 50, 150, 75, 175]

for x in numbers:
    # Skip all triple digit numbers
    if x >= 100:
        continue;
    print( x )

input( "\nPress Enter to exit..." )
