[
	| collection expected |

	collection := OrderedCollection new.
	collection add: 1; add: 2; add: 3.
	Assert true: (collection removeIndex: 2) = 2.
	Assert true: collection = (OrderedCollection new add: 1; add: 3; yourself).

	collection := OrderedCollection new.
	collection add: 1; add: 2; add: 3.
	Assert true: (collection removeIndex: 1) = 1.
	Assert true: collection = (OrderedCollection new add: 2; add: 3; yourself).

	collection := OrderedCollection new.
	collection add: 1; add: 2; add: 3.
	Assert true: (collection removeIndex: 3) = 3.
	Assert true: collection = (OrderedCollection new add: 1; add: 2; yourself).

	collection := OrderedCollection new.
	collection add: 1; add: 2; add: 3; add: 4; add: 5; add: 6.

	expected := OrderedCollection new.
	expected add: 1; add: 2; add: 7; add: 3; add: 4; add: 5; add: 6.
	collection add: 7 beforeIndex: 3.
	Assert true: collection = expected.

	expected := OrderedCollection new.
	expected add: 1; add: 2; add: 7; add: 3; add: 8; add: 4; add: 5; add: 6.
	collection add: 8 afterIndex: 4.
	Assert true: collection = expected.

	Assert do: [OrderedCollection new last] expect: Error.
]

[
	Assert true: (OrderedCollection new add: 1; add: 2; add: 3; yourself) asArray = #(1 2 3).
]
