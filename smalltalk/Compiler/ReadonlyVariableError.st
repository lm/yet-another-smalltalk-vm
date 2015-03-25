ReadonlyVariableError := CompileError [

	"accessing"

	defaultMessageText [
		| sourceCode |

		sourceCode := identifier sourceCode.
		^'Cannot write to readonly variable: ',
			identifier value,
			' in ''', sourceCode source,
			''' line ', sourceCode line printString,
			' column ', sourceCode column printString
	]

]
