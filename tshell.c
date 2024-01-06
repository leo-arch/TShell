/* Tiny shell: The most basic shell I can think of */

/*
 * This file is part of Tshell
 *
 * Tshell is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Tshell is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
*/

#include <stdio.h>  /* printf, fgets */
#include <stdlib.h> /* exit, calloc, realloc, free */
#include <string.h> /* strlen, strncpy, strerror */
#include <unistd.h> /* geteuid, chdir, fork, execvp */
#include <wait.h>   /* waitpid */
#include <errno.h>  /* errno */
#include <pwd.h>    /* getpwuid */

#define DEFAULT_PATH getpwuid(getuid())->pw_dir
#define MAX_INPUT 4096

static char **parse_input(char *str);
static void exec_cmd(char **cmd);

int
main(void)
{
	char input[MAX_INPUT] = "";

	for (;;) { /* An infinite loop to keep taking commands. */
		printf("> "); /* A basic prompt. */
		/* For a more elaborated prompt, try to get at least the
		 * current working directory. */

		fgets(input, sizeof(input), stdin); /* Take input. */
		/* For a usable command line, with cool editing capabilities,
		 * take a look at readline or libedit. */

		unsigned int inlen = strlen(input);
		if (inlen > 0)
			input[inlen - 1] = '\0'; /* Remove trailing new line char. */

		/* Parse the input string and get an array containing
		 * command and arguments. */
		char **cmd = parse_input(input);
		if (cmd == NULL)
			continue;

		/* Execute the command returned by parse_input(). */
		exec_cmd(cmd);

		/* Free the cmds array */
		for (unsigned int i = 0; cmd[i]; i++)
			free(cmd[i]);
		free(cmd);
	}

	return 0; /* Never reached */
}


/* Get substrings from 'str' using space as IFS (input field separator) */
char **
parse_input(char *str)
{
	if (!str || !*str)
		return (char **)NULL;

	char **substr = (char **)NULL;
	char buf[MAX_INPUT] = "";

	unsigned int str_len = strlen(str);
	if (str_len == 0)
		return (char **)NULL;

	unsigned int length = 0;
	unsigned int substr_n = 0;

	for (unsigned int i = 0; i < str_len; i++) {
		while (str[i] != ' ' && str[i] != '\0' && length < sizeof(buf) - 1)
			buf[length++] = str[i++];

		if (length == 0)
			continue;

		buf[length] = '\0';

		substr = realloc(substr, (substr_n + 2) * sizeof(char **));
		substr[substr_n] = calloc(length, sizeof(char *));
		strncpy(substr[substr_n++], buf, length);

		length = 0;
	}

	if (substr_n == 0)
		return (char **)NULL;

	substr[substr_n] = (char *)NULL;

	return substr;
}

/* Execute a given command */
void
exec_cmd(char **cmd)
{
	/* A little 'cd' implementation */
	if (strcmp(cmd[0], "cd") == 0) {
		if (cmd[1]) {
			int ret = chdir(cmd[1]);
			if (ret == -1)
				perror("cd");
		} else {
			chdir(DEFAULT_PATH);
		}

		return;
	}

	/* Define some exit command */
	if (strcmp(cmd[0], "exit") == 0) {
		for (unsigned int i = 0; cmd[i]; i++)
			free(cmd[i]);
		free(cmd);

		exit(0);
	}

	/* Add here more internal commands. To get an idea enter 'help'
	 * in bash. */

	/* Pipes, concatenation, stream redirection, etc? Good look with that! */

	pid_t pid = fork();
	if (pid < 0) { /* Fork failed */
		perror("fork");
		return;
	}

	if (pid == 0) { /* Child process */
		if (execvp(cmd[0], cmd) == -1) {
			fprintf(stderr, "Tshell: %s: %s\n", cmd[0], strerror(errno));
			exit(1);
		}
	} else { /* Parent process */
		waitpid(pid, NULL, 0);
	}
}
