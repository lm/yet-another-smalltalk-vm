PositionableStream := Stream [

	"accessing"

	peek [
		SubClassResponsibility signal.
	]


	peekFor: anObject [
		self peek = anObject ifTrue: [
			self next.
			^true].
		^false
	]


	upTo: anObject [
		| stream object |

		stream := CollectionStream with: (String new: 100).

		[self atEnd or: [(object := self next) = anObject]] whileFalse: [
			stream nextPut: object].
		^stream contents
	]


	upToAll: aCollection [
		| result prefix ch j resultColl |

		self atEnd ifTrue: [^self species new].
		aCollection isEmpty ifTrue: [^self species new].
		result := CollectionStream with: (self species new: 32).

		"Use the Knuth-Morris-Pratt algorithm."
		prefix := self prefixTableFor: aCollection.
		ch := self next.
		j := 1.
		result nextPut: ch.

		[
			(ch = (aCollection at: j) or: [(j := prefix at: j) = 0]) ifTrue: [
				j := j + 1.
				j > prefix size ifTrue: [
					"TODO: CollectionStream#contents returns whole contents not just up to pointer
					result skip: aCollection size negated."
					resultColl := result contents.
					^resultColl copyFrom: 1 to: resultColl size - aCollection size
				].
				self atEnd ifTrue: [^result contents].
				ch := self next.
				result nextPut: ch.
			].
		] repeat.
	]


	"private"

	prefixTableFor: aCollection [
		| prefix j |

		prefix := aCollection size < 256
			ifTrue: [ByteArray new: aCollection size]
			ifFalse: [Array new: aCollection size].
		prefix
			at: 1 put: 0;
			at: 2 put: 1.

		2 to: aCollection size - 1 do: [ :i | | ch |
			ch := aCollection at: i.
			j := prefix at: i.
			[j <= 1 or: [ch = (aCollection at: j)]] whileFalse: [j := prefix at: j].
			prefix at: i + 1 put: j.
		].

		^prefix
	]


	"positioning"

	position [
		SubClassResponsibility signal.
	]


	position: anInteger [
		SubClassResponsibility signal.
	]


	reset [
		SubClassResponsibility signal.
	]


	setToEnd [
		SubClassResponsibility signal.
	]


	skip: anInteger [
		self position: self position + anInteger.
	]


	skipTo: anObject [
		SubClassResponsibility signal.
	]


	"testing"

	isEmpty [
		SubClassResponsibility signal.
	]

]
