Block := Object [

	<shape: BlockShape>

	| compiledBlock receiver outerContext homeContext |


	"accessing"

	compiledBlock [
		^compiledBlock
	]


	receiver [
		^receiver
	]


	outerContext [
		^outerContext
	]


	homeContext [
		^homeContext
	]


	"evaluating"

	value [
		<primitive: BlockValuePrimitive>
		OutOfRangeError signal.
	]


	value: arg [
		<primitive: BlockValuePrimitive1>
		OutOfRangeError signal.
	]


	value: arg1 value: arg2 [
		<primitive: BlockValuePrimitive2>
		OutOfRangeError signal.
	]


	value: arg1 value: arg2 value: arg3 [
		<primitive: BlockValuePrimitive3>
		OutOfRangeError signal.
	]


	valueWithArguments: anArray [
		<primitive: BlockValueArgsPrimitive>
		OutOfRangeError signal.
	]


	"controlling"

	whileTrue: aBlock [
		<primitive: BlockWhileTrue2Primitive>
		Error signal: self printString, ' must return a ', Boolean printString.
	]


	whileFalse: aBlock [
		[self value not] whileTrue: aBlock.
	]


	whileTrue [
		<primitive: BlockWhileTruePrimitive>
		Error signal: self printString, ' must return a ', Boolean printString.
	]


	whileFalse [
		[self value not] whileTrue.
	]


	repeat [
		[self value. true] whileTrue.
	]


	on: anException do: aBlock [
		<primitive: BlockOnExceptionPrimitive>
	]


	on: anException do: aBlock on: anException2 do: aBlock2 [
		^[self on: anException do: aBlock] on: anException2 do: aBlock2.
	]


	on: anException do: aBlock on: anException2 do: aBlock2 on: anException3 do: aBlock3 [
		^[[self on: anException do: aBlock] on: anException2 do: aBlock2] on: anException3 do: aBlock3.
	]


	"schedulling"

	fork [
		Error signal.
	]


	newProcess [
		Error signal.
	]

]
