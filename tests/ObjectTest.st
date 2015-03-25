TestIndexedObject := Object [

	<shape: IndexedShape>

]


[
	| object stream |

	object := Object new.

	Assert do: [object at: -1] expect: OutOfRangeError.
	Assert do: [object at: 0] expect: OutOfRangeError.
	Assert do: [object at: 1] expect: OutOfRangeError.

	Assert do: [object at: -1 put: nil] expect: OutOfRangeError.
	Assert do: [object at: 0 put: nil] expect: OutOfRangeError.
	Assert do: [object at: 1 put: nil] expect: OutOfRangeError.

	Assert do: [object basicAt: -1] expect: OutOfRangeError.
	Assert do: [object basicAt: 0] expect: OutOfRangeError.
	Assert do: [object basicAt: 1] expect: OutOfRangeError.

	Assert do: [object basicAt: -1 put: nil] expect: OutOfRangeError.
	Assert do: [object basicAt: 0 put: nil] expect: OutOfRangeError.
	Assert do: [object basicAt: 1 put: nil] expect: OutOfRangeError.

	Assert true: object size == 0.
	Assert true: object basicSize == 0.
	Assert true: object yourself == object.

	Assert do: [object instVarAt: -1] expect: OutOfRangeError.
	Assert do: [object instVarAt: 0] expect: OutOfRangeError.
	Assert do: [object instVarAt: 1] expect: OutOfRangeError.

	Assert do: [object instVarAt: -1 put: nil] expect: OutOfRangeError.
	Assert do: [object instVarAt: 0 put: nil] expect: OutOfRangeError.
	Assert do: [object instVarAt: 1 put: nil] expect: OutOfRangeError.

	Assert true: object = object.
	Assert false: object = nil.

	Assert true: object == object.
	Assert false: object == nil.

	Assert true: object hash > 0.

	Assert false: object ~= object.
	Assert true: object ~= nil.

	Assert false: object ~~ object.
	Assert true: object ~= nil.

	Assert true: object identityHash > 0.

	Assert true: object class == Object.
	Assert true: (object isKindOf: Object).
	Assert false: (object isKindOf: SmallInteger).
	Assert true: (1 isKindOf: Object).
	Assert true: (object isMemberOf: Object).
	Assert false: (object isMemberOf: SmallInteger).
	Assert true: (object respondsTo: #size).
	Assert false: (object respondsTo: #foo).

	"Assert do: [object doesNotUnderstand: ]"

	Assert true: object printString = 'an Object'.
	stream := CollectionStream with: (String new: 32).
	object examineOn: stream.
	Assert true: stream contents = ('an Object' copyWith: Character lf).
]


[
	| object |

	object := TestIndexedObject new: 8.

	Assert do: [object at: -1] expect: OutOfRangeError.
	Assert true: (object at: 1) == nil.
	Assert true: (object at: 8) == nil.
	Assert do: [object at: 9] expect: OutOfRangeError.

	Assert do: [object at: -1 put: nil] expect: OutOfRangeError.
	Assert do: [object at: 9 put: nil] expect: OutOfRangeError.

	object at: 1 put: 1.
	Assert true: (object at: 1) == 1.
	object at: 8 put: 2.
	Assert true: (object at: 8) == 2.

	Assert true: object size == 8.
	Assert true: object basicSize == 8.

	Assert true: object printString = 'a TestIndexedObject'.
]


[
	| object object2 |

	object := Object new.
	object2 := Object new.
	object become: object2.
	Assert true: object identityHash == object2 identityHash.
]


[
	| object object2 |

	object := TestIndexedObject new: 0.
	object2 := TestIndexedObject new: 1.

	object become: object2.
	Assert true: object identityHash == object2 identityHash.
]


[
	| object |

	object := Object new.

	Assert false: object == object shallowCopy.
	Assert false: object = object shallowCopy.

	object := Array new: 16.
	object at: 1 put: Object new.
	Assert false: object == object shallowCopy.
	Assert true: object = object shallowCopy.
	Assert true: (object at: 1) == (object shallowCopy at: 1).
	Assert false: (object at: 1) == (object deepCopy at: 1).
]
