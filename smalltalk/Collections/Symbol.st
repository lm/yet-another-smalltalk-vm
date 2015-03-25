Symbol := String [

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


	"printing"

	printOn: aStream [
		aStream
			nextPut: $#;
			nextPutAll: self.
	]

]
