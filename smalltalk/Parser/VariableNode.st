LiteralNode subclass: VariableNode [

	name [
		^self value
	]


	acceptVisitor: aVisitor [
		aVisitor visitVariableNode: self.
	]

]
