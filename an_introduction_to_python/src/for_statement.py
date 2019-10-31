print( "for loop #1\n" )

for i in range(5):
    print( i )


print( "\nfor loop #2\n" )
    
for i in range(1, 5):
    print( i )
else:
    print( "The for loop is over" )

print( "\nfor loop #3\n" )

weapons = [ "Pistol",
            "Rifle",
            "Grenade",
            "Rocket Launcher" ]

print( "-- Weapon Inventory --" )

for x in weapons:
    print( x )

input( "\nPress Enter to exit..." )
