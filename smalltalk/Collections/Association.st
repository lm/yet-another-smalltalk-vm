Association := Object [

	| key value |


	"instance creation"

	class key: key value: value [
		^self new key: key value: value
	]


	"initialization"

	key: aKey value: aValue [
		key := aKey.
		value := aValue.
	]


	"accessing"

	key [
		^key
	]


	value [
		^value
	]


	value: anObject [
		value := anObject.
	]


	"printing"

	printOn: aStream [
		key == self ifTrue: [aStream nextPutAll: '(recursion)'] ifFalse: [key basicPrintOn: aStream].
		aStream nextPutAll: '->'.
		value == self ifTrue: [aStream nextPutAll: '(recursion)'] ifFalse: [value basicPrintOn: aStream].
	]

]
