set unwindonsignal on

define ds
	set $code = findNativeCodeAtIc($arg0)
	set $method = 0
	set $block = 0
	set $source = 0
	if (((RawObject *) $code->compiledCode)->class == Handles.CompiledMethod->raw)
		set $method = (RawCompiledMethod *) $code->compiledCode
		set $source = (RawSourceCode *) asObject($method->sourceCode)
	else
		set $block = (RawCompiledBlock *) $code->compiledCode
		set $method = (RawCompiledMethod *) asObject($block->method)
		set $source = (RawSourceCode *) asObject($block->sourceCode)
	end
	set $class = (RawClass *) asObject($method->ownerClass)
	set $selector = (RawString *) asObject($method->selector)
	printClass $class
	printf "#%s ", $selector->contents
	if $block
		printf "[] "
	end
	printSourceCode $source
	printf "\n"
	disassemble $code->insts, $code->insts + $code->size
end

define rawds
	set $code = findNativeCodeAtIc($arg0)
	disassemble $code->insts, $code->insts + $code->size
end

define printClass
	set $class = (RawClass *) $arg0
	if $class->class == Handles.MetaClass->raw
		set $class = (RawClass *) asObject(((RawMetaClass *) $arg0)->instanceClass)
	end
	set $name = (RawString *) asObject($class->name)
	printf "%s", $name->contents
	if $arg0->class == Handles.MetaClass->raw
		printf " class"
	end
end

define printSourceCode
	set $sName = (RawString *) asObject($arg0->sourceOrFileName)
	printf "%s:%li:%li", $sName->contents, asCInt($arg0->line), asCInt($arg0->column)
end

define parentbp
	p/x ((Value *) $arg0)[0]
end
