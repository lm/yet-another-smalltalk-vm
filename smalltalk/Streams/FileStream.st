FileStream := ExternalStream [

	| name |


	"instance creation"

	class read: aString [
		^self new initializeFileName: aString mode: 1
	]


	class write: aString [
		^self new initializeFileName: aString mode: 2
	]


	class readOrWrite: aString [
		^self new initializeFileName: aString mode: 3
	]


	"IO primitives"

	class open: aString mode: anInteger [
		<primitive: StreamOpenPrimitive>
		IoError last appendName: aString; signal.
	]


	initializeFileName: aString mode: anInteger [
		name := aString.
		self initializeDescriptor: (self class open: aString mode: anInteger).
	]


	"accessing"

	name [
		^name
	]

]
