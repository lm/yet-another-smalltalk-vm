Integer subclass: SmallInteger [

	"comparing"

	hash [
		^self
	]


	identityHash [
		^self
	]


	< aNumber [
		 <primitive: IntLessThanPrimitive>
		 Error signal.
	]


	"arithmetic"

	+ aNumber [
		<primitive: IntAddPrimitive>
		Error signal.
	]


	- aNumber [
		<primitive: IntSubPrimitive>
		Error signal.
	]


	* aNumber [
		<primitive: IntMulPrimitive>
		Error signal.
	]


	/ aNumber [
		"IntDiv"
		<primitive: IntQuoPrimitive>
		Error signal.
	]


	"// aNumber [
		<primitive: IntDivPrimitive>
	]"


	\\ aNumber [
		<primitive: IntModPrimitive>
		Error signal.
	]


	quo: aNumber [
		<primitive: IntQuoPrimitive>
		Error signal.
	]


	rem: aNumber [
		<primitive: IntRemPrimitive>
		Error signal.
	]


	negated [
		<primitive: IntNegPrimitive>
		Error signal.
	]


	"bit manipulation"

	bitAnd: anInteger [
		<primitive: IntAndPrimitive>
		Error signal.
	]


	bitOr: anInteger [
		<primitive: IntOrPrimitive>
		Error signal.
	]


	bitXor: anInteger [
		<primitive: IntXorPrimitive>
		Error signal.
	]


	bitShift: anInteger [
		<primitive: IntShiftPrimitive>
		Error signal.
	]


	bitInvert [
		^self bitXor: -1
	]


	highBit [
		| n bit |

		self = 0 ifTrue: [^0].

		bit := 0.
		self < 0
			ifTrue: [
				"Increment the result by one if not a power of two"
				n := self negated.
				(n bitAnd: self) = n ifFalse: [bit := 1]]
			ifFalse: [
				n := self.
				bit := 0].

		[n > 1073741823] whileTrue: [
			bit := bit + 30.
			n := n bitShift: -30].

		n > 65535 ifTrue: [
			bit := bit + 16.
			n := n bitShift: -16].

		n > 255 ifTrue: [
			bit := bit + 8.
			n := n bitShift: -8].

		n > 15 ifTrue: [
			bit := bit + 4.
			n := n bitShift: -4].

		n > 3 ifTrue: [
			bit := bit + 2.
			n := n bitShift: -2].

		n > 1 ifTrue: [
			bit := bit + 1.
			n := n bitShift: -1].

		^n + bit
	]


	"private"

	zero [
		^0
	]


	unity [
		 ^1
	]


	asObject [
		<primitive: IntAsObjectPrimitive>
	]


	"copying"

	shallowCopy [
		^self
	]


	deepCopy [
		^self
	]


	"testing"

	even [
		^(self bitAnd: 1) = 0
	]


	odd [
		^(self bitAnd: 1) = 1
	]


	"printing"

	printOn: aStream [
		self printOn: aStream base: 10.
	]


	printOn: aStream base: base [
		aStream nextPutAll: (self printStringBase: base).
	]


	printString [
		^self printStringBase: 10
	]


	printStringBase: base [
		| int string i |

		self < 0
			ifTrue: [
				int := self negated.
				i := (int floorLog: base) + 2.
				string := String new: i.
				string at: 1 put: $-]
			ifFalse: [
				int := self.
				i := (int floorLog: base) + 1.
				string := String new: i].

		[string at: i put: (Character digitValue: int \\ base).
		i := i - 1.
		(int := int // base) > 0]
			whileTrue.

		^string
	]

]
