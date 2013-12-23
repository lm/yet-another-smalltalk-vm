LiteralNode subclass: StringNode [

	acceptVisitor: aVisitor [
		aVisitor visitStringNode: self.
	]

]
