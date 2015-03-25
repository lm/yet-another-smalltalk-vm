VMTools := Object [

	class printCCharacters [
		0 to: 255 do: [ :i |
			| ch defined |
			defined := nil.
			ch := Character codePoint: i.
			"Transcript nextPutAll: '/*'.
			ch printOn: Transcript.
			Transcript nextPutAll: '*/ '."
			('0123456789' includes: ch)
				ifTrue: [
					Transcript nextPutAll: 'NUM'.
					defined := true].
			('abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ' includes: ch)
				ifTrue: [
					Transcript nextPutAll: 'LET'.
					defined := true].
			(ch = Character space or: [ch = Character tab or: [ch = Character lf or: [ch = (Character codePoint: 13)]]])
				ifTrue: [
					Transcript nextPutAll: 'SEP'.
					defined := true].
			('!%&*+,-/<=>?@\|~' includes: ch)
				ifTrue: [
					Transcript nextPutAll: 'SPEC'.
					defined := true].
			defined isNil ifTrue: [Transcript nextPut: $0].
			(i + 1) \\ 8 = 0
				ifTrue: [Transcript nextPut: $,; lf]
				ifFalse: [Transcript nextPutAll: ', '].
		]
	]


	class printCCharacterClasses [
		0 to: 255 do: [ :i |
			| ch |
			ch := Character codePoint: i.
			('abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_' includes: ch)
				ifTrue: [Transcript nextPutAll: 'IDENT_BEGIN']
				ifFalse: [
					('0123456789' includes: ch)
						ifTrue: [Transcript nextPutAll: 'IDENT']
						ifFalse: [Transcript nextPut: $0]].
			(i + 1) \\ 8 = 0
				ifTrue: [Transcript nextPut: $,; lf]
				ifFalse: [Transcript nextPutAll: ', '].
		].
	]


	class printCCharacterDigitValues [
		0 to: 255 do: [ :i |
			| ch |
			ch := Character codePoint: i.
			(ch isDigit: 36)
				ifTrue: [ch digitValue printOn: Transcript]
				ifFalse: [Transcript nextPutAll: '36'].
			(i + 1) \\ 8 = 0
				ifTrue: [Transcript nextPut: $,; lf]
				ifFalse: [Transcript nextPutAll: ', ']].
	]

]
