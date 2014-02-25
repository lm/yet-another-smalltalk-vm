#ifndef CLI_H
#define CLI_H

#include <unistd.h>

typedef struct {
	char *error;
	char operand;
	char *bootstrapDir;
	char *snapshotFileName;
	char *fileName;
	char *eval;
	_Bool printHelp;
} CliArgs;


static void parseCliArgs(CliArgs *cliArgs, int argc, char **args)
{
	cliArgs->error = NULL;
	cliArgs->operand = '0';
	cliArgs->bootstrapDir = NULL;
	cliArgs->snapshotFileName = "snapshot";
	cliArgs->fileName = NULL;
	cliArgs->eval = NULL;
	cliArgs->printHelp = 0;

	int arg;
	opterr = 0;
	while ((arg = getopt(argc, args, "hb:s:f:e:")) != -1) {
		switch (arg) {
		case 'e':
			cliArgs->eval = optarg;
			break;
		case 'f':
			cliArgs->fileName = optarg;
			break;
		case 's':
			cliArgs->snapshotFileName = optarg;
			break;
		case 'b':
			cliArgs->bootstrapDir = optarg;
			break;
		case 'h':
			cliArgs->printHelp = 1;
			break;
		case '?':
			cliArgs->operand = optopt;
			switch (optopt) {
			case 'e':
			case 'f':
			case 's':
			case 'b':
				cliArgs->error = "Option -%c requires an operand";
				break;
			default:
				cliArgs->error = "Unrecognized option: '-%c'";
				cliArgs->operand = optopt;
				break;
			}
			break;
		}
	}
	opterr = 1;

	if (cliArgs->eval != NULL && cliArgs->fileName != NULL) {
		cliArgs->error = "Cannot use -e and -f together";
	}
}


static void printCliHelp(void)
{
	printf(
		"Usage:\t<executable> [-e <code>] [-f <file>] [-s <snapshot file>] [-b <kernel dir>]\n"
		"\t-e evaluate code\n"
		"\t-f compile classes and evaluate code within specified file\n"
		"\t-s path to snapshot file\n"
		"\t-b bootstrap from kernel directory\n"
		"\t-h prints this help\n"
	);
}

#endif
