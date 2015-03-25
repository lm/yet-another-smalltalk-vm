DateTime := Magnitude [

	| milliSeconds MillisecondsPerDay DaysIn4Years DaysIn100Years DaysIn400Years
		Days1970To2000 DaysOffset YearsOffset |

	class now [
		^self new initializeMilliSeconds: self currentMicroTime // 1000
	]


	class initialize [
		MillisecondsPerDay := 24 * 60 * 60 * 1000.
		DaysIn4Years := 4 * 365 + 1.
		DaysIn100Years := 25 * DaysIn4Years - 1.
		DaysIn400Years := 4 * DaysIn100Years + 1.
		Days1970To2000 := 30 * 365 + 7.
		DaysOffset := (1000 * DaysIn400Years) + (5 * DaysIn400Years) - Days1970To2000.
		YearsOffset := 400000.
	]


	class currentMicroTime [
		<primitive: CurrentMicroTimePrimitive>
	]


	initializeMilliSeconds: anInteger [
		milliSeconds := anInteger.
	]


	"accessing"

	second [
		^milliSeconds // 1000 \\ 60
	]


	minute [
		^milliSeconds // (1000 * 60) \\ 60
	]


	hour [
		^milliSeconds // (1000 * 60 * 60) \\ 24
	]


	day [
		^self computeDate at: 3
	]


	month [
		^self computeDate at: 2
	]


	year [
		^self computeDate at: 1
	]


	computeDate [
		| resultYear resultMonth days yd1 yd2 yd3 isLeap daysUntilMonth result |

		resultYear := 0.
		resultMonth := 0.

		days := milliSeconds // MillisecondsPerDay + DaysOffset.

		resultYear := 400 * (days // DaysIn400Years) - YearsOffset.

		days := days \\ DaysIn400Years - 1.
		yd1 := days // DaysIn100Years.
		days := days \\ DaysIn100Years.

		resultYear := resultYear + (100 * yd1).

		days := days + 1.
		yd2 := days // DaysIn4Years.
		days := days \\ DaysIn4Years.
		resultYear := resultYear + (4 * yd2).

		days := days - 1.
		yd3 := days // 365.
		days := days \\ 365.
		resultYear := resultYear + yd3.

		isLeap := (yd1 = 0 or: [yd2 ~= 0]) and: [yd3 = 0].
		isLeap ifTrue: [days := days + 1].

		daysUntilMonth := isLeap
			ifTrue: [#(0 31 60 91 121 152 182 213 244 274 305 335)]
			ifFalse: [#(0 31 59 90 120 151 181 212 243 273 304 334)].
		resultMonth := 12.
		[(daysUntilMonth at: resultMonth) > days] whileTrue: [resultMonth := resultMonth - 1].

		result := Array new: 3.
		result at: 1 put: resultYear.
		result at: 2 put: resultMonth.
		result at: 3 put: days - (daysUntilMonth at: resultMonth) + 1.
		^result
	]


	"printing"

	printOn: aStream [
		| date year month day |

		date := self computeDate.
		year := date at: 1.
		month := date at: 2.
		day := date at: 3.

		year printOn: aStream.
		aStream nextPut: $-.
		month < 10 ifTrue: [aStream nextPut: $0].
		month printOn: aStream.
		aStream nextPut: $-.
		day < 10 ifTrue: [aStream nextPut: $0].
		day printOn: aStream.
		aStream space.

		self hour < 10 ifTrue: [aStream nextPut: $0].
		self hour printOn: aStream.
		aStream nextPut: $:.
		self minute < 10 ifTrue: [aStream nextPut: $0].
		self minute printOn: aStream.
		aStream nextPut: $:.
		self second < 10 ifTrue: [aStream nextPut: $0].
		self second printOn: aStream.
	]

]
