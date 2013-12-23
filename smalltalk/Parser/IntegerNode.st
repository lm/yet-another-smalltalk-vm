LiteralNode subclass: IntegerNode [

	acceptVisitor: aVisitor [
		aVisitor visitIntegerNode: self.
	]

]
