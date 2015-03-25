LiteralNode := Object [

	| value sourceCode |


	class value: anObject [
		^self new initializeValue: anObject
	]


	initializeValue: anObject [
		value := anObject.
	]


	value [
		^value
	]


	sourceCode [
		^sourceCode
	]

]
