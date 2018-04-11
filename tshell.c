//Tiny shell: The most basic shell I can think of

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <errno.h>
#include <pwd.h>

#define DEFAULT_PATH getpwuid(getuid())->pw_dir

char **parse_input(char *str);
void exec_cmd(char **cmds);

int main(void)
{
	char input[1024]="";
	for (;;) { //An infinite loop to keep taking commands
		printf("> ");
		fgets(input, sizeof(input), stdin); //Save input into the input array
		input[strlen(input)-1]='\0'; //Remove trailing new line char
		char **cmds=parse_input(input); //Parse the input string and get an array containing
		//command and arguments to be pased to the execvp() in exec_cmd();
		if (cmds) {
			exec_cmd(cmds); //execute cmds returned by parse_input() 
			for (unsigned i=0;cmds[i];i++) { //free the cmds array
				free(cmds[i]);
			}
			free(cmds);
		}
	}
	return 0; //Never reached
}


char **parse_input(char *str) //Get substrings from 'str' using space as IFS(input field separator)
{
	if (!str) return NULL;
	char **substr=NULL;
	char buf[1024]="";
	size_t str_len=strlen(str);
	if (str_len == 0) return NULL;
	unsigned length=0, substr_n=0;
	for (unsigned i=0;i<str_len;i++) {
		while (str[i] != ' ' && str[i] != '\0' && length < sizeof(buf))
			buf[length++]=str[i++];
		if (length) {
			buf[length]='\0';
			substr=realloc(substr, sizeof(char **)*(substr_n+1));
			substr[substr_n]=calloc(length, sizeof(char *));
			strncpy(substr[substr_n++], buf, length);
			length=0;
		}
	}
	if (!substr_n) return NULL;
	substr=realloc(substr, sizeof(char **)*(substr_n+1));
	substr[substr_n]=NULL;
	return substr;
}

void exec_cmd(char **cmds) //Execute a given command
{
	if (strcmp (cmds[0], "cd") == 0) {	//A little 'cd' implementation
		if (cmds[1]) {
			int ret=0;
			if ((ret=chdir(cmds[1])) == -1)
				perror("cd");
		}
		else chdir(DEFAULT_PATH);
		return;
	}
	if (strcmp(cmds[0], "exit") == 0) {	//Define some exit command
		for (unsigned i=0;cmds[i];i++)
			free(cmds[i]);
		free(cmds);
		exit(EXIT_SUCCESS);
	}		
	pid_t pid=fork();
	if (pid < 0) { //fork failed
		perror("fork");
		return;
	}
	if (pid == 0) { //child process
		if (execvp(cmds[0], cmds) == -1) {
			fprintf(stderr, "Tshell: %s: %s\n", cmds[0], strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
	else { //parent process
		waitpid(pid, NULL, 0);
	}
}
