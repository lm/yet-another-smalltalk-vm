StringNode := LiteralNode [

	acceptVisitor: aVisitor [
		aVisitor visitStringNode: self.
	]

]
