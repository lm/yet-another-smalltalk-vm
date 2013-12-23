CompiledCode subclass: CompiledMethod [

	| literals selector ownerClass sourceCode descriptors |


	"accessing"

	literals [
		^literals
	]


	selector [
		^selector
	]


	ownerClass [
		^ownerClass
	]


	sourceCode [
		^sourceCode
	]


	descriptors [
		^descriptors
	]


	sendTo: anObject [
		<primitive: MethodSendPrimitive>
	]


	"printing"

	printOn: aStream [
		ownerClass printOn: aStream.
		selector printOn: aStream.
	]

]
