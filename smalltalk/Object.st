Object := nil [

	"accessing"

	at: anInteger [
		<primitive: AtPrimitive>
		(OutOfRangeError value: anInteger between: 1 and: self basicSize) signal.
	]


	at: anInteger put: anObject [
		<primitive: AtPutPrimitive>
		(OutOfRangeError value: anInteger between: 1 and: self basicSize) signal.
	]


	basicAt: anInteger [
		<primitive: AtPrimitive>
		(OutOfRangeError value: anInteger between: 1 and: self basicSize) signal.
	]


	basicAt: anInteger put: anObject [
		<primitive: AtPutPrimitive>
		(OutOfRangeError value: anInteger between: 1 and: self basicSize) signal.
	]


	size [
		<primitive: SizePrimitive>
	]


	basicSize [
		<primitive: SizePrimitive>
	]


	yourself [
		^self
	]


	"system primitives"

	instVarAt: anInteger [
		<primitive: InstVarAtPrimitive>
		(OutOfRangeError value: anInteger between: 1 and: self class instanceSize) signal.
	]


	instVarAt: anInteger put: anObject [
		<primitive: InstVarAtPutPrimitive>
		(OutOfRangeError value: anInteger between: 1 and: self class instanceSize) signal.
	]


	become: otherObject [
		<primitive: BecomePrimitive>
	]


	"comparing"

	= anObject [
		<primitive: IdentityPrimitive>
	]


	== anObject [
		<primitive: IdentityPrimitive>
	]


	hash [
		<primitive: HashPrimitive>
	]


	~= anObject [
		^self = anObject == false
	]


	~~ anObject [
		^self == anObject == false
	]


	identityHash [
		<primitive: HashPrimitive>
	]


	"private"

	species [
		^self class
	]


	"class membership"

	class [
		<primitive: ClassPrimitive>
	]


	isKindOf: aClass [
		^self class == aClass
			ifTrue: [true]
			ifFalse: [self class inheritsFrom: aClass]
	]


	isMemberOf: aClass [
		^self class == aClass
	]


	respondsTo: aSymbol [
		^self class canUnderstand: aSymbol
	]


	"testing"

	isNil [
		^false
	]


	notNil [
		^self isNil not
	]


	"copying"

	copy [
		^self shallowCopy
	]


	shallowCopy [
		| copy class index |

		class := self class.

		class isIndexable
			ifTrue: [
				index := self basicSize.
				copy := class basicNew: index.
				[index > 0] whileTrue: [
					copy basicAt: index put: (self basicAt: index).
					index := index - 1]]
			ifFalse: [
				copy := class basicNew].

		index := class instanceSize.
		[index > 0] whileTrue: [
			copy instVarAt: index put: (self instVarAt: index).
			index := index - 1].

		^copy
	]


	deepCopy [
		| copy class index |

		class := self class.

		class isIndexable
			ifTrue: [
				index := self basicSize.
				copy := class basicNew: index.
				[index > 0] whileTrue: [
					copy basicAt: index put: (self basicAt: index) copy.
					index := index - 1]]
			ifFalse: [
				copy := class basicNew].

		index := class instanceSize.
		[index > 0] whileTrue: [
			copy instVarAt: index put: (self instVarAt: index) copy.
			index := index - 1].

		^copy
	]


	"error handling"

	doesNotUnderstand: aMessage [
		MessageNotUnderstood new
			receiver: self;
			message: aMessage;
			signal.
	]


	"associating"

	-> value [
		^Association key: self value: value
	]


	"printing"

	printString [
		| stream |

		stream := CollectionStream with: (String new: 128).
		self printOn: stream.
		^stream contents
	]


	printNl [
		self printOn: Transcript.
		Transcript lf.
	]


	print [
		self printOn: Transcript.
	]


	printOn: aStream [
		self basicPrintOn: aStream.
	]


	basicPrintOn: aStream [
		| name |

		name := self class name.
		aStream
			nextPutAll: (name first isVowel ifTrue: ['an '] ifFalse: ['a ']);
			nextPutAll: name.
	]


	inspect [
		self examineOn: Transcript.
	]


	examineOn: aStream [
		| instVars |

		instVars := self class instanceVariables.

		self basicPrintOn: aStream.
		aStream lf.

		1 to: instVars size do: [ :i |
			aStream
				space;
				nextPutAll: (instVars at: i);
				nextPutAll: ': '.
			(self instVarAt: i) == self
				ifTrue: [aStream nextPutAll: 'self (recursion)']
				ifFalse: [(self instVarAt: i) basicPrintOn: aStream].
			aStream lf].

		1 to: self basicSize do: [ :i |
			aStream nextPutAll: ' - '.
			(self basicAt: i) == self
				ifTrue: [aStream nextPutAll: 'self (recursion)']
				ifFalse: [(self basicAt: i) basicPrintOn: aStream].
			aStream lf].
	]

]
