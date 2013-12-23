[
	Assert true: (Number readFrom: (CollectionStream on: '1234567890')) = 1234567890.
	Assert true: (Number readFrom: (CollectionStream on: 'FF') base: 16) = 16rFF.
]
