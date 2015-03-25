CompiledBlock := CompiledCode [

	| method sourceCode descriptors |


	"accessing"

	method [
		^method
	]


	sourceCode [
		^sourceCode
	]


	descriptors [
		^descriptors
	]


	literals [
		^method literals
	]


	ownerClass [
		^method ownerClass
	]


	"printing"

	printOn: aStream [
		| index |

		method printOn: aStream.
		aStream nextPutAll: '[]'.
	]

]
