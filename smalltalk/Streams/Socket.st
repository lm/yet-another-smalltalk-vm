Socket := ExternalStream [

	"instance creation"

	class connectTo: address port: port [
		^self new initializeAddress: address port: port
	]


	class connect: address port: port [
		<primitive: SocketConnectPrimitive>
	]


	"initialization"

	initializeAddress: address port: port [
		self initializeDescriptor: (self class connect: address port: port).
	]


	"accessing private"

	directNext: anInteger into: aCollection startingAt: start [
		| size |

		size := ((self class available: descriptor) min: anInteger) max: 1.
		^self class read: descriptor next: size into: aCollection startingAt: start
	]


	"flushing"

	flush [

	]


	"testing"

	atEnd [
		^false
	]

]
