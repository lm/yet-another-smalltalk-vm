SourceCode := Object [

	| source position sourceSize line column |


	"accessing"

	source [
		^source
	]


	position [
		^position
	]


	sourceSize [
		^sourceSize
	]


	sourceContents [
		^source
	]


	line [
		^line
	]


	column [
		^column
	]


	"printing"

	printOn: aStream [
		aStream nextPut: $'; nextPutAll: source; nextPut: $'; nextPut: $:.
		line printOn: aStream.
		aStream nextPut: $:.
		column printOn: aStream.
	]

]
