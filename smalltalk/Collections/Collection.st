Collection := Iterator [

	"accessing"

	size [
		| tally |

		tally := 0.
		self do: [ :each | tally := tally + 1].
		^tally
	]


	"testing"

	includes: anObject [
		self do: [ :v | v = anObject ifTrue: [^true]].
		^false
	]


	includesAnyOf: aCollection [
		^self anySatisfy: [ :v | aCollection includes: v]
	]


	isEmpty [
		^self size = 0
	]


	occurrencesOf: anObject [
		| tally |

		tally := 0.
		self do: [ :v | v = anObject ifTrue: [tally := tally + 1]].
		^tally
	]


	"adding"

	add: anObject [
		SubClassResponsibility signal.
	]


	addAll: aCollection [
		aCollection do: [ :v | self add: v].
		^aCollection
	]


	"removing"

	remove: anObject [
		^self remove: anObject ifAbsent: [(NotFoundError value: anObject) signal]
	]


	remove: anObject ifAbsent: aBlock [
		SubClassResponsibility signal.
	]


	removeAll: aCollection [
		aCollection do: [ :v | self remove: v].
		^aCollection
	]


	"private"

	emptyCheck [
		self isEmpty ifTrue: [OutOfRangeError signal].
	]


	"printing"

	printOn: aStream [
		| items |

		super printOn: aStream.

		items := 0.
		aStream nextPutAll: ' ('.
		self do: [ :v |
			(items := items + 1) = 7 ifTrue: [
				aStream nextPutAll: '...)'.
				^self].
			v == self
				ifTrue: [aStream nextPutAll: 'self(recursion)']
				ifFalse: [v basicPrintOn: aStream].
			aStream space].
		aStream nextPut: $).
	]

]
