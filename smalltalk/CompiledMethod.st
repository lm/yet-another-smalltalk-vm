CompiledMethod := CompiledCode [

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


	sendTo: anObject withArguments: anArray [
		<primitive: MethodSendArgsPrimitive>
		Error signal.
	]


	"printing"

	printOn: aStream [
		ownerClass basicPrintOn: aStream.
		selector printOn: aStream.
	]

]
