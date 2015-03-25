AssertError := Error [

	| expected actual |


	class expected: anObject got: anActualObject [
		^(self new initializeExpected: anObject actual: anActualObject) signal
	]


	initializeExpected: anObject actual: anActualObject [
		expected := anObject.
		actual := anActualObject.
	]


	defaultMessageText [
		^'Failed assertion: expected ', expected printString, ' but got ', actual printString
	]

]
