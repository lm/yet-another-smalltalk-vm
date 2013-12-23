Object subclass: RBObject [

	"Examples-Richards"

	"| DeviceA DeviceB DevicePacketKind HandlerA HandlerB Idler Worker WorkPacketKind |"

	"initialize"

	class initialize [
		DeviceA := 5.
		DeviceB := 6.
		DevicePacketKind := 1.
		HandlerA := 3.
		HandlerB := 4.
		Idler := 1.
		Worker := 2.
		WorkPacketKind := 2
	]


	"utilities"

	append: packet head: queueHead [
		| mouse link |
		packet link: nil.
		queueHead isNil ifTrue: [^packet].
		mouse := queueHead.
		[(link := mouse link) isNil] whileFalse: [mouse := link].
		mouse link: packet.
		^queueHead
	]
]


RBObject subclass: Packet [

	| link identity kind datum data |

	"Examples-Richards"
	"instance creation"

	class create: link identity: identity kind: kind [
		^super new
			link: link
			identity: identity
			kind: kind
	]


	"initialize"

	link: aLink identity: anIdentity kind: aKind [
		link := aLink.
		identity := anIdentity.
		kind := aKind.
		datum := 1.
		data := ByteArray new: 4
	]


	"accessing"

	data [
		^data
	]


	datum [
		^datum
	]


	datum: someData [
		datum := someData
	]


	identity [
		^identity
	]


	identity: anIdentity [
		identity := anIdentity
	]


	kind [
		^kind
	]


	link [
		^link
	]


	link: aWorkQueue [
		link := aWorkQueue
	]

]


RBObject subclass: RichardsBenchmark [
	| taskList currentTask currentTaskIdentity taskTable tracing layout queuePacketCount holdCount |

	"Examples-Richards"
	"instance creation"

	class report: string times: count run: aBlock [
		"Report the time required to execute the given block."

		| time |
		time := 0.
		count timesRepeat: [time := time + aBlock value].
		Transcript
			nextPutAll: string , ' ' , (time // count) printString , ' milliseconds';
			lf
	]


	class standardBenchmark [
		"This the combined benchmark."

		self
			report: 'Richards simulations'
			times: 100
			run: [self start]
	]


	class start: trace [
		^self new start: trace
	]


	class start [
		^self new start
	]


	"creation"

	createDevice: identity priority: priority work: work state: state [
		| data |
		data := DeviceTaskDataRecord create.
		self
			createTask: identity
			priority: priority
			work: work
			state: state
			function:
				[:work :word |
				| data functionWork |
				data := word.
				functionWork := work.
				functionWork isNil
					ifTrue:
						[(functionWork := data pending) isNil
							ifTrue: [self wait]
							ifFalse:
								[data pending: nil.
								self queuePacket: functionWork]]
					ifFalse:
						[data pending: functionWork.
						tracing ifTrue: [self trace: functionWork datum].
						self holdSelf]]
			data: data
	]


	createHandler: identity priority: priority work: work state: state [
		| data |
		data := HandlerTaskDataRecord create.
		self
			createTask: identity
			priority: priority
			work: work
			state: state
			function:
				[:work :word |
				| data workPacket count devicePacket |
				data := word.
				work isNil
					ifFalse:
						[WorkPacketKind == work kind
							ifTrue: [data workInAdd: work]
							ifFalse: [data deviceInAdd: work]].
				(workPacket := data workIn) isNil
					ifTrue: [self wait]
					ifFalse:
						[count := workPacket datum.
						count > 4
							ifTrue:
								[data workIn: workPacket link.
								self queuePacket: workPacket]
							ifFalse:
								[(devicePacket := data deviceIn) isNil
									ifTrue: [self wait]
									ifFalse:
										[data deviceIn: devicePacket link.
										devicePacket datum: (workPacket data at: count).
										workPacket datum: count + 1.
										self queuePacket: devicePacket]]]]
			data: data
	]


	createIdler: identity priority: priority work: work state: state [
		| data |
		data := IdleTaskDataRecord create.
		self
			createTask: identity
			priority: priority
			work: work
			state: state
			function:
				[:work :word |
				| data |
				data := word.
				data count: data count - 1.
				0 = data count
					ifTrue: [self holdSelf]
					ifFalse:
						[0 = (data control bitAnd: 1)
							ifTrue:
								[data control: data control // 2.
								self release: DeviceA]
							ifFalse:
								[data control: (data control // 2 bitXor: 53256).
								self release: DeviceB]]]
			data: data
	]


	createPacket: link identity: identity kind: kind [
		^Packet
			create: link
			identity: identity
			kind: kind
	]


	createTask: identity priority: priority work: work state: state function: aBlock data: data [
		| t |
		t := TaskControlBlock
					link: taskList
					create: identity
					priority: priority
					initialWorkQueue: work
					initialState: state
					function: aBlock
					privateData: data.
		taskList := t.
		taskTable at: identity put: t
	]


	createWorker: identity priority: priority work: work state: state [
		| data |
		data := WorkerTaskDataRecord create.
		self
			createTask: identity
			priority: priority
			work: work
			state: state
			function:
				[:work :word |
				| data |
				data := word.
				work isNil
					ifTrue: [self wait]
					ifFalse:
						[data destination: (HandlerA = data destination
									ifTrue: [HandlerB]
									ifFalse: [HandlerA]).
						work identity: data destination.
						work datum: 1.
						1 to: 4
							do:
								[:i |
								data count: data count + 1.
								data count > 26 ifTrue: [data count: 1].
								work data at: i put: $A codePoint + data count - 1].
						self queuePacket: work]]
			data: data
	]


	"private"

	findTask: identity [
		| t |
		t := taskTable at: identity.
		t isNil ifTrue: [self error: 'findTask failed'].
		^t
	]


	holdSelf [
		holdCount := holdCount + 1.
		currentTask taskHolding: true.
		^currentTask link
	]


	initScheduler [
		queuePacketCount := holdCount := 0.
		taskTable := Array new: 6.
		taskList := nil
	]


	initTrace: trace [
		tracing := trace.
		layout := 0
	]


	queuePacket: packet [
		| t |
		t := self findTask: packet identity.
		t isNil ifTrue: [^nil].
		queuePacketCount := queuePacketCount + 1.
		packet link: nil.
		packet identity: currentTaskIdentity.
		^t addInput: packet checkPriority: currentTask
	]


	release: identity [
		| t |
		t := self findTask: identity.
		t isNil ifTrue: [^nil].
		t taskHolding: false.
		t priority > currentTask priority ifTrue: [^t] ifFalse: [^currentTask]
	]


	trace: id [
		layout := layout - 1.
		0 >= layout
			ifTrue:
				[Transcript cr.
				layout := 50].
		Transcript show: id printString
	]


	wait [
		currentTask taskWaiting: true.
		^currentTask
	]


	"scheduling"

	schedule [
		currentTask := taskList.
		[currentTask isNil] whileFalse:
				[currentTask isTaskHoldingOrWaiting
					ifTrue: [currentTask := currentTask link]
					ifFalse:
						[currentTaskIdentity := currentTask identity.
						tracing ifTrue: [self trace: currentTaskIdentity].
						currentTask := currentTask runTask]]
	]


	"initialize"

	start [
		^self start: false
	]


	start: trace [
		| workQ mark1 mark2 mark3 mark4 |

		self
			initTrace: trace;
			initScheduler.
		mark1 := DateTime currentMicroTime // 1000.
		tracing
			ifTrue:
				[Transcript
					show: 'Bench mark starting';
					cr].
		self
			createIdler: Idler
			priority: 0
			work: nil
			state: TaskState running.
		workQ := self
					createPacket: nil
					identity: Worker
					kind: WorkPacketKind.
		workQ := self
					createPacket: workQ
					identity: Worker
					kind: WorkPacketKind.
		self
			createWorker: Worker
			priority: 1000
			work: workQ
			state: TaskState waitingWithPacket.
		workQ := self
					createPacket: nil
					identity: DeviceA
					kind: DevicePacketKind.
		workQ := self
					createPacket: workQ
					identity: DeviceA
					kind: DevicePacketKind.
		workQ := self
					createPacket: workQ
					identity: DeviceA
					kind: DevicePacketKind.
		self
			createHandler: HandlerA
			priority: 2000
			work: workQ
			state: TaskState waitingWithPacket.
		workQ := self
					createPacket: nil
					identity: DeviceB
					kind: DevicePacketKind.
		workQ := self
					createPacket: workQ
					identity: DeviceB
					kind: DevicePacketKind.
		workQ := self
					createPacket: workQ
					identity: DeviceB
					kind: DevicePacketKind.
		self
			createHandler: HandlerB
			priority: 3000
			work: workQ
			state: TaskState waitingWithPacket.
		self
			createDevice: DeviceA
			priority: 4000
			work: nil
			state: TaskState waiting.
		self
			createDevice: DeviceB
			priority: 5000
			work: nil
			state: TaskState waiting.
		tracing
			ifTrue:
				[Transcript
					show: 'Starting';
					cr].
		mark2 := DateTime currentMicroTime // 1000.
		self schedule.
		mark3 := DateTime currentMicroTime // 1000.
		tracing
			ifTrue:
				[Transcript
					show: 'Finished';
					cr.
				Transcript
					show: 'QueuePacket count = ';
					show: queuePacketCount printString;
					show: ' HoldCount = ';
					show: holdCount printString;
					cr.
				Transcript
					cr;
					show: 'End of run';
					cr].
		queuePacketCount = 23246 & (holdCount = 9297)
			ifFalse: [self error: 'wrong result'].
		mark4 := DateTime currentMicroTime // 1000.
		tracing
			ifTrue:
				[Transcript
					show: '***Scheduler time = ';
					show: (mark3 - mark2) printString;
					show: ' Total time = ';
					show: (mark4 - mark1) printString;
					cr].
		^mark3 - mark2
	]

]


RBObject subclass: TaskState [

	| packetPendingIV taskWaiting taskHolding |

	"Examples-Richards"
	"instance creation"

	class packetPending [
		^super new packetPending
	]


	class running [
		^super new running
	]


	class waiting [
		^super new waiting
	]


	class waitingWithPacket [
		^super new waitingWithPacket
	]


	"initialize"

	packetPending [
		packetPendingIV := true.
		taskWaiting := false.
		taskHolding := false
	]


	running [
		packetPendingIV := taskWaiting := taskHolding := false
	]


	waiting [
		packetPendingIV := taskHolding := false.
		taskWaiting := true
	]


	waitingWithPacket [
		taskHolding := false.
		taskWaiting := packetPendingIV := true
	]


	"accessing"

	isPacketPending [
		^packetPendingIV
	]


	isTaskHolding [
		^taskHolding
	]


	isTaskWaiting [
		^taskWaiting
	]


	taskHolding: aBoolean [
		taskHolding := aBoolean
	]


	taskWaiting: aBoolean [
		taskWaiting := aBoolean
	]


	"testing"

	isRunning [
		^packetPendingIV not and: [taskWaiting not and: [taskHolding not]]
	]


	isTaskHoldingOrWaiting [
		^taskHolding or: [packetPendingIV not and: [taskWaiting]]
	]


	isWaiting [
		^packetPendingIV not and: [taskWaiting and: [taskHolding not]]
	]


	isWaitingWithPacket [
		^packetPendingIV and: [taskWaiting and: [taskHolding not]]
	]

]


RBObject subclass: DeviceTaskDataRecord [

	| pending |

	"Examples-Richards"
	"instance creation"

	class create [
		^super new create
	]


	"initialize"

	create [
		pending := nil
	]


	"accessing"

	pending [
		^pending
	]


	pending: packet [
		pending := packet
	]

]


RBObject subclass: HandlerTaskDataRecord [

	| workIn deviceIn |

	"Examples-Richards"
	"instance creation"

	class create [
		^super new create
	]


	"initialize"

	create [
		<category: ''>
		workIn := deviceIn := nil
	]


	"accessing"

	deviceIn [
		^deviceIn
	]


	deviceIn: aPacket [
		deviceIn := aPacket
	]


	deviceInAdd: packet [
		deviceIn := self append: packet head: deviceIn
	]


	workIn [
		^workIn
	]


	workIn: aWorkQueue [
		workIn := aWorkQueue
	]


	workInAdd: packet [
		workIn := self append: packet head: workIn
	]

]


RBObject subclass: WorkerTaskDataRecord [

	| destination count |

	"Examples-Richards"
	"instance creation"

	class create [
		^super new create
	]


	"initialize"

	create [
		destination := HandlerA.
		count := 0
	]


	count [
		^count
	]


	count: aCount [
		count := aCount
	]


	destination [
		^destination
	]


	destination: aHandler [
		destination := aHandler
	]

]


TaskState subclass: TaskControlBlock [

	| link identity priority input state function handle |

	"Examples-Richards"
	"instance creation"

	class link: link create: identity priority: priority initialWorkQueue: initialWorkQueue initialState: initialState function: aBlock privateData: privateData [
		^super new
			link: link
			identity: identity
			priority: priority
			initialWorkQueue: initialWorkQueue
			initialState: initialState
			function: aBlock
			privateData: privateData
	]


	"initialize"

	link: aLink identity: anIdentity priority: aPriority initialWorkQueue: anInitialWorkQueue initialState: anInitialState function: aBlock privateData: aPrivateData [
		link := aLink.
		identity := anIdentity.
		priority := aPriority.
		input := anInitialWorkQueue.
		packetPendingIV := anInitialState isPacketPending.
		taskWaiting := anInitialState isTaskWaiting.
		taskHolding := anInitialState isTaskHolding.
		function := aBlock.
		handle := aPrivateData.
	]


	"accessing"

	identity [
		^identity
	]


	link [
		^link
	]


	priority [
		^priority
	]


	"scheduling"

	addInput: packet checkPriority: oldTask [
		input isNil
			ifTrue:
				[input := packet.
				packetPendingIV := true.
				priority > oldTask priority ifTrue: [^self]]
			ifFalse: [input := self append: packet head: input].
		^oldTask
	]


	runTask [
		| message |
		self isWaitingWithPacket
			ifTrue:
				[message := input.
				input := message link.
				input isNil ifTrue: [self running] ifFalse: [self packetPending]]
			ifFalse: [message := nil].
		^function value: message value: handle
	]

]


RBObject subclass: IdleTaskDataRecord [

	| control count |

	"Examples-Richards"
	"instance creation"

	class create [
		^super new create
	]


	"initialize"

	create [
		control := 1.
		count := 10000
	]


	"accessing"

	control [
		^control
	]


	control: aNumber [
		control := aNumber
	]


	count [
		^count
	]


	count: aCount [
		count := aCount
	]

]


[
	RichardsBenchmark standardBenchmark.
]
