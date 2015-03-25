MessageNotUnderstood := Error [

	| receiver message |


	"accessing"

	receiver: anObject [
		receiver := anObject.
	]


	receiver [
		^receiver
	]


	message: aMessage [
		message := aMessage.
	]


	message [
		^message
	]


	defaultMessageText [
		^receiver printString, ' did not understand ', message selector printString
	]

]
