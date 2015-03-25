FalseNode := LiteralNode [

	acceptVisitor: aVisitor [
		aVisitor visitFalseNode: self.
	]

]
