BlockTest := Object [

	class outerReturn [
		[^1] value.
	]

]

[
	Assert true: [] value isNil.
	Assert true: [1] value = 1.
	Assert true: BlockTest outerReturn = 1.
]

[
	Assert do: [[ :a | ] value] expect: OutOfRangeError.
	Assert do: [[ :a | ] value: 1 value: 2] expect: OutOfRangeError.
]

[
	| block |

	block := [false].
	Assert true: block whileTrue == block.
	Assert true: (block whileTrue: []) == block.
]

[
	Assert do: [[] whileTrue] expect: Error.
	Assert do: [[1] whileTrue] expect: Error.
	Assert do: [[Object] whileTrue] expect: Error.
	Assert do: [[] whileTrue: []] expect: Error.
	Assert do: [[1] whileTrue: []] expect: Error.
	Assert do: [[Object] whileTrue: []] expect: Error.
]
