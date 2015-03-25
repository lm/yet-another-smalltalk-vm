OuterReturnTest := Object [

	class createBlock [
		^[^1]
	]


	class outerReturn [
		[^1].
		[[^4] value. ^2] value.
		^3
	]

	class outerReturn2 [
		true ifTrue: [^1].
		^2
	]

	class outerReturn3 [
		true ifFalse: [^1].
		^2
	]
]


[
	Assert do: [OuterReturnTest createBlock value] expect: Error.
	Assert true: OuterReturnTest outerReturn = 4.
	Assert true: OuterReturnTest outerReturn2 = 1.
	Assert true: OuterReturnTest outerReturn3 = 2.
]
