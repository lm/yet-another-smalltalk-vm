#include "vm/Heap.h"
#include "vm/Handle.h"
#include "vm/Bootstrap.h"
#include "vm/Snapshot.h"
#include "vm/Entry.h"
#include "vm/Repl.h"
#include "vm/Thread.h"
#include "vm/CompiledCode.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static void printUsage(void);


int main(int argc, char **args)
{
	char *bootstrapDir = NULL;
	char *snapshotFileName = "snapshot";
	char *fileName = NULL;
	char *eval = NULL;
	int result = 0;
	int arg;

	while ((arg = getopt(argc, args, "b:s:f:e:")) != -1) {
		switch (arg) {
		case 'b':
			bootstrapDir = optarg;
			break;
		case 's':
			snapshotFileName = optarg;
			break;
		case 'f':
			fileName = optarg;
			break;
		case 'e':
			eval = optarg;
			break;
		case ':':
			printf("Option -%c requires an operand\n", optopt);
			printUsage();
			exit(EXIT_FAILURE);
		case '?':
			printf("Unrecognized option: '-%c'\n", optopt);
			printUsage();
			exit(EXIT_FAILURE);
		}
	}

	initHeap();
	FILE *snapshot;
	if (bootstrapDir) {
		snapshot = fopen(snapshotFileName, "w+");
		if (snapshot == NULL) {
			printf("Cannot write to snapshot file: '%s'\n", snapshotFileName);
			exit(EXIT_FAILURE);
		}
		bootstrap(bootstrapDir);
		snapshotWrite(snapshot);
	} else {
		snapshot = fopen(snapshotFileName, "r");
		if (snapshot == NULL) {
			printf("Cannot read snapshot file: '%s'\n", snapshotFileName);
			exit(EXIT_FAILURE);
		}
		snapshotRead(snapshot);
		initThread(&CurrentThread);
	}
	fclose(snapshot);

	if (fileName == NULL && eval == NULL) {
		runRepl();
	} else {
		if (fileName) {
			parseFileAndInitialize(fileName);
		}
		if (eval) {
			result = asCInt(evalCode(eval));
		}
	}

	//printMethodsUsage();

	freeHandles();
	freeHeap();
	return result;
}


static void printUsage(void)
{
	printf(
		"Usage:\t<executable> [-b <kernel dir>] [-s <snapshot file>] [-f <file>] [-e <code>]\n"
		"\t-b bootstrap from kernel directory\n"
		"\t-s path to snapshot file\n"
		"\t-f compile classes and evaluate code within specified file\n"
		"\t-e evaluate code\n"
	);
}
