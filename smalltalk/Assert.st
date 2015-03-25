Assert := Object [

	class true: aBool [
		aBool ifFalse: [AssertError expected: true got: aBool].
	]


	class false: aBool [
		aBool ifTrue: [AssertError expected: false got: aBool].
	]


	class do: aBlock expect: aClass [
		| signaled |

		[signaled := true.
		aBlock value.
		signaled := false] on: aClass do: [ :e | ].

		signaled ifFalse: [AssertError expected: aClass got: nil].
	]

]
