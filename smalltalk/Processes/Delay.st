Delay := Object [

	| microseconds |


	class forMicroseconds: anInteger [
		^super new initMicroseconds: anInteger
	]


	class forMilliseconds: anInteger [
		^self forMicroseconds: anInteger * 1000
	]


	class forSeconds: anInteger [
		^self forMicroseconds: anInteger * 1000000
	]


	initMicroseconds: anInteger [
		microseconds := anInteger.
	]


	wait [
		<primitive: DelayWaitPrimitive>
	]

]
