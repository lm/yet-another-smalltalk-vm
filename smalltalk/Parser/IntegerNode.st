IntegerNode := LiteralNode [

	acceptVisitor: aVisitor [
		aVisitor visitIntegerNode: self.
	]

]
