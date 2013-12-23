LiteralNode subclass: NilNode [

	acceptVisitor: aVisitor [
		aVisitor visitNilNode: self.
	]

]
