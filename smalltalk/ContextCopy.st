ContextCopy := Context [

	| receiver arguments temporaries |


	class fromContext: aContex [
		^self new initializeFromContext: aContex
	]


	initializeFromContext: aContext [
		ic := aContext ic.
		code := aContext code.
		outer := aContext outer.
		home := aContext home.
		receiver := aContext receiver.
		arguments := aContext arguments.
		temporaries := aContext temporaries.
	]


	argumentAt: anInteger [
		^arguments at: anInteger
	]


	arguments [
		^arguments
	]


	temporaryAt: anInteger [
		^temporaries at: anInteger
	]


	temporaries [
		^temporaries
	]


	receiver [
		^receiver
	]


	examineOn: aStream [
		| instVars |

		instVars := self class instanceVariables.

		self basicPrintOn: aStream.
		aStream lf.

		1 to: instVars size do: [ :i |
			aStream
				space;
				nextPutAll: (instVars at: i);
				nextPutAll: ': '.
			(self instVarAt: i) == self
				ifTrue: [aStream nextPutAll: 'self (recursion)']
				ifFalse: [(self instVarAt: i) printOn: aStream].
			aStream lf].
	]

]
