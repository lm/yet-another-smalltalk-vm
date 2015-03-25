MockNodeVisitor := Object [

	visitClassNode: aNode [
		Assert true: (aNode isKindOf: ClassNode).
	]


	visitVariableNode: aNode [
		Assert true: (aNode isKindOf: VariableNode).
	]


	visitMethodNode: aNode [
		Assert true: (aNode isKindOf: MethodNode).
	]


	visitBlockNode: aNode [
		Assert true: (aNode isKindOf: BlockNode).
	]


	visitExpressionNode: aNode [
		Assert true: (aNode isKindOf: ExpressionNode).
	]


	visitIntegerNode: aNode [
		Assert true: (aNode isKindOf: IntegerNode).
	]


	visitMessageExpressionNode: aNode [
		Assert true: (aNode isKindOf: MessageExpressionNode).
	]


	visitArrayNode: aNode [
		Assert true: (aNode isKindOf: ArrayNode).
	]


	visitCharacterNode: aNode [
		Assert true: (aNode isKindOf: CharacterNode).
	]


	visitFalseNode: aNode [
		Assert true: (aNode isKindOf: FalseNode).
	]


	visitTrueNode: aNode [
		Assert true: (aNode isKindOf: TrueNode).
	]


	visitNilNode: aNode [
		Assert true: (aNode isKindOf: NilNode).
	]

]


[ "test valid smalltalk code"
	| parser |

	parser := Parser parseString: 'Foo := Object []'.
	Assert true: (parser parseClass isKindOf: ClassNode).
	parser := Parser parseString: 'foo []'.
	Assert true: (parser parseMethod isKindOf: MethodNode).
	parser := Parser parseString: 'foo []'.
	Assert true: (parser parseMethodOrBlock isKindOf: MethodNode).
	parser := Parser parseString: '[]'.
	Assert true: (parser parseMethodOrBlock isKindOf: BlockNode).
]

[ "test empty input"
	| parser |

	parser := Parser parseString: ''.
	Assert do: [parser parseClass] expect: ParseError.
	parser := Parser parseString: ''.
	Assert do: [parser parseMethod] expect: ParseError.
	parser := Parser parseString: ''.
	Assert do: [parser parseMethodOrBlock] expect: ParseError.
]

[ "test invalid smalltalk code"
	| parser |

	parser := Parser parseString: 'a'.
	Assert do: [parser parseClass] expect: ParseError.
	parser := Parser parseString: 'Object subclass'.
	Assert do: [parser parseClass] expect: ParseError.
	parser := Parser parseString: 'Object subclass: Foo'.
	Assert do: [parser parseClass] expect: ParseError.
	parser := Parser parseString: 'Object subclass: $'.
	Assert do: [parser parseClass] expect: ParseError.

	parser := Parser parseString: 'a'.
	Assert do: [parser parseMethod] expect: ParseError.
	parser := Parser parseString: 'a ['.
	Assert do: [parser parseMethod] expect: ParseError.
	parser := Parser parseString: 'a: ['.
	Assert do: [parser parseMethod] expect: ParseError.
	parser := Parser parseString: '+ 1 ['.
	Assert do: [parser parseMethod] expect: ParseError.

	parser := Parser parseString: 'a'.
	Assert do: [parser parseMethodOrBlock] expect: ParseError.
	parser := Parser parseString: 'a ['.
	Assert do: [parser parseMethodOrBlock] expect: ParseError.
	parser := Parser parseString: 'a: ['.
	Assert do: [parser parseMethodOrBlock] expect: ParseError.
	parser := Parser parseString: '[x:]'.
	Assert do: [parser parseMethodOrBlock] expect: ParseError.
]

[ "extra long method selector"
	| stream method |

	stream := CollectionStream with: (String new: 512).
	stream next: 256 put: $a; nextPutAll: '[]'.

	method := (Parser parseString: stream contents) parseMethod.
	Assert true: method selector size = 256.
	Assert true: (method selector at: 1) = $a.
	Assert true: (method selector at: 256) = $a.
]

[ "visitor test"
	| parser |

	parser := Parser parseString: 'Foo := Object [ bar: a [ | b | b := 1 + 2. #(1 $a). false. true. nil ] ]'.
	parser parseClass acceptVisitor: MockNodeVisitor new.
]
