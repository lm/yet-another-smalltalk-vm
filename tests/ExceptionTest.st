ExceptionTest := Object [

	class outerReturnWithinHandleBlock [
		[Exception signal] on: Exception do: [ :e | ^e].
	]


	class outerReturnWithinBlock [
		[^1] on: Exception do: [ :e | ].
	]


	class outerReturnWithinBlockOfBlock [
		[[^1] value] on: Exception do: [ :e | ].
	]

]


[
	| exception |

	exception := Exception new.
	[exception signal] on: Exception do: [ :e | Assert true: e == exception].
	[Error signal] on: Exception do: [ :e | Assert true: e ~= exception].
]


[
	Assert true: (ExceptionTest outerReturnWithinHandleBlock isMemberOf: Exception).
	Assert true: ExceptionTest outerReturnWithinBlock = 1.
	Assert true: ExceptionTest outerReturnWithinBlockOfBlock = 1.
]
