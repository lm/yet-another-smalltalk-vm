Object subclass: Process [

	<shape: ProcessShape>

	| block |


	initializeBlock: aBlock [
		block := aBlock.
	]


	"changing process state"

	resume [
		<primitive: ProcessCreatePrimitive>
	]


	suspend [

	]


	terminate [
		<primitive: ExitPrimitive>
	]

]
