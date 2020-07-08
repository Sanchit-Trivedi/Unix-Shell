#include<stdio.h>
#include<string.h> 
#include<stdlib.h> 
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<ctype.h>
#include<fcntl.h>
#define MAXCHAR 10000
#define MAXCOMMANDS 1000

int tokenize_at_pipes(char input[MAXCHAR],char tokens[MAXCOMMANDS][MAXCOMMANDS]);
int redirectionCheck(char* command);
void execute(char **argv);
void tokenize_at_spaces(char cmd[MAXCOMMANDS]);
void redirection(char *input, int state);
void call_pipe(char **in, int *pcount, int i);

void strip(char *str){
	int begin = 0,end = strlen(str) - 1;
	while (isspace((unsigned char) str[begin]))
		begin++;
	while ((end >= begin) && isspace((unsigned char) str[end]))
		end--;
	int i;
	for (i = begin; i <= end; i++)
		str[i - begin] = str[i];
	str[i - begin] = '\0';
}

int main(){
	while(1){
		char cwd[1024] = {};
		getcwd(cwd,1024);
		printf("%s%s%s", "Shell@sanchit:~",cwd,"$ ");
		char input[MAXCHAR]={};
		char commands[MAXCOMMANDS][MAXCOMMANDS]={{}};
		fgets(input,MAXCHAR,stdin);
		strtok(input,"\n");strtok(input,"\n");
		if (input[0] == '\n'|| input[0]==00){
			continue;
		}
		if(strcmp(input,"exit")==0){
			exit(0);
		}
		if(fork()==0){//In child
			for(int i = 0; i < MAXCOMMANDS; i++){
				commands[i][0] = '\0';
			} 
			int pipe_count = tokenize_at_pipes(input,commands);
			if (pipe_count == 0){
				int x = 0;
				if ((x = redirectionCheck(commands[0]))<0){
					tokenize_at_spaces(commands[0]);
				}
				else{
					redirection(commands[0],x);
				}
			}
			else{
				for(int i = 0; i <= pipe_count; i++){
					strip(commands[i]);
				}
				int counter = pipe_count+1;
				call_pipe(commands, &counter, 0);
			}
		}
		else{
			wait(NULL);
		}
	}
}

int tokenize_at_pipes(char input[MAXCHAR],char tokens[MAXCOMMANDS][MAXCOMMANDS]){
	char* ptr = strtok(input,"|");
	int i = 0;
	while (ptr!=NULL){
		strcpy(tokens[i], ptr);
		i++;
		ptr = strtok(NULL,"|");
	}
	return (i-1);
}

void tokenize_at_spaces(char cmd[MAXCOMMANDS]){
	if (cmd[0] == '\0') return;
	char *args[MAXCOMMANDS];
	for (int i=0;i<MAXCOMMANDS;i++) args[i] = NULL;
	char *p = strtok(cmd," \t");
	int i = 0;
	while (p != NULL){
		args[i] = p;
		i++;
		p = strtok(NULL," \t");		
	}
	if (strcmp(args[0],"exit")==0){
		exit(0);
	}
	else if(strcmp(args[0],"cd")==0){
		chdir(args[1]);
	}
	else{
		if (args[0] != NULL)
			execute(args);
	}
}

void execute(char **argv){
	if (execvp(argv[0], argv) < 0) {     // execute the command  
		printf("ERROR: execution failed\n");
		exit(1);
	}
}

//checks for redirection and returns the case
int redirectionCheck(char *command){
	char *out = strstr(command, ">");
	char *in = strstr(command, "<");
	char *append = strstr(command,">>");
	if((append!=NULL) && (in!=NULL)){
		return 5;//both append and input redirection
	}else if((out != NULL) && (in != NULL)){
		return 4;//both input and output redirection
	}else if(append!=NULL){
		return 3;//append redirection only
	}else if(out != NULL){
		return 2;//output redirection only
	}else if(in != NULL){
		return 1;//input redirection only
	}else{
		return -1;
	}
}

void call_pipe(char **in, int *pcount, int i){
	if(i == *pcount - 1){
		//final process
		char cpy[MAXCOMMANDS];
		strcpy(cpy, in[i]);

		char *args[MAXCOMMANDS];
		for (int i=0;i<MAXCOMMANDS;i++) args[i] = NULL;
		char *p = strtok(cpy," \t");
		int i = 0;
		while (p != NULL){
			args[i] = p;
			i++;
			p = strtok(NULL," \t");		
		}
		int x;
		if((x = redirectionCheck(in[i])) < 0){
				execute(args);
		}
		else{ 
			redirection(in[i], x);
		}
	}
	if(i < *pcount){
		int fd[2];
		pid_t pid;
		char cpy[MAXCOMMANDS];
		char *args[MAXCOMMANDS];

		if(pipe(fd) < 0){
			printf("pipe failed");
			exit(1);
		}
		pid = fork();
		if(pid < 0){
			printf("fork failed");
			exit(1);
		}

		if(pid != 0){
			dup2(fd[1], 1);
			close(fd[0]);
			in[i+1] = NULL;

			strcpy(cpy,in[i]);

			for (int i=0;i<MAXCOMMANDS;i++) args[i] = NULL;
			char *p = strtok(cpy," \t");
			int i = 0;
			while (p != NULL){
				args[i] = p;
				i++;
				p = strtok(NULL," \t");		
			}
			int x;
			if((x = redirectionCheck(in[i])) < 0){
				execute(args);
			}
			else{
				redirection(in[i], x);}
			wait(NULL);
		
		}
		else{
			if(i != *pcount-1){
				dup2(fd[0], 0);
			}
			close(fd[1]);
			
			i++;
			call_pipe(in, pcount, i);
		}
	}
}

//implements redirection
void redirection(char *input, int cond){

	char cpy[MAXCOMMANDS],cmd[MAXCOMMANDS],filename[MAXCOMMANDS];
	int fd;
	strcpy(cpy,input);
	char *temp;
	temp = strtok(cpy, " ");
	strcpy(cpy,input);
	if(cond == 1){
		temp = strtok(cpy, "<");
		temp[strlen(temp) - 1] = '\0';
		strcpy(cmd, temp);

		temp = strtok(NULL, "\0");
		strcpy(filename,temp);
		strip(filename);

		if((fd = open(filename, O_RDONLY, 0644)) < 0){
			printf("Error opening file");
		}

		dup2(fd, 0);
		close(fd);
		tokenize_at_spaces(cmd);

	}
	else if(cond == 2){
		char*p = strstr(cpy,">");
		temp = strtok(cpy,">");
		temp[strlen(temp) - 1] = '\0';
		strcpy(cmd,temp);

		temp = strtok(NULL, "\0");
		strcpy(filename,temp);
		strip(filename);

		if((fd = creat(filename , 0644)) < 0){
			printf("Error opening file");
		}
		if((p-1)[0]=='1'){
			dup2(fd,1);
		}
		else if((p-1)[0]=='2'){
			dup2(fd,2);
		}
		else if((p+1)[0]=='&'){
			dup2(2,1);
			close(2);
		}
		else{
			dup2(fd,1);
		}
		close(fd);
		tokenize_at_spaces(cmd);
	}
	else if(cond == 3){
		char *p = strstr(cpy,">>");
		strcpy(p,p+1);
		temp = strtok(cpy,">");
		temp[strlen(temp) - 1] = '\0';
		strcpy(cmd,temp);

		temp = strtok(NULL, "\0");
		strcpy(filename,temp);
		strip(filename);

		if((fd = open(input, O_WRONLY | O_APPEND)) < 0){
			printf("Error opening file");
		}

		dup2(fd, 1);
		close(fd);
		tokenize_at_spaces(cmd);
	}
	else if(cond == 4){
		char filename2[MAXCOMMANDS];
		int fd2;

		temp = strtok(cpy, "<");
		temp[strlen(temp) - 1] = '\0';
		strcpy(cmd,temp);

		temp = strtok(NULL, ">");
		strcpy(filename,temp);
		filename[strlen(filename) - 1] = '\0';
		temp[strlen(temp) - 1] = '\0';
		temp = strtok(NULL, "\0");
		strcpy(filename2,temp);
		strip(filename);
		strip(filename2);

		if((fd = open(filename, O_RDONLY, 0644)) < 0){
			printf("Error opening file");
		}

		if((fd2 = creat(filename2 , 0644)) < 0){
			printf("Error opening file");
		}

		dup2(fd, 0);
		close(fd);

		dup2(fd2, 1);
		close(fd2);

		tokenize_at_spaces(cmd);
	}
	else if (cond == 5){
		char filename2[MAXCOMMANDS];
		int fd2;

		temp = strtok(cpy, "<");
		temp[strlen(temp) - 1] = '\0';
		strcpy(cmd,temp);
		
		char *p = strstr(cpy,">>");
		strcpy(p,p+1);
		temp = strtok(NULL, ">");
		strcpy(filename,temp);
		filename[strlen(filename) - 1] = '\0';
		temp[strlen(temp) - 1] = '\0';

		temp = strtok(NULL, "\0");
		strcpy(filename2,temp);
		strip(filename);
		strip(filename2);

		if((fd = open(filename, O_RDONLY, 0644)) < 0){
			printf("Error opening file");
		}

		if((fd2 = open(input, O_WRONLY | O_APPEND)) < 0){
			printf("Error opening file");
		}

		dup2(fd, 0);
		close(fd);

		dup2(fd2, 1);
		close(fd2);

		tokenize_at_spaces(cmd);
	}
}