True := Boolean [

	"logical operations"

	& aBoolean [
		^aBoolean
	]


	not [
		^false
	]


	| aBoolean [
		^self
	]


	"controlling"

	ifTrue: aBlock [
		^aBlock value
	]


	ifFalse: aBlock [
		^nil
	]


	ifTrue: aTrueBlock ifFalse: aFalseBlock [
		^aTrueBlock value
	]


	ifFalse: aFalseBlock ifTrue: aTrueBlock [
		^aTrueBlock value
	]


	and: aBlock [
		^aBlock value
	]


	or: aBlock [
		^self
	]


	"printing"

	printOn: aStream [
		aStream nextPutAll: 'true'.
	]

]
