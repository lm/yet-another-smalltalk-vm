RedefinitionError := CompileError [

	"accessing"

	defaultMessageText [
		| sourceCode |

		sourceCode := identifier sourceCode.
		^'Cannot redefine: ',
			identifier value,
			' in ''', sourceCode source,
			''' line ', sourceCode line printString,
			' column ', sourceCode column printString
	]

]
