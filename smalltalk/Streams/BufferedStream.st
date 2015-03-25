BufferedStream := PositionableStream [

	| buffer position buffered atEnd |


	"initialization"

	initialize [
		buffer := self createBuffer.
		position := buffer size + 1.
		buffered := 0.
	]


	createBuffer [
		SubClassResponsibility signal.
	]


	"accessing"

	peek [
		self atEnd ifTrue: [^nil].
		buffered = 0 ifTrue: [self bufferAtLeast: 1].
		^buffer at: position
	]


	next [
		| object |

		buffered = 0 ifTrue: [self bufferAtLeast: 1].
		object := buffer at: position.
		position := position + 1.
		buffered := buffered - 1.
		^object
	]


	next: anInteger [
		| collection |

		collection := buffer class new: anInteger.
		self next: anInteger into: collection startingAt: 1.
		^collection
	]


	contents [
		| contents |
		contents := CollectionStream on: (buffer class new: buffer size).
		[self atEnd] whileFalse: [contents nextPut: self next].
		^contents contents
	]


	"accessing private"

	next: anInteger into: aCollection startingAt: start [
		| read rest |

		read := self nextAvailable: anInteger into: aCollection startingAt: 1.
		read = anInteger ifTrue: [^self].

		rest := anInteger - read.
		rest > buffer size ifTrue: [
			read := self directNext: rest into: aCollection startingAt: read + 1.
			atEnd := read < rest.
			read < rest ifTrue: [(OutOfRangeError value: rest between: 1 and: read) signal].
			^self].

		self
			bufferAtLeast: rest;
			nextAvailable: rest into: aCollection startingAt: read + 1.
	]


	nextAvailable: anInteger into: aCollection startingAt: start [
		| available |

		buffered = 0 ifTrue: [^0].
		available := anInteger min: buffered.
		aCollection replaceFrom: start to: start + available - 1 with: buffer startingAt: position.
		position := position + available.
		buffered := buffered - available.
		^available
	]


	bufferAtLeast: least [
		position := 1.
		buffered := self directNext: buffer size into: buffer startingAt: 1.
		atEnd := buffered < buffer size.
		buffered < least ifTrue: [(OutOfRangeError value: least between: 1 and: buffered) signal].
	]


	directNext: anInteger into: aCollection startingAt: start [
		SubClassResponsibility signal.
	]


	"testing"

	atEnd [
		atEnd isNil ifTrue: [self bufferAtLeast: 0].
		^atEnd and: [buffered == 0]
	]

]
