Dictionary := HashedCollection [

	"instance creation"

	class new [
		^self new: 31
	]


	"accessing"

	at: key [
		^self at: key ifAbsent: [(NotFoundError value: key) signal]
	]


	at: key ifAbsent: aBlock [
		| index |

		index := self findIndex: key ifAbsent: [^aBlock value].
		^(contents at: index) value
	]


	at: key ifPresent: aBlock [
		| index |

		index := self findIndex: key ifAbsent: [^nil].
		^aBlock value: (contents at: index) value
	]


	at: key put: anObject [
		| index |

		index := self findIndex: key.
		(contents at: index) isNil
			ifTrue: [self atNewIndex: index put: key -> anObject]
			ifFalse: [(contents at: index) value: anObject].

		^anObject
	]


	keyAtValue: anObject [
		^self keyAtValue: anObject ifAbsent: [(NotFoundError value: anObject) signal]
	]


	keyAtValue: anObject ifAbsent: aBlock [
		self associationsDo: [ :assoc | assoc value == anObject ifTrue: [^assoc key]].
		^aBlock value
	]


	keys [
		| keys |

		keys := Set new: self size.
		self keysDo: [ :key | keys add: key].
		^keys
	]


	values [
		| values |

		values := Bag new: self size.
		self do: [ :value | values add: value].
		^values
	]


	"adding"

	add: anAssociation [
		| index assoc |

		index := self findIndex: anAssociation key.
		assoc := contents at: index.

		assoc isNil
			ifTrue: [
				self atNewIndex: index put: anAssociation.
				^anAssociation]
			ifFalse: [
				assoc value: anAssociation value.
				^assoc].
	]


	"removing"

	remove: anAssociation ifAbsent: aBlock [
		self removeKey: anAssociation key ifAbsent: aBlock.
		^anAssociation
	]


	removeKey: anObject [
		^self removeKey: anObject ifAbsent: [(NotFoundError value: anObject) signal]
	]


	removeKey: anObject ifAbsent: aBlock [
		| index assoc |

		index := self findIndex: anObject ifAbsent: [^aBlock value].
		assoc := contents at: index.
		contents at: index put: nil.
		tally := tally - 1.
		^assoc value
	]


	"enumerating"

	do: aBlock [
		contents do: [ :assoc | assoc notNil ifTrue: [aBlock value: assoc value]].
	]


	"dictionary enumerating"

	associationsDo: aBlock [
		super do: aBlock.
	]


	keysDo: aBlock [
		self associationsDo: [ :assoc | aBlock value: assoc key].
	]


	keysAndValuesDo: aBlock [
		self associationsDo: [ :assoc | aBlock value: assoc key value: assoc value].
	]


	"testing"

	includesKey: anObject [
		^super includes: anObject
	]


	includes: anObject [
		self do: [ :element | element = anObject ifTrue: [^true]].
		^false
	]


	"private"

	findIndex: anObject [
		| size index assoc |

		size := contents size.
		index := (anObject hash bitAnd: size - 1) + 1.

		[assoc := contents at: index.
		(assoc isNil or: [assoc key = anObject]) ifTrue: [^index].
		index := index == size ifTrue: [1] ifFalse: [index + 1]]
			repeat.
	]


	"printing"

	printOn: aStream [
		| items |

		super basicPrintOn: aStream.

		items := 0.
		aStream nextPutAll: ' ('.
		self associationsDo: [ :v |
			(items := items + 1) = 7 ifTrue: [
				aStream nextPutAll: '...)'.
				^self].
			v value == self
				ifTrue: [
					v key basicPrintOn: aStream.
					aStream nextPutAll: '->self(recursion)']
				ifFalse: [
					v printOn: aStream].
			aStream space].
		aStream nextPut: $).
	]

]
