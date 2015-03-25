ParseError := Error [

	| token sourceCode |


	defaultMessageText [
		^'Parse error: unexpected ',
			(token isEmpty ifTrue: ['end of input'] ifFalse: [token]),
			' in ''', sourceCode source,
			''' line ', sourceCode line printString,
			' column ', sourceCode column printString
	]

]
