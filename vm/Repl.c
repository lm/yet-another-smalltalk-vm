#include "Repl.h"
#include "Entry.h"
#include "Handle.h"
#include "Smalltalk.h"
#include "Class.h"
#include "Iterator.h"
#include "../linenoise/linenoise.h"
#include <stdlib.h>

static void replAutocomplete(const char *buf, linenoiseCompletions *lc);


void runRepl(void)
{
	char *line;

	linenoiseSetCompletionCallback(replAutocomplete);
	linenoiseHistoryLoad("history.txt");

	while ((line = linenoise("Smalltalk> ")) != NULL) {
		evalCode(line);
		linenoiseHistoryAdd(line);
		linenoiseHistorySave("history.txt");
		free(line);
	}
}


static void replAutocomplete(const char *buf, linenoiseCompletions *lc)
{
	HandleScope scope;
	openHandleScope(&scope);

	Class *repl = getClass("Repl");
	String *str = asString((char *) buf);
	EntryArgs args = { .size = 0 };
	entryArgsAddObject(&args, (Object *) repl);
	entryArgsAddObject(&args, (Object *) str);
	Value result = sendMessage(getSymbol("autocomplete:"), &args);

	if (getClassOf(result) != Handles.OrderedCollection->raw) {
		closeHandleScope(&scope, NULL);
		return;
	}

	Iterator iterator;
	initOrdCollIterator(&iterator, scopeHandle(asObject(result)), 0, 0);
	while (iteratorHasNext(&iterator)) {
		HandleScope scope2;
		openHandleScope(&scope2);
		linenoiseAddCompletion(lc, ((String *) iteratorNextObject(&iterator))->raw->contents);
		closeHandleScope(&scope2, NULL);
	}

	closeHandleScope(&scope, NULL);
}
