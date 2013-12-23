LiteralNode subclass: FalseNode [

	acceptVisitor: aVisitor [
		aVisitor visitFalseNode: self.
	]

]
