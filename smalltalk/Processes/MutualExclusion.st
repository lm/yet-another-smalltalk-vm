Object subclass: MutualExclusion [


	<shape: MutexShape>


	class new [
		^super new initialize
	]


	initialize [
		<primitive: MutexInitPrimitive>
	]


	lock [
		<primitive: MutexLockPrimitive>
	]


	unlock [
		<primitive: MutexUnlockPrimitive>
	]


	finalize [
		<primitive: MutexFreePrimitive>
	]


	copy [
		^super copy initialize
	]

]
