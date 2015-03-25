ExpressionNode := Object [

	| returns assigments receiver messageExpressions sourceCode |


	returns [
		^returns
	]


	enableReturn [
		returns := true.
	]


	disableReturn [
		returns := false.
	]


	assigments [
		^assigments
	]


	receiver [
		^receiver
	]


	messageExpressions [
		^messageExpressions
	]


	sourceCode [
		^sourceCode
	]


	acceptVisitor: aVisitor [
		aVisitor visitExpressionNode: self.
		assigments do: [ :assigment | assigment acceptVisitor: aVisitor].
		receiver acceptVisitor: aVisitor.
		messageExpressions do: [ :exp | exp acceptVisitor: aVisitor].
	]

]
