CollectionStream := PositionableStream [

	| collection position end |


	"instance creation"

	class on: aCollection [
		^self new
			initializeCollection: aCollection
			from: 1
			to: aCollection size + 1
	]


	class with: aCollection [
		^self new
			initializeCollection: aCollection
			from: 1
			to: 1
	]


	"initializing"

	initializeCollection: aCollection from: aPos to: anEnd [
		collection := aCollection.
		position := aPos.
		end := anEnd.
	]


	"testing"

	isEmpty [
		^position = 1
	]


	"accessing"

	peek [
		self atEnd ifTrue: [^nil].
		^collection at: position
	]


	next [
		| object |

		self atEnd ifTrue: [^nil].
		object := collection at: position.
		position := position + 1.
		^object
	]


	next: anInteger [
		| newPos result |

		newPos := position + anInteger.
		result := collection copyFrom: position to: newPos - 1.
		position := newPos.
		^result
	]


	nextAvailablePutAllOn: aStream [
		[self atEnd] whileFalse: [aStream nextPut: self next].
	]


	putAllOn: aStream [
		1 to: end - 1 do: [ :i | aStream nextPut: (collection at: i)].
	]


	nextPut: anObject [
		position > collection size ifTrue: [collection := collection copyResized: position * 2].
		collection at: position put: anObject.
		position := position + 1.
		end := end max: position.
		^anObject
	]


	atEnd [
		^position >= end
	]


	contents [
		^collection copyFrom: 1 to: end - 1
	]


	upToEnd [
		^collection copyFrom: position to: end - 1
	]


	size [
		^end - 1
	]


	"positioning"

	position [
		^position
	]


	position: anInteger [
		(anInteger < 1 or: [anInteger > end]) ifTrue: [Error signal].
		position := anInteger.
	]


	reset [
		position := 1.
		end := 1.
	]


	setToEnd [
		position := end.
	]


	species [
		^collection class
	]

]
