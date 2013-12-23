LiteralNode subclass: CharacterNode [

	acceptVisitor: aVisitor [
		aVisitor visitCharacterNode: self.
	]

]
