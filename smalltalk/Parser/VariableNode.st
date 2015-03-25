VariableNode := LiteralNode [

	name [
		^self value
	]


	acceptVisitor: aVisitor [
		aVisitor visitVariableNode: self.
	]

]
