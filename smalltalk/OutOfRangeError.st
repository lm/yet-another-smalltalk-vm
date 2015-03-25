OutOfRangeError := Error [

	| value min max |


	"instance creation"

	class value: value between: min and: max [
		^self new initializeValue: value min: min max: max
	]


	"initialization"

	initializeValue: aValue min: aMin max: aMax [
		value := aValue.
		min := aMin.
		max := aMax.
	]


	"accessing"

	value [
		^value
	]


	min [
		^min
	]


	max [
		^max
	]


	defaultMessageText [
		^value printString, ' is out of allowed range (',
			min printString, ' to ', max printString, ')'
	]

]
