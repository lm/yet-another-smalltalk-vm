MethodNode := Object [

	| className selector pragmas body sourceCode |


	className [
		^className
	]


	selector [
		^selector
	]


	pragmas [
		^pragmas
	]


	body [
		^body
	]


	arguments [
		^body arguments
	]


	temporaries [
		^body temporaries
	]


	expressions [
		^body expressions
	]


	sourceCode [
		^sourceCode
	]


	acceptVisitor: aVisitor [
		aVisitor visitMethodNode: self.
		pragmas do: [ :pragma | pragma acceptVisitor: aVisitor].
		body acceptVisitor: aVisitor.
	]

]
