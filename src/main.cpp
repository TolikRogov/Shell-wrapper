#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>

char **parse_cmd(const char* cmd);
static void run_cmd(const char *cmd);

// int main() {
// 	while(1) {
// 		char *cmd = NULL;
// 		// read cmd here
// 		run_cmd(cmd);
// 	}
// 	return 0;
//  }

 char **parse_cmd(const char* cmd) {
	char **args = NULL;
	return args;
 }

 static void run_cmd(const char *cmd) {
	const pid_t pid = fork();

	if (pid < 0) {
		printf("fork failed!\n");
		return;
	}

	if (pid) {
		int status = 0;
		waitpid(pid, &status, 0);
		printf("Ret code: %d\n", WEXITSTATUS(status));
		return;
	}

	char **args = parse_cmd(cmd);
	execvp(args[0], args);
	printf("exec* failed\n");
	return;
}
