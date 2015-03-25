Number := Magnitude [

	"instance creation"

	class readFrom: aStream base: anInteger [
		| char sign int exp isFloat |

		isFloat := false.
		sign := 1.
		int := 0.

		char := aStream peek.
		char isNil ifTrue: [^0].
		char = $- ifTrue: [
			sign := -1.
			aStream next].

		char := aStream peek.
		char isNil ifTrue: [^0].
		char := char asUppercase.
		((char isDigit: anInteger) or: [char = $.]) ifFalse: [^0].

		[char notNil and: [(char := char asUppercase) isDigit: anInteger]] whileTrue: [
			aStream next.
			int := sign * char digitValue + (int * anInteger).
			char := aStream peek.
		].
		char isNil ifTrue: [^int].

		char = $. ifTrue: [
			aStream next.
			isFloat := true.
			[(char := aStream peek) notNil and: [(char := char asUppercase) isDigit: anInteger]] whileTrue: [
				sign := sign / anInteger.
				int := sign * char digitValue + int.
				aStream next.
			].
		].

		exp := 0.
		(anInteger = 10 and: [char = $E]) ifFalse: [
			^isFloat ifTrue: [int asFloat] ifFalse: [int]
		].

		aStream next.
		char := aStream peek.
		char isNil ifTrue: [^int].
		sign := 1.
		char = $+ ifTrue: [sign := 1. aStream next].
		char = $- ifTrue: [sign := -1. aStream next].

		[(char := aStream peek) notNil and: [char isDigit]] whileTrue: [
			exp := char digitValue + (exp * 10).
			aStream next.
		].

		int := int * (10 raisedToInteger: exp * sign).
		^int asFloat
	]


	class readFrom: aStream [
		^self readFrom: aStream base: 10
	]


	"arithmetic"

	+ aNumber [
		SubClassResponsibility signal.
	]


	- aNumber [
		SubClassResponsibility signal.
	]


	* aNumber [
		SubClassResponsibility signal.
	]


	/ aNumber [
		SubClassResponsibility signal.
	]


	// aNumber [
		"Answer the integer quotient defined by division with truncation toward negative infinity."
		SubClassResponsibility signal.
	]


	\\ aNumber [
		"Answer the integer remainder defined by division with truncation toward negative infinity.
		This is the modulo operation."
		SubClassResponsibility signal.
	]


	abs [
		self < 0 ifTrue: [^self negated].
		^self
	]


	negated [
		^0 - self
	]


	quo: aNumber [
		"Answer the integer quotient defined by division with truncation toward zero."
		SubClassResponsibility signal.
	]

	rem: aNumber [
		"Answer the integer remainder defined by division with truncation toward zero."
		SubClassResponsibility signal.
	]


	"mathematical functions"

	exp [

	]


	ln [

	]


	log: aNumber [

	]


	floorLog: aNumber [

	]


	raisedTo: aNumber [

	]


	raisedToInteger: anInteger [

	]


	sqrt [

	]


	squared [
		^self * self
	]


	"testing"

	even [

	]


	odd [

	]


	negative [
		^self < 0
	]


	positive [
		^self >= 0
	]


	strictlyPositive [
		^self > 0
	]


	sign [
		self < 0 ifTrue: [^-1].
		self > 0 ifTrue: [^1].
		^0
	]


	"truncation and round off"

	ceiling [

	]


	floor [

	]


	truncated [

	]


	truncateTo: aNumber [

	]


	rounded [

	]


	roundTo: aNumber [

	]


	"converting"

	degreesToRadians [

	]


	radiansToDegrees [

	]


	"intervals"

	to: stop [
		^Interval from: self to: stop
	]


	to: stop by: step [
		^Interval from: self to: stop by: step
	]


	to: stop by: step do: aBlock [
		| i |

		i := self.
		step > step zero
			ifTrue: [
				[i <= stop] whileTrue: [
					aBlock value: i.
					i := i + step]]
			ifFalse: [
				[i >= stop] whileTrue: [
					aBlock value: i.
					i := i + step]].
	]


	to: stop do: aBlock [
		| i |

		i := self.
		[i <= stop] whileTrue: [
			aBlock value: i.
			i := i + self unity].
	]

]
