StreamView := PositionableStream [

	| stream position limit |


	class on: aStream limit: anInteger [
		^super new initializeStream: aStream limit: anInteger
	]


	initializeStream: aStream limit: anInteger [
		stream := aStream.
		position := 1.
		limit := anInteger.
	]


	"accessing"

	next [
		position > limit ifTrue: [Error signal].
		position := position + 1.
		^stream next
	]


	next: anInteger [
		| newPosition |

		newPosition := position + anInteger.
		(newPosition - limit > 1) ifTrue: [Error signal].
		position := newPosition.
		^stream next: anInteger
	]


	contents [
		| contents |

		contents := stream contents.
		contents size > limit ifTrue: [contents := contents copyFrom: 1 to: limit].
		^contents
	]


	nextPut: anObject [
		ShouldNotImplement signal.
	]


	peek [
		position > limit ifTrue: [^nil].
		^stream peek
	]


	"positioning"

	position [
		^position
	]


	position: anInteger [
		anInteger > limit ifTrue: [Error signal].
		stream position: anInteger.
		position := anInteger.
	]


	reset [
		position := 1.
		stream reset.
	]


	setToEnd [
		stream position: limit.
	]


	skip: anInteger [
		self position: position + anInteger.
	]


	"testing"

	atEnd [
		^position = limit or: [stream atEnd]
	]

]
