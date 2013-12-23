SourceCode subclass: FileSourceCode [

	"accessing"

	sourceContents [
		| stream |

		stream := FileStream read: self source.
		stream position: self position.
		^stream next: self sourceSize
	]

]
