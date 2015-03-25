Set := HashedCollection [

	"instance creation"

	class new [
		^self new: 31
	]


	"accessing"

	add: anObject [
		| index |

		anObject isNil ifTrue: [^anObject].
		index := self findIndex: anObject.
		(contents at: index) isNil ifTrue: [self atNewIndex: index put: anObject].
		^anObject
	]


	"private"

	findIndex: anObject [
		| size index object |

		size := contents size.
		index := (anObject hash bitAnd: size - 1) + 1.

		[object := contents at: index.
		(object isNil or: [object = anObject]) ifTrue: [^index].
		index := index == size ifTrue: [1] ifFalse: [index + 1]]
			repeat.
	]

]
