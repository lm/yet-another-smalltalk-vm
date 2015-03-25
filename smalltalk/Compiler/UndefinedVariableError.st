UndefinedVariableError := CompileError [

	"accessing"

	defaultMessageText [
		| sourceCode |

		sourceCode := identifier sourceCode.
		^'Undefined variable: ',
			identifier value,
			' in ''', sourceCode source,
			''' line ', sourceCode line printString,
			' column ', sourceCode column printString
	]

]
