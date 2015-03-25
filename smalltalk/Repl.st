Repl := Object [

	class autocomplete: aString [
		| completions index part |

		completions := OrderedCollection new: 8.
		index := aString lastIndexOf: Character space.
		part := index > 0
			ifTrue: [aString copyFrom: index + 1]
			ifFalse: [aString].

		SymbolTable do: [ :symbol |
			(symbol isNil not and: [symbol startsWith: part]) ifTrue: [
				completions add: ((CollectionStream on: (String new: index + symbol size + 1))
					nextPutAll: (aString copyFrom: 1 to: index);
					nextPutAll: symbol;
					nextPut: (Character codePoint: 0);
					contents)]].
		^completions
	]


	class run [
		| input compiler |

		input := self fileNamed: 'stdin'.
		compiler := Compiler new.
		[Transcript nextPutAll: 'Smalltalk> '.
		compiler evaluate: (input upTo: Character lf)] repeat.
	]

]
