ArrayNode := LiteralNode [

	acceptVisitor: aVisitor [
		aVisitor visitArrayNode: self.
		self value do: [ :v | v acceptVisitor: aVisitor].
	]

]
