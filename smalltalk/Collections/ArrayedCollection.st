ArrayedCollection := SequenceableCollection [

	<shape: IndexedShape>


	"instance creation"

	class with: anObject [
		^(self new: 1) at: 1 put: anObject; yourself
	]


	class with: anObject with: anObject2 [
		^(self new: 2)
			at: 1 put: anObject;
			at: 2 put: anObject2;
			yourself
	]


	"accessing"

	size [
		^self basicSize
	]


	"adding"

	add: anObject [
		ShouldNotImplement signal.
	]

]
