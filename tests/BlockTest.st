Object subclass: BlockTest [

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
