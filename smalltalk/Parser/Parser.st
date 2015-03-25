Parser := Object [

	| stream source atEnd |


	class parseString: aString [
		^self new initializeString: aString
	]


	class parseStream: aStream [
		^(aStream isKindOf: CollectionStream)
			ifTrue: [self new initializeString: aStream contents]
			ifFalse: [self new initializeStream: aStream]
	]


	initializeStream: aStream [
		stream := aStream.
		source := aStream name.
		atEnd := false.
	]


	initializeString: aString [
		source := aString.
		atEnd := false.
	]


	parseClass [
		^self signalIfException: [self basicParseClass]
	]


	basicParseClass [
		<primitive: ParseClassPrimitive>
	]


	parseMethod [
		^self signalIfException: [self basicParseMethod]
	]


	basicParseMethod [
		<primitive: ParseMethodPrimitive>
	]


	parseMethodOrBlock [
		^self signalIfException: [self basicParseMethodOrBlock]
	]


	basicParseMethodOrBlock [
		<primitive: ParseMethodOrBlockPrimitive>
	]


	signalIfException: aBlock [
		| result |

		result := aBlock value.
		(result isKindOf: Exception) ifTrue: [result signal].
		^result
	]


	atEnd [
		^atEnd
	]

]
