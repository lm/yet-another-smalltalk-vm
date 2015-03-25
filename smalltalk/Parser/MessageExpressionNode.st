MessageExpressionNode := Object [

	| selector arguments sourceCode |


	selector [
		^selector
	]


	arguments [
		^arguments
	]


	sourceCode [
		^sourceCode
	]


	acceptVisitor: aVisitor [
		aVisitor visitMessageExpressionNode: self.
		arguments do: [ :arg | arg acceptVisitor: aVisitor].
	]

]
