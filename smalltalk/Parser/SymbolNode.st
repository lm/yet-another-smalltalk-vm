LiteralNode subclass: SymbolNode [

	acceptVisitor: aVisitor [
		aVisitor visitSymbolNode: self.
	]

]
