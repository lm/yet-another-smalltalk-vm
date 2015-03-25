IoError := Error [

	"instance creation"

	class last [
		<primitive: LastIoErrorPrimitive>
	]


	appendName: aString [
		| stream |

		stream := CollectionStream with: (String new: self messageText size + 3 + aString size).
		stream nextPutAll: self messageText;
			nextPut: Character space.
		aString printOn: stream.
		self messageText: stream contents.
	]

]
