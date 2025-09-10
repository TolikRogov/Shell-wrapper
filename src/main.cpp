#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#ifdef __APPLE__
	#include <_ctype.h>
#else
	#include <ctype.h>
#endif

#define ARG_MAX 2097152

#define ESC_BOLD 			"\\033[1m"
#define ESC_BRIGHT_RED 		"\\033[91m"
#define ESC_BRIGHT_YELLOW 	"\\033[93m"
#define ESC_RESET 			"\\033[0m"

char **parse_cmd(const char *cmd);
static void run_cmd(const char *cmd);
int strwords(const char *string);

int main() {

	while (true) {
		char cmd[ARG_MAX] = {};

		system("echo \"" ESC_BOLD ESC_BRIGHT_RED
			   "$USER@$HOSTNAME" ESC_RESET ":"
			   ESC_BOLD ESC_BRIGHT_YELLOW "$PWD"
			   ESC_RESET "$ \\c\"");

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
		if (words_count > 0 && *p == '|')
			break;

		if (isspace(*p)) {
			words_count++;
			while (isspace((*(p + 1)))) p++;
		}
	}

	return words_count;
 }

 char **parse_cmd(const char* cmd) {

	char **args = NULL;
	size_t words_amount = (size_t)strwords(cmd);

	args = (char**)calloc(words_amount + 1, sizeof(char*));
	if (!words_amount || !args) {
		printf("memory allocation failed!\n");
		return NULL;
	}

	size_t arg_index = 0;
	char delim[] = " \t\n";

	char cmd_copy[ARG_MAX] = {};
	strcpy(cmd_copy, cmd);

	for (char *p = strtok(cmd_copy,delim); arg_index < words_amount; p = strtok(NULL, delim))
		args[arg_index++] = p;

	return args;
 }

 static void run_cmd(const char *cmd) {

	if (!cmd)
		return;

	static int pipefd[2] = {};
	static int fd_in = 0;
	if (pipe(pipefd) == -1) {
        perror("pipe failed");
        return;
    }

	static int conv_cnt = -1;
	const char *new_cmd = strchr(cmd, '|');
	if (new_cmd || conv_cnt > -1)
		conv_cnt++;

	const pid_t pid = fork();
	char **args = NULL;

	if (pid < 0) {
		printf("fork failed!\n");
		return;
	}

	if (pid) {
		int status = 0;
		waitpid(pid, &status, 0);

		close(pipefd[1]);
		if (conv_cnt > 0)
			close(fd_in);
		fd_in = pipefd[0];

		if (args) {
			free(args);
			args = NULL;
		}

		printf("Child process exit code: %d\n", WEXITSTATUS(status));
		
		if (new_cmd)
			run_cmd(new_cmd + 1);

		fd_in = 0;
		conv_cnt = -1;
		return;
	}

	args = parse_cmd(cmd);

	if (conv_cnt > 0)
          dup2(fd_in, STDIN_FILENO);

	if (new_cmd) {
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
	}

	if (args) execvp(args[0], args);
	printf("exec* failed\n");

	return;
}
