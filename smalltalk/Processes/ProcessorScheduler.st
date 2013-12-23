Object subclass: ProcessorScheduler [

	class initialize [
		Processor := self new.
	]


	thisProcess [
		^Process new
	]

]
