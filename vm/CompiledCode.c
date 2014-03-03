#include "CompiledCode.h"
#include "Thread.h"
#include "Smalltalk.h"
#include "Class.h"
#include "Heap.h"
#include "HeapPage.h"
#include "Handle.h"
#include "Iterator.h"
#include "Assert.h"


NativeCode *findNativeCodeAtIc(uint8_t *ic)
{
	PageSpaceIterator iterator;
	NativeCode *obj;
	pageSpaceIteratorInit(&iterator, &CurrentThread.heap.execSpace);
	obj = (NativeCode *) pageSpaceIteratorNext(&iterator);
	while (obj != NULL) {
		if ((obj->tags & TAG_FREESPACE) == 0 && obj->insts <= ic && ic < obj->insts + obj->size) {
			return obj;
		}
		obj = (NativeCode *) pageSpaceIteratorNext(&iterator);
	}
	return NULL;
}


void printMethodsUsage(void)
{
	PageSpaceIterator iterator;
	pageSpaceIteratorInit(&iterator, &CurrentThread.heap.execSpace);
	NativeCode *code = (NativeCode *) pageSpaceIteratorNext(&iterator);

	while (code != NULL) {
		if ((code->tags & TAG_FREESPACE) == 0) {
			if (code->compiledCode != NULL) {
				RawCompiledMethod *method = (RawCompiledMethod *) code->compiledCode;
				_Bool isBlock = method->class == Handles.CompiledBlock->raw;
				if (isBlock) {
					method = (RawCompiledMethod *) asObject(((RawCompiledBlock *) method)->method);
				}
				RawString *selector = (RawString *) asObject(method->selector);
				RawClass *class = (RawClass *) asObject(method->ownerClass);
				printClassName(class);
				printf(
					"#%.*s%s\t",
					(int) selector->size, selector->contents,
					isBlock ? "[]" : ""
				);
			} else {
				printf("<unknown>\t");
			}
			printf("%zu\n", code->counter);
		}
		code = (NativeCode *) pageSpaceIteratorNext(&iterator);
	}
}
