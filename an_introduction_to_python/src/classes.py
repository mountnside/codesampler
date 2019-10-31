class GameCharacter:
    def __init__( self, name, hitPoints ):
        self.name = name
        self.hitPoints = hitPoints

    def name( self ):
        return self.name


x = GameCharacter( "Knight", 25 )

print( x.name )
print( x.hitPoints )
