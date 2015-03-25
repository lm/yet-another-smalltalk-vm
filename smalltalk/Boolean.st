Boolean := Object [

	"logical operations"

	& aBoolean [
		SubClassResponsibility signal.
	]


	not [
		SubClassResponsibility signal.
	]


	xor: aBoolean [
		^(self == aBoolean) not
	]


	| aBoolean [
		SubClassResponsibility signal.
	]


	"controlling"

	ifTrue: aBlock [
		SubClassResponsibility signal.
	]


	ifFalse: aBlock [
		SubClassResponsibility signal.
	]


	ifTrue: aTrueBlock ifFalse: aFalseBlock [
		SubClassResponsibility signal.
	]


	ifFalse: aFalseBlock ifTrue: aTrueBlock [
		SubClassResponsibility signal.
	]


	and: aBlock [
		SubClassResponsibility signal.
	]


	or: aBlock [
		SubClassResponsibility signal.
	]


	"copying"

	deepCopy [
		^self
	]


	shallowCopy [
		^self
	]

]
