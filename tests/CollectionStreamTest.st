[
	| stream |

	stream := CollectionStream on: 'abbccdd'.

	Assert true: stream peek = $a.
	Assert true: stream peek = $a.

	Assert true: stream next = $a.
	Assert true: (stream next: 2) = 'bb'.
	Assert true: (stream next: 2) = 'cc'.
	Assert true: stream next = $d.
	Assert do: [stream next: 2] expect: OutOfRangeError.

	stream := CollectionStream on: 'foo' copy.
	Assert true: stream contents = 'foo' copy.
	Assert true: stream upToEnd = 'foo' copy.

	stream next.
	Assert true: stream contents = 'foo' copy.
	Assert true: stream upToEnd = 'oo' copy.

	stream := CollectionStream with: (String new: 1).
	stream nextPut: $a; nextPut: $a.
	stream nextPutAll: 'bb'.
	Assert true: stream contents = 'aabb' copy.

	stream := CollectionStream on: 'abc' copy.
	stream next; nextPut: $x.
	Assert true: stream contents = 'axc' copy.
	stream nextPutAll: 'xx'.
	Assert true: stream contents = 'axxx' copy.

	stream := CollectionStream with: 'abc' copy.
	Assert true: stream atEnd.

	stream := CollectionStream on: 'abc' copy.
	Assert false: stream atEnd.
	stream next: 2.
	Assert false: stream atEnd.
	stream next.
	Assert true: stream atEnd.
]
