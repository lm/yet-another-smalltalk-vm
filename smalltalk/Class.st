Class := ClassDescription [

	| name comment category classVariables |


	"accessing"

	name [
		^name
	]


	classVariables [
		^classVariables
	]


	initialize [

	]


	"printing"

	printOn: aStream [
		aStream nextPutAll: name.
	]

]
