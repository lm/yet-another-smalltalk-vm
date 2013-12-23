[
	| array |

	array := Array new: 2.

	Assert true: array size = 2.
	Assert true: (array at: 1) isNil.
	Assert true: (array at: 2) isNil.
	Assert true: (array at: 1 put: 3) = 3.
]

[
	Assert true: (Array with: 1 with: 2) = (Array with: 1 with: 2).
	Assert true: (Array with: 1 with: 2) ~= (Array with: 1 with: 3).

	Assert true: (Array with: 1 with: 2) hash = (Array with: 1 with: 2) hash.
	Assert true: (Array with: 1 with: 2) hash ~= (Array with: 1 with: 3) hash.
]
