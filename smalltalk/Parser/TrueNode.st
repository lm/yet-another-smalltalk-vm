LiteralNode subclass: TrueNode [

	acceptVisitor: aVisitor [
		aVisitor visitTrueNode: self.
	]

]
