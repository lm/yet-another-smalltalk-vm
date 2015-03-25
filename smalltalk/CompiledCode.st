CompiledCode := Object [

	<shape: CompiledCodeShape>

	| header |


	"accessing"

	header [
		^header
	]


	argumentsSize [
		^(header bitShift: -6) bitAnd: 16rFF
	]


	temporariesSize [
		^(header bitShift: -14) bitAnd: 16rFF
	]


	hasContext [
		^((header bitShift: -22) bitAnd: 16rFF) = 1
	]


	outerReturns [
		^((header bitShift: -36) bitAnd: 16rFF) = 1
	]


	primitive [
		| primitive |
		primitive := (header bitShift: -46) bitAnd: 16rFFFF.
		^primitive = 0 ifTrue: [] ifFalse: [primitive - 1]
	]


	sourceNode [
		^(Parser parseString: self sourceCode sourceContents) parseMethodOrBlock
	]


	temporaries [
		^self sourceNode temporaries collect: [ :temporary | temporary name]
	]


	arguments [
		^self sourceNode arguments collect: [ :argument | argument name]
	]



	"printing"

	printInstructionsOn: aStream [
		| insts |

		insts := CollectionStream on: self.
		[insts atEnd] whileFalse: [
			aStream nextPut: $<.
			insts position printOn: aStream.
			aStream nextPut: $>; nextPut: Character tab.
			self printInstruction: insts on: aStream].
	]


	printInstruction: anInstStream on: aStream [
		| inst |

		inst := anInstStream next.

		inst = 0 ifTrue: [
			aStream nextPutAll: 'copy: '.
			self printOperand: anInstStream on: aStream.
			aStream nextPutAll: ' to: '.
			self printOperand: anInstStream on: aStream].

		(inst = 1 or: [inst = 2]) ifTrue: [ | args |
			aStream nextPutAll: 'send: '.
			(self literals at: anInstStream next + 1) printOn: aStream.
			args := anInstStream next.
			aStream nextPutAll: ' to: '.
			self printOperand: anInstStream on: aStream.
			args timesRepeat: [
				aStream nextPutAll: ' arg: '.
				self printOperand: anInstStream on: aStream].
			inst = 2 ifTrue: [
				aStream nextPutAll: ' storeIn: '.
				self printOperand: anInstStream on: aStream]].

		inst = 3 ifTrue: [
			aStream nextPutAll: 'return: '.
			self printOperand: anInstStream on: aStream].

		inst = 4 ifTrue: [
			aStream nextPutAll: 'returnToOuter: '.
			self printOperand: anInstStream on: aStream].

		aStream lf.
	]


	printOperand: anInstStream on: aStream [
		| operand |

		operand := anInstStream next.
		operand = 0 ifTrue: [
			8 timesRepeat: [anInstStream next printOn: aStream].
			^anInstStream].

		operand = 1 ifTrue: [
			^aStream nextPutAll: 'nil'].

		operand = 2 ifTrue: [
			^aStream nextPutAll: 'true'].

		operand = 3 ifTrue: [
			^aStream nextPutAll: 'false'].

		operand = 4 ifTrue: [
			^aStream nextPutAll: 'thisContext'].

		operand = 5 ifTrue: [
			aStream nextPutAll: 'tmp#'.
			^anInstStream next printOn: aStream].

		operand = 6 ifTrue: [ | index |
			index := anInstStream next.
			index = 1
				ifTrue: [
					aStream nextPutAll: 'self']
				ifFalse: [
					aStream nextPutAll: 'arg#'.
					index - 1 printOn: aStream]].

		operand = 7 ifTrue: [
			^aStream nextPutAll: 'super'].

		operand = 8 ifTrue: [
			aStream nextPutAll: 'ctx#'.
			anInstStream next printOn: aStream.
			aStream nextPut: $[.
			anInstStream next printOn: aStream.
			^aStream nextPut: $]].

		operand = 9 ifTrue: [
			aStream nextPutAll: 'inst#'.
			^anInstStream next printOn: aStream].

		operand = 10 ifTrue: [
			aStream nextPutAll: 'inst#'.
			anInstStream next printOn: aStream,
			aStream nextPutAll: 'of '.
			^self printOperand: anInstStream on: aStream].

		(operand = 11 or: [operand = 12 or: [operand = 13]]) ifTrue: [
			(self literals at: anInstStream next + 1) printOn: aStream.
			^anInstStream].
	]

]
