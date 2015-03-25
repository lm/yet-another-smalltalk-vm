InternetAddress := Object [

	| address |


	"instance creation"

	class lookup: aString [
		<primitive: SocketHostLookup>
		Error signal.
	]


	"printing"

	printOn: aStream [
		(address bitAnd: 16rFF) printOn: aStream.
		aStream nextPut: $..
		((address bitAnd: 16rFF00) bitShift: -8) printOn: aStream.
		aStream nextPut: $..
		((address bitAnd: 16rFF0000) bitShift: -16) printOn: aStream.
		aStream nextPut: $..
		((address bitAnd: 16rFF000000) bitShift: -24) printOn: aStream.
	]

]
