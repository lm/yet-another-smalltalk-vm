Interval := ArrayedCollection [

	<shape: FixedShape>

	| start stop step |


	"instance creation"

	class from: start to: stop [
		^self new initializeFrom: start to: stop by: 1
	]


	class from: start to: stop by: step [
		^self new initializeFrom: start to: stop by: step
	]


	"initializing"

	initializeFrom: aStart to: aStop by: aStep [
		start := aStart.
		stop := aStop.
		step := aStep.
	]


	"acessing"

	at: anInteger [
		(anInteger >= 1 and: [anInteger <= self size]) ifTrue: [
			^start + (step * (anInteger - 1))].
		OutOfRangeError signal.
	]


	at: anInteger put: anObject [
		Error signal.
	]


	first [
		^start
	]


	increment [
		^step
	]


	last [
		^stop - (stop - start \\ step)
	]


	size [
		step < 0 ifTrue: [
			start < stop ifTrue: [^0].
			^stop - start // step + 1].
		stop < start ifTrue: [^0].
		^stop - start // step + 1
	]


	"comparing"

	= anObject [
		^self class == anObject class and: [start = anObject first
			and: [step = anObject increment and: [self size = anObject size]]].
	]


	hash [
		^(((start hash bitShift: 2) bitOr: stop hash) bitShift: 1) bitOr: self size
	]


	"adding"

	add: anObject [
		ShouldNotImplement signal.
	]


	"removing"

	remove: anObject [
		ShouldNotImplement signal.
	]


	"enumerating"

	collect: aBlock [
		| collection index |

		collection := Array new: self size.
		index := 0.
		start to: stop by: step do: [ :i |
			collection at: (index := index + 1) put: (aBlock value: i)].
		^collection
	]


	do: aBlock [
		start to: stop by: step do: aBlock.
	]


	reverseDo: aBlock [
		stop to: start by: step negated do: aBlock.
	]

]
