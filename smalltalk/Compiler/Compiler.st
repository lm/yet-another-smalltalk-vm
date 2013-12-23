Object subclass: Compiler [

	includeFile: aString [
		| stream parser classes |
		stream := FileStream read: aString.
		parser := Parser parseStream: stream.
		classes := OrderedCollection new.
		[parser atEnd] whileFalse: [
			classes add: (self buildClass: parser parseClass)].
		classes do: [ :class | class initialize].
		stream close.
	]


	buildClass: aClassNode [
		| result |

		result := self basicBuildClass: aClassNode.
		(result isKindOf: Exception) ifTrue: [result signal].
		^result
	]


	basicBuildClass: aClassNode [
		<primitive: BuildClassPrimitive>
	]


	compileMethod: aMethodNode in: aClass [
		| result |

		result := self basicCompileMethod: aMethodNode in: aClass.
		(result isKindOf: Exception) ifTrue: [result signal].
		^result
	]


	basicCompileMethod: aMethodNode in: aClass [
		<primitive: CompileMethodPrimitive>
	]


	evaluate: aString [
		| node method |

		node := (Parser parseString: 'eval [', aString, ']') parseMethod.
		node body expressions isEmpty ifFalse: [
			node body expressions last enableReturn].
		method := self compileMethod: node in: UndefinedObject.
		^method sendTo: nil
	]

]
