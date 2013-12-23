[
	Assert do: [6 < Object] expect: Error.
	Assert do: [6 + Object] expect: Error.
	Assert do: [6 - Object] expect: Error.
	Assert do: [6 * Object] expect: Error.
	Assert do: [6 // Object] expect: Error.
	Assert do: [6 \\ Object] expect: Error.
	Assert do: [6 / Object] expect: Error.
	Assert do: [6 quo: Object] expect: Error.
	Assert do: [6 rem: Object] expect: Error.
	Assert do: [6 bitAnd: Object] expect: Error.
	Assert do: [6 bitOr: Object] expect: Error.
	Assert do: [6 bitXor: Object] expect: Error.
	Assert do: [6 bitShift: Object] expect: Error.
]
