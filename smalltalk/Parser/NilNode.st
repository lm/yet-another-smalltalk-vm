NilNode := LiteralNode [

	acceptVisitor: aVisitor [
		aVisitor visitNilNode: self.
	]

]
