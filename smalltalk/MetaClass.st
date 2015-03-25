MetaClass := ClassDescription [

	| instanceClass |


	"accessing"

	instanceClass [
		^instanceClass
	]


	name [
		^instanceClass name
	]


	"printing"

	printOn: aStream [
		instanceClass printOn: aStream.
		aStream nextPutAll: ' class'.
	]

]
