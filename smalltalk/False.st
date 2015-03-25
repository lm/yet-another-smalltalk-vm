False := Boolean [

	"logical operations"

	& aBoolean [
		^self
	]


	not [
		^true
	]


	| aBoolean [
		^aBoolean
	]


	"controlling"

	ifTrue: aBlock [
		^nil
	]


	ifFalse: aBlock [
		^aBlock value
	]


	ifTrue: aTrueBlock ifFalse: aFalseBlock [
		^aFalseBlock value
	]


	ifFalse: aFalseBlock ifTrue: aTrueBlock [
		^aFalseBlock value
	]


	and: aBlock [
		^self
	]


	or: aBlock [
		^aBlock value
	]


	"printing"

	printOn: aStream [
		aStream nextPutAll: 'false'.
	]

]
