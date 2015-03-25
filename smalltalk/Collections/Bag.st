Bag := Collection [

	<shape: IndexedShape>

	| contents |


	"instance creation"

	class new [
		^self new: 31
	]


	class new: anInteger [
		^self basicNew initialize: anInteger
	]


	"initialization"

	initialize: anInteger [
		contents := Dictionary new: anInteger.
	]


	"testing"

	includes: anObject [
		^contents includesKey: anObject
	]


	occurrencesOf: anObject [
		^contents at: anObject ifAbsent: [0]
	]


	"adding"

	add: anObject [
		^self add: anObject withOccurrences: 1
	]


	add: anObject withOccurrences: anInteger [
		(self includes: anObject)
			ifTrue: [contents at: anObject put: (self occurrencesOf: anObject) + anInteger]
			ifFalse: [contents at: anObject put: anInteger].
		^anObject
	]


	"removing"

	remove: anObject ifAbsent: aBlock [
		| count |

		count := contents at: anObject ifAbsent: [^aBlock value].
		count = 1
			ifTrue: [contents removeKey: anObject]
			ifFalse: [contents at: anObject put: count - 1].
		^anObject
	]


	"enumerating"

	do: aBlock [
		contents associationsDo: [ :assoc | assoc value timesRepeat: [aBlock value: assoc key]].
	]

]
