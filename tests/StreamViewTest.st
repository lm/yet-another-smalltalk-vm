[
	| stream view |

	stream := CollectionStream on: 'abcdef'.
	view := StreamView on: stream limit: 3.

	Assert true: view peek = $a.
	Assert true: view next = $a.
	Assert true: (view next: 2) = 'bc'.
	Assert do: [view next] expect: Error.
	Assert do: [view next: 2] expect: Error.
	Assert true: view peek = nil.

	stream := CollectionStream on: 'abcdef'.
	view := StreamView on: stream limit: 3.
	Assert true: view contents = 'abc'.

	stream := CollectionStream on: 'abcdef'.
	view := StreamView on: stream limit: 3.
	view position: 2.
	Assert true: view next = $b.
	view position: 3.
	Assert do: [view position: 4] expect: Error.

	"stream := CollectionStream on: 'abcdef'.
	view := StreamView on: stream limit: 3.
	view next: 2.
	view reset.
	Assert true: view next = $a."

	stream := CollectionStream on: 'abcdef'.
	view := StreamView on: stream limit: 3.
	view skip: 2.
	Assert true: view next = $c.
	view skip: -3.
	Assert true: view next = $a.
]
