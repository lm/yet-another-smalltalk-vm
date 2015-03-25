HashedCollection := Collection [

	| contents tally |


	"instance creation"

	class new: anInteger [
		| collection |

		collection := super new.
		collection initialize: (self computeSize: anInteger).
		^collection
	]


	class computeSize: anInteger [
		| size |

		size := 8 max: (anInteger * 4 + 2) // 3.
		(size bitAnd: size - 1) = 0 ifFalse: [
			size := size - 1.
			size := size bitOr: (size bitShift: -1).
			size := size bitOr: (size bitShift: -2).
			size := size bitOr: (size bitShift: -4).
			size := size bitOr: (size bitShift: -8).
			size := size bitOr: (size bitShift: -16).
			size := size + 1].
		^size
	]


	"initialization"

	initialize: anInteger [
		contents := Array new: anInteger.
		tally := 0.
	]


	"accesing"

	size [
		^tally
	]


	at: key [
		SubClassResponsibility signal.
	]


	at: key put: anObject [
		SubClassResponsibility signal.
	]


	atNewIndex: anInteger put: anObject [
		contents at: anInteger put: anObject.
		tally := tally + 1.
		tally = contents size ifTrue: [self grow].
	]


	"adding"

	grow [
		| oldContents |

		oldContents := contents.
		self initialize: oldContents size * 2.
		oldContents do: [ :item | self add: item].
	]


	"removing"

	remove: anObject ifAbsent: aBlock [
		| index |

		index := self findIndex: anObject ifAbsent: [^aBlock value].
		contents at: index put: nil.
		tally := tally - 1.
		"TODO: shrink contents array?"
		^anObject
	]


	"enumerating"

	do: aBlock [
		contents do: [ :item | item notNil ifTrue: [aBlock value: item]].
	]


	"testing"

	includes: anObject [
		self findIndex: anObject ifAbsent: [^false].
		^true
	]


	"private"

	findIndex: anObject ifAbsent: aBlock [
		| index |

		index := self findIndex: anObject.
		(contents at: index) isNil ifTrue: [aBlock value].
		^index
	]


	findIndex: anObject [
		SubClassResponsibility signal.
	]


	"printing"

	examineOn: aStream [
		super examineOn: aStream.
		self associationsDo: [ :v |
			aStream nextPutAll: ' - '.
			v == self
				ifTrue: [aStream nextPutAll: 'self (recursion)']
				ifFalse: [v printOn: aStream].
			aStream lf].
	]

]
