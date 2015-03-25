Integer := Number [

	"arithmetic"

	// aNumber [
		^(self - (self < 0 ifTrue: [aNumber - 1] ifFalse: [0])) quo: aNumber
	]


	"mathematical functions"

	floorLog: radix [
		| me result |

		self < 0 ifTrue: ["Arithmetic"Error signal: 'cannot extract logarithm of a negative number'].
		radix <= radix unity ifTrue: [
			radix <= radix zero ifTrue: ["Arithmetic"Error signal: 'base of a logarithm cannot be negative'].
			radix = radix unity ifTrue: ["Arithmetic"Error signal: 'base of a logarithm cannot be 1'].
			^(self ceilingLog: radix reciprocal) negated
		].

		"radix isInteger ifFalse: [^(radix coerce: self) floorLog: radix]."
		me := self.
		result := 0.
		[me >= radix] whileTrue: [
			me := me // radix.
			result := result + 1.
		].

		^result
	]


	"iterators"

	timesRepeat: aBlock [
		1 to: self do: [ :each | aBlock value].
	]

]
