UndefinedObject := Object [

	"testing"

	isNil [
		^true
	]


	"copying"

	copy [
		^self
	]


	"printing"

	printOn: aStream [
		aStream nextPutAll: 'nil'.
	]

]
