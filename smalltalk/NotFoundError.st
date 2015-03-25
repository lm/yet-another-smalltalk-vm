NotFoundError := Exception [

	| value |


	"instance creation"

	class value: anObject [
		^self new value: anObject
	]


	"accessing"

	value: anObject [
		value := anObject.
	]


	value [
		^value
	]


	defaultMessageText [
		^value printString, ' not found'
	]

]
