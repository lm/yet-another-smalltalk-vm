CharacterNode := LiteralNode [

	acceptVisitor: aVisitor [
		aVisitor visitCharacterNode: self.
	]

]
