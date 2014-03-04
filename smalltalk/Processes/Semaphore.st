Object subclass: Semaphore [

	| mutex mutex2 counter |


	"instance creation"

	class new [
		^super new initialize
	]

	class forMutualExclusion [
		^self new signal
	]


	"initializing"

	initialize [
		mutex := MutualExclusion new.
		mutex2 := mutex copy lock.
		counter := 0.
	]


	"communication"

	signal [
		mutex lock.
		counter := counter + 1.
		counter > 0 ifTrue: [mutex2 unlock].
		mutex unlock.
	]


	wait [
		mutex2 lock.
		mutex lock.
		counter := counter - 1.
		counter > 0 ifTrue: [mutex2 unlock].
		mutex unlock.
	]


	"mutual exclusion"

	critical: aBlock [
		self wait.
		aBlock value.
		self signal.
	]

]
