ClassNode := Object [

	| name superName pragmas variables methods sourceCode |


	name [
		^name
	]


	superName [
		^superName
	]


	variables [
		^variables
	]


	methods [
		^methods
	]


	sourceCode [
		^sourceCode
	]


	acceptVisitor: aVisitor [
		aVisitor visitClassNode: self.
		name acceptVisitor: aVisitor.
		superName acceptVisitor: aVisitor.
		pragmas do: [ :pragma | pragma acceptVisitor: aVisitor].
		variables do: [ :var | var acceptVisitor: aVisitor].
		methods do: [ :method | method acceptVisitor: aVisitor].
	]

]
