Magnitude := Object [

	"comparing"

	< aMagnitude [
		SubClassResponsibility signal.
	]


	> aMagnitude [
		^aMagnitude < self
	]


	>= aMagnitude [
		^(self < aMagnitude) not
	]


	<= aMagnitude [
		^(aMagnitude < self) not
	]


	between: min and: max [
		^self >= min and: [self <= max]
	]


	"testing"

	max: aMagnitude [
		 ^aMagnitude <= self ifTrue: [self] ifFalse: [aMagnitude]
	]


	min: aMagnitude [
		^aMagnitude > self ifTrue: [self] ifFalse: [aMagnitude]
	]

]
