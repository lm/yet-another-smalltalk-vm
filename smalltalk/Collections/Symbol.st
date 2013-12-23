String subclass: Symbol [

	"comparing"

	hash [
		<primitive: HashPrimitive>
	]


	= anObject [
		^self == anObject
	]


	"converting"

	asSymbol [

	]


	"primitives"

	isKeyword [
		^self size > 1 and: [self last = $:]
	]


	keywords [
		| keywords pos |

		keywords := OrderedCollection new.
		pos := 1.

		self keysAndValuesDo: [ :i :ch |
			ch = $: ifTrue: [
				keywords add: (self copyFrom: pos to: i).
				pos := i + 1].
		].
		keywords isEmpty ifTrue: [keywords add: self].
		^keywords
	]


	"printing"

	printOn: aStream [
		aStream
			nextPut: $#;
			nextPutAll: self.
	]

]
