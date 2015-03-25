Behavior := Object [

	| superClass subClasses methodDictionary instanceShape instanceVariables |


	"instance creation"

	new [
		<primitive: BehaviorNewPrimitive>
	]


	new: anInteger [
		<primitive: BehaviorNewSizePrimitive>
		(anInteger isMemberOf: SmallInteger) ifFalse: [Error signal: 'size must be SmallInteger'].
		Error signal.
	]


	basicNew [
		<primitive: BehaviorNewPrimitive>
	]


	basicNew: anInteger [
		<primitive: BehaviorNewSizePrimitive>
		Error signal.
	]


	"shape and size"

	instanceSize [
		^(instanceShape bitShift: -14) bitAnd: 16rFF
	]


	instanceShape [
		^instanceShape
	]


	isIndexable [
		^((instanceShape bitShift: -22) bitAnd: 16rFF) = 1
	]


	instanceVariables [
		^instanceVariables isNil ifTrue: [#()] ifFalse: [instanceVariables]
	]


	"accessing class hierarchy"

	superClass [
		^superClass
	]


	subClasses [
		^subClasses
	]


	methodDictionary [
		^methodDictionary
	]


	"testing class hierarchy"

	inheritsFrom: aClass [
		| aSuperClass |

		aSuperClass := superClass.
		[aSuperClass == nil] whileFalse: [
			aSuperClass == aClass ifTrue: [^true].
			aSuperClass := aSuperClass superClass].
		^false
	]


	"accessing the method dictionary"

	selectors [
		^methodDictionary keys
	]


	allSelectors [
		| selectors |

		selectors := self selectors.
		self allSuperClassesDo: [ :class | selectors addAll: class selectors].
		^selectors
	]


	lookupSelector: aSymbol [
		| class |

		class := self.
		[class methodDictionary at: aSymbol ifPresent: [ :code | ^code].
		(class := class superClass) isNil] whileFalse.
		^nil
	]


	compiledMethodAt: aSymbol [
		^methodDictionary at: aSymbol
	]


	"testing method dictionary"

	canUnderstand: aSymbol [
		(self includesSelector: aSymbol) ifTrue: [^true].
		superClass == nil ifTrue: [^false].
		^superClass canUnderstand: aSymbol
	]


	includesSelector: aSymbol [
		^methodDictionary includesKey: aSymbol
	]


	"enumerating"

	allSuperClassesDo: aBlock [
		| class |

		class := self.
		[(class := class superClass) notNil] whileTrue: [
			aBlock value: class].
	]


	allSubClassesDo: aBlock [
		self subClasses do: [ :aClass |
			aBlock value: aClass.
			aClass allSubClassesDo: aBlock].
	]


	"printing"

	printHierarchy [
		Transcript nextPutAll: self name; lf.
		self printSubClassesOn: Transcript level: 1.
	]


	printSubClassesOn: aStream level: anInteger [
		self subClasses do: [ :aClass |
			Transcript
				next: anInteger put: Character space;
				nextPutAll: aClass name;
				lf.
			aClass printSubClassesOn: aStream level: anInteger + 1].
	]

]
