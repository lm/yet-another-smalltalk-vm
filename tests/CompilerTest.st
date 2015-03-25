[
	| compiler |

	compiler := Compiler new.

	Assert true: (compiler evaluate: '1') == 1.
	Assert true: (compiler evaluate: '^1') == 1.
	Assert true: (compiler evaluate: 'self') == nil.
	Assert true: (compiler evaluate: '| a | a := 1. a') == 1.
	Assert do: [compiler evaluate: 'self foo'] expect: MessageNotUnderstood.
	Assert true: Foo isNil.
	compiler includeFile: 'tests/CompilerTestFile.st'.
	Assert true: Foo notNil.
]

CompilerTestClassA := Object [

	| ClassVar |


	defineClassVar [
		ClassVar := 1
	]

]


CompilerTestClassB := CompilerTestClassA [

	classVar [
		^ClassVar
	]

]


[
	| object |

	object := CompilerTestClassB new.
	Assert true: object classVar isNil.
	object defineClassVar.
	Assert true: object classVar = 1.
]
