String := ArrayedCollection [

	<shape: StringShape>


	"comparing"

	hash [
		<primitive: StringHashPrimitive>
	]


	"= aCollection [
		<primitive: ArrayEqualsPrimitive>
	]"


	"converting"

	asSymbol [
		<primitive: StringAsSymbolPrimitive>
	]


	asLowercase [
		| newString |

		newString := self class new: self size.
		1 to: self size do: [ :i | newString at: i put: (self at: i) asLowercase].
		^newString
	]


	asNumber [
		^Number readFrom: (CollectionStream on: self)
	]


	trimSeparators [
		1 to: self size do: [ :start |
			(self at: start) isSeparator ifFalse: [
				self size to: start by: -1 do: [ :stop |
					(self at: stop) isSeparator ifFalse: [
						^self copyFrom: start to: stop
					].
				].
			].
		].
		^''
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
			nextPut: $';
			nextPutAll: self;
			nextPut: $'.
	]

]
