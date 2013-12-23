ArrayedCollection subclass: String [

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


	"printing"

	printOn: aStream [
		aStream
			nextPut: $';
			nextPutAll: self;
			nextPut: $'.
	]

]
