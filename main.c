#include "vm/Bootstrap.h"
#include "vm/Snapshot.h"
#include "vm/Entry.h"
#include "vm/Repl.h"
#include "vm/Thread.h"
#include "vm/Cli.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static void bootstrapSmalltalk(char *snapshotFileName, char *bootstrapDir);


int main(int argc, char **args)
{
	CliArgs cliArgs;
	int result = EXIT_SUCCESS;

	parseCliArgs(&cliArgs, argc, args);
	initThread(&CurrentThread);
	bootstrapSmalltalk(cliArgs.snapshotFileName, cliArgs.bootstrapDir);

	if (cliArgs.error != NULL) {
		printf(cliArgs.error, cliArgs.operand);
		printf("\n");
		result = EXIT_FAILURE;
	} else if (cliArgs.printHelp) {
		printCliHelp();
	} else if (cliArgs.fileName != NULL) {
		Value blockResult;
		if (parseFileAndInitialize(cliArgs.fileName, &blockResult)) {
			result = valueTypeOf(blockResult, VALUE_INT) ? asCInt(blockResult) : result;
		} else {
			result = EXIT_FAILURE;
		}
	} else if (cliArgs.eval != NULL) {
		result = asCInt(evalCode(cliArgs.eval));
	} else {
		runRepl();
	}

	freeHandles();
	freeThread(&CurrentThread);
	return result;
}



static void bootstrapSmalltalk(char *snapshotFileName, char *bootstrapDir)
{
	FILE *snapshot;
	if (bootstrapDir) {
		snapshot = fopen(snapshotFileName, "w+");
		if (snapshot == NULL) {
			printf("Cannot write to snapshot file: '%s'\n", snapshotFileName);
			exit(EXIT_FAILURE);
		}
		if (!bootstrap(bootstrapDir)) {
			printf("Bootstrap failed\n");
			exit(EXIT_FAILURE);
		}
		snapshotWrite(snapshot);
	} else {
		snapshot = fopen(snapshotFileName, "r");
		if (snapshot == NULL) {
			printf("Cannot read snapshot file: '%s'\n", snapshotFileName);
			exit(EXIT_FAILURE);
		}
		snapshotRead(snapshot);
	}
	fclose(snapshot);
}
