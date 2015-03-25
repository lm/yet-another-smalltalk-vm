Context := Object [

	<shape: ContextShape>

	| ic code parent outer home |


	"accessing"

	parent [
		<primitive: ContextParentPrimitive>
	]


	ic [
		^ic
	]


	code [
		^code
	]


	outer [
		^outer
	]


	home [
		^home
	]


	line [
		^((self positionDescriptor bitShift: -30) bitAnd: 16rFFFF) + code sourceCode line
	]


	column [
		^(self positionDescriptor bitShift: -14) bitAnd: 16rFFFF
	]


	positionDescriptor [
		<primitive: ContextPositionDescriptorPrimitive>
	]


	size [
		ShouldNotImplement signal.
	]


	argumentAt: anInteger [
		<primitive: ContextArgumentAt>
	]


	arguments [
		| arguments |

		arguments := Array new: code argumentsSize.
		1 to: arguments size do: [ :i | arguments at: i put: (self argumentAt: i)].
		^arguments
	]


	temporaryAt: anInteger [
		<primitive: ContextTemporaryAt>
	]


	temporaries [
		| temporaries |

		temporaries := Array new: code temporariesSize.
		1 to: temporaries size do: [ :i | temporaries at: i put: (self temporaryAt: i)].
		^temporaries
	]


	receiver [
		^self argumentAt: 0
	]


	copy [
		^ContextCopy fromContext: self
	]


	"printing"

	printOn: aStream [
		self code printOn: aStream.
	]


	printBacktrace [
		self printBacktraceOn: Transcript.
	]


	printBacktraceOn: aStream [
		| context i |

		context := self.
		i := 1.

		[i printOn: aStream.
		aStream nextPutAll: '. '.
		context printOn: aStream.
		aStream
			nextPutAll: ' ... ';
			nextPutAll: context code sourceCode source;
			nextPut: $:.
		context line printOn: aStream.
		aStream nextPut: $:.
		context column printOn: aStream.
		aStream lf.
		context := context parent.
		i := i + 1.
		context isNil]
			whileFalse.
	]


	examineOn: aStream [
		super examineOn: aStream.
		aStream nextPutAll: ' receiver: '.
		self receiver printOn: aStream.
		aStream lf; nextPutAll: ' arguments: '.
		self arguments printOn: aStream.
		aStream lf; nextPutAll: ' temporaries: '.
		self temporaries printOn: aStream.
		aStream lf.
	]

]
