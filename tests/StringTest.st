[
	Assert true:
		('  abc gh i  ' splitBy: Character space) = (OrderedCollection with: 'abc' with: 'gh' with: 'i').
	Assert true:
		('2abc1gh3i1' splitByAll: #($1 $2 $3)) = (OrderedCollection with: 'abc' with: 'gh' with: 'i').
]
