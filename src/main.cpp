#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>

#define ARG_MAX 2097152

#define ESC_BOLD 			"\\033[1m"
#define ESC_BRIGHT_RED 		"\\033[91m"
#define ESC_BRIGHT_YELLOW 	"\\033[93m"
#define ESC_RESET 			"\\033[0m"

char **parse_cmd(const char* cmd);
static void run_cmd(const char *cmd);
int strwords(const char *string);

int main() {
	while (true) {
		char cmd[ARG_MAX] = {};
		system("echo -n \"" ESC_BOLD ESC_BRIGHT_RED
			   "$USER@$HOSTNAME" ESC_RESET ":"
			   ESC_BOLD ESC_BRIGHT_YELLOW "$PWD"
			   ESC_RESET "$ \"");
		read(STDIN_FILENO, cmd, sizeof(cmd));
		run_cmd(cmd);
	}
	return 0;
 }

 int strwords(const char *string) {
	int words_count = 0;
	const char *p = string;
	while (isspace((*p))) p++;
	for (; *p != '\0'; p++) {
		if (isspace(*p)) {
			words_count++;
			p++;
		}
		while (isspace((*p))) p++;
	}
	return words_count;
 }

 char **parse_cmd(const char* cmd) {
	char **args = NULL;

	size_t words_amount = (size_t)strwords(cmd);
	args = (char**)calloc(words_amount, sizeof(char*));
	if (!args) {
		printf("memory allocation failed!\n");
		return NULL;
	}

	size_t arg_index = 0;
	char cmd_copy[ARG_MAX] = {};
	char delim[] = " \t\n";
	strcpy(cmd_copy, cmd);
	for (char *p = strtok(cmd_copy,delim); p != NULL; p = strtok(NULL, delim))
		args[arg_index++] = p;

	return args;
 }

 static void run_cmd(const char *cmd) {
	const pid_t pid = fork();
	char **args = NULL;

	if (pid < 0) {
		printf("fork failed!\n");
		return;
	}

	if (pid) {
		int status = 0;
		waitpid(pid, &status, 0);
		if (args) {
			free(args);
			args = NULL;
		}
		printf("Child process exit code: %d\n", WEXITSTATUS(status));
		return;
	}

	args = parse_cmd(cmd);
	execvp(args[0], args);
	printf("exec* failed\n");
	return;
}
