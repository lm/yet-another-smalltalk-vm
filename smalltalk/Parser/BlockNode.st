BlockNode := Object [

	| arguments temporaries expressions scope sourceCode |


	arguments [
		^arguments
	]


	temporaries [
		^temporaries
	]


	expressions [
		^expressions
	]


	scope [
		^scope
	]


	sourceCode [
		^sourceCode
	]


	acceptVisitor: aVisitor [
		aVisitor visitBlockNode: self.
		arguments do: [ :arg | arg acceptVisitor: aVisitor].
		temporaries do: [ :tmp | tmp acceptVisitor: aVisitor].
		expressions do: [ :expr | expr acceptVisitor: aVisitor].
	]

]
