MethodContext := Context [

	"accessing"

	method [
		^self code
	]


	"error handling"

	cannotReturn: anObject [
		Error signal: 'return from death context'.
	]

]

