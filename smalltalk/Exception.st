Exception := Object [

	| messageText |


	"instance creation"

	class signal [
		self new signal.
	]


	class signal: aString [
		self new
			messageText: aString;
			signal.
	]


	"exception handling"

	class handles: anException [
		^anException isKindOf: self
	]


	signal [
		<primitive: ExceptionSignal>
		self defaultAction.
	]


	defaultAction [
		Transcript nextPutAll: self messageText; lf.
		self signalingContext printBacktrace.
		Processor thisProcess terminate.
	]


	"backtrace"

	generateBacktrace [
		| context backtrace |

		backtrace := OrderedCollection new: 8.
		context := self signalingContext.

		[backtrace add: context copy.
		context := context parent.
		context notNil]
			whileTrue.

		^backtrace
	]


	signalingContext [
		| context |

		context := thisContext.
		[(context code ownerClass == Exception or: [context code ownerClass inheritsFrom: Exception])
		or: [context code ownerClass == Exception class or: [context code ownerClass inheritsFrom: Exception class]]]
			whileTrue: [context := context parent].

		^context
	]


	"accessing"

	messageText: aString [
		messageText := aString.
	]


	messageText [
		^messageText isNil ifTrue: [self defaultMessageText] ifFalse: [messageText]
	]


	defaultMessageText [
		^self class name
	]

]
