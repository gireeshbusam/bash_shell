#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <ctype.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/signal.h>

//Stores the process ids of processes running in the background
int backgroundList[100];
//Index to handle backgroundList array
int bgPrIndex = 0;

//Ensures that the main process doesn't get stopped but will get pulled back into foreground
void stdinHandler(int sig_no) {
	//Bringing the main process back to the foreground
	if(tcsetpgrp(0, getpid()) == -1) {
		perror("tcsetpgrp");
		exit(EXIT_FAILURE);
	}
}

//Ensures that the main process doesn't get stopped but will get pulled back into foreground
void stdoutHandler(int sig_no) {
	//Bringing the main process back to the foreground
	if(tcsetpgrp(1, getpid()) == -1) {
		perror("tcsetpgrp");
		exit(EXIT_FAILURE);
	}
}

//Validation function which checks the number of commands for each type of special character
char validMaxCommands(char *com, char *spChar) {
	char isValid = 'N';
	int count = 0; //Maintains a count of commands

	//For text file concatenation
	if(strcmp(spChar, "#") == 0) {
		char *validCatToken = strtok(com, "#");

		while(validCatToken != NULL) {
			count++;
			validCatToken = strtok(NULL, "#");
		}

		if(count <= 6) {
			isValid = 'Y';
		}
	} else if(strcmp(spChar, "|") == 0) { //For piping
		char *validPipeToken = strtok(com, "|");

		while(validPipeToken != NULL) {
			count++;
			validPipeToken = strtok(NULL, "|");
		}

		if(count <= 7) {
			isValid = 'Y';
		}
	} else if(strcmp(spChar, "&&||") == 0) { //For conditional Execution
		char *validConditionToken = strtok(com, "&&||");

		while(validConditionToken != NULL) {
			count++;
			validConditionToken = strtok(NULL, "&&||");
		}

		if(count <= 6) {
			isValid = 'Y';
		}
	} else if(strcmp(spChar, ";") == 0) { //For Sequential Execution
		char *validSeqToken = strtok(com, ";");

		while(validSeqToken != NULL) {
			count++;
			validSeqToken = strtok(NULL, ";");
		}

		if(count <= 5) {
			isValid = 'Y';
		}
	}

	return isValid; //Returns 'Y' if the command is within the specified limits 'N' otherwise
}

//Validation function which checks whether each command contains arguments within >=1 && <=5
char validArguments(char *com) {
	char isValidArgs = 'N';

	int count = 0; //Maintains a count of the arguments in each command

	char *validArgsCat = strtok(com, " ");

	while(validArgsCat != NULL) {
		count++;
		validArgsCat = strtok(NULL, " ");
	}

	if(count >= 1 && count <=5) {
		isValidArgs = 'Y';
	}

    /*
	* Returns 'Y' if the number of arguments in each command lie within the limit
	* 'N' otherwise
	*/
	return isValidArgs; 
}

// Validation function which checks whether the command is valid or not
char isValidCommand(char *com) {
	char isValidCmd = 'Y';

	char *sp1 = "|";
	char *sp2 = "#";
	char *sp3 = "&&";
	char *sp4 = "||";
	char *sp5 = "<";
	char *sp6 = ">";
	char *sp7 = ">>";
	char *sp8 = "&";
	char *sp9 = ";";

	if(strncmp(com, sp1, strlen(sp1)) == 0) {
		isValidCmd = 'N';
	} else if(strncmp(com, sp2, strlen(sp2)) == 0) {
		isValidCmd = 'N';
	} else if(strncmp(com, sp3, strlen(sp3)) == 0) {
		isValidCmd = 'N';
	} else if(strncmp(com, sp4, strlen(sp4)) == 0) {
		isValidCmd = 'N';
	} else if(strncmp(com, sp5, strlen(sp5)) == 0) {
		isValidCmd = 'N';
	} else if(strncmp(com, sp6, strlen(sp6)) == 0) {
		isValidCmd = 'N';
	} else if(strncmp(com, sp7, strlen(sp7)) == 0) {
		isValidCmd = 'N';
	} else if(strncmp(com, sp8, strlen(sp8)) == 0) {
		isValidCmd = 'N';
	} else if(strncmp(com, sp9, strlen(sp9)) == 0) {
		isValidCmd = 'N';
	}

	/*
	* Returns 'Y' if the command is valid
	* Returns 'N' if the command is invalid
	*/
	return isValidCmd;
}

//Function to convert relative paths specified in the command to absolute paths
char *aPath(char *com) {
	char *filepath = strstr(com, "~/");

	char *folder = malloc(2048);

	char *token = strtok(filepath, "~");

	while(token != NULL) {
		strcpy(folder, token);
		token = strtok(NULL, "~");
	}

	char *homePath = getenv("HOME");

	char *absolutePath = malloc(2048);

	strcat(absolutePath, homePath);
	//strcat(absolutePath, "/");
	strcat(absolutePath, folder);

	return absolutePath;
}

//Removing the leading and trailing white spaces in the command when tokenizing with the specified special character
char *trimString(char *com) {
	int i;
	int start = 0;
	int last = strlen(com) - 1;

	while(isspace((unsigned char) com[start])) {
		start++;
	}

	while((last >= start) && isspace((unsigned char) com[last])) {
		last--;
	}

	for(i = start; i <= last; i++) {
		com[i - start] = com[i];
	}

	com[i - start] = '\0';

	return com;
}

//Performing .txt files concatenation when the entered command contains the special character '#'
void txtConcat(char *com) {
	char validCom[20000];

	strcpy(validCom, "");
	strcpy(validCom, com);

	//Validating number of commands
	char isValid = validMaxCommands(validCom, "#");

	if(isValid == 'N') {
		printf("A maximum of 5 operations are supported. Please try again.\n");
		return;
	} 
	
	int pid;

	//Parent Process
	if((pid = fork()) > 0) {
		int status;
		int cid = wait(&status);
		//exit(EXIT_SUCCESS);
	} else if(pid == 0) { //Child Process
		int i = 1;
		char *comArr[100];
		char *token = strtok(com, "#");

		//Storing first index in the comArr with cat to perform concatenation when replacing the child with execvp
		comArr[0] = "cat";

		while(token != NULL) {
			token = trimString(token);

			while(i < 7) {
				comArr[i] = token;
				break;
			}
			i++;
			token = strtok(NULL, "#");
		}

		comArr[i] = NULL;

		char *fp = malloc(20000);

		for(int j = 0; comArr[j] != NULL; j++) {
			//strcpy(fp, "");
			char validArgsCom[20000];

			strcpy(validArgsCom, "");
			strcpy(validArgsCom, comArr[j]);

			//Validating number of arguments within each command
			char validArgsFlag = validArguments(validArgsCom);

			if(validArgsFlag == 'N') {
				printf("\n The argc of each command cannot be empty \n");
				return;
			}
			
			fp = strstr(comArr[j], "~/");
			if(fp != NULL) {
				char *folder = aPath(fp);
				comArr[j] = folder;
			}
		}

		//Replacing the child process with execvp and performing concatenation of .txt files
		execvp(comArr[0], comArr);
	}
}

//Executes the pipes one by one
void executePipe(char *programName, char **args, int read_fd, int write_fd) {
	int pid = fork();

	if(pid < 0) {
		printf("\n ERROR OCCURED WHILE FORKING... EXITING \n");
		exit(EXIT_FAILURE);
	} else if(pid == 0) { //CHILD PROCESS
		if(read_fd != 0) {
			dup2(read_fd, 0);
			close(read_fd);
		}
		if(write_fd != 1) {
			dup2(write_fd, 1);
			close(write_fd);
		}

		//Replacing the child process with the program in the command
		execvp(programName, args);
		perror("execvp");
		exit(EXIT_FAILURE);
	}
	wait(NULL);
}

//Function to handle pipes in input
void createPipes(char *com) {
	char validCom[20000];

	strcpy(validCom, "");
	strcpy(validCom, com);

	//Validating number of commands
	char isValid = validMaxCommands(validCom, "|");

	if(isValid == 'N') {
		printf("\n A maximum of 6 piping operations are supported. Please try again. \n");
		return;
	}

	//To maintain a count of number of pipes
	int count = 0;

	//Array to store each command
	char *cmdArr[7];

	//Tokenizing the command with '|' to separate commands
	char *pipeToken = strtok(com, "|");

	while(pipeToken != NULL && count < 7) {
		//printf("\n CMD: %s \n", pipeToken);
		cmdArr[count] = pipeToken;
		count++;
		pipeToken = strtok(NULL, "|");
	}
	cmdArr[count] = NULL;	

	//File Descriptors for diffierent pipes
	int fdArr[count - 1][2];

	for(int i = 0; i < (count - 1); i++) {
		//Initializing pipes
		if(pipe(fdArr[i]) == -1) {
			printf("\n ERROR INITIALIZING PIPES... EXITING \n");
			exit(EXIT_FAILURE);
		}
	}

	int fd1 = 0;

	for(int i = 0; i < count; i++) {
		char validArgsCom[20000];

		strcpy(validArgsCom, "");
		strcpy(validArgsCom, cmdArr[i]);

		//Validating number of arguments within each command
		char validArgsFlag = validArguments(validArgsCom);

		if(validArgsFlag == 'N') {
			printf("\n The argc of each command should be between >=1 && <=5 \n");
			return;
		}

		char *arguments[100];
		int j = 0;

		//Tokenizing with " " to build the arguments for execvp
		char *token = strtok(cmdArr[i], " ");
		
		while(token != NULL) {
			arguments[j] = token;
			j++;
			token = strtok(NULL, " ");
		}

		//Appending NULL as the last Argument in the arguments array to perform exec
		arguments[j] = NULL;
	
	
		if(i == count - 1) {
			executePipe(arguments[0], arguments, fd1, 1);
		} else {
			executePipe(arguments[0], arguments, fd1, fdArr[i][1]);
			close(fdArr[i][1]);
			fd1 = fdArr[i][0];
		}
	}

	//Closing all the file descriptors after performing all the piping operations
	for(int i = 0; i < (count - 1); i++) {
		close(fdArr[i][0]);
		close(fdArr[i][1]);
	}
	//exit(EXIT_SUCCESS);
}

//When there are no special characters we will just execute the command
void execCommand(char *com) {
	char validArgsCom[20000];

	strcpy(validArgsCom, "");
	strcpy(validArgsCom, com);

	//Validating the total number of arguments within each command
	char validArgsFlag = validArguments(validArgsCom);

	if(validArgsFlag == 'N') {
		printf("\n The argc of each command should be between >=1 && <=5 \n");
		return;
	}

	char *cmdArr[100];
	int i = 0;

	int pid = fork();

	if(pid > 0) {
		int status;
		wait(&status);
	} else if(pid < 0) {
		printf("\n ERROR FORKING \n");
		exit(EXIT_FAILURE);
	} else {
		//Tokenizing the user entered input with spaces to separate out arguments
		char *token = strtok(com, " ");

		while(token != NULL) {
			cmdArr[i] = token;
			i++;
			token = strtok(NULL, " ");
		}

		cmdArr[i] = NULL;

		//Executing the command
		execvp(cmdArr[0], cmdArr);
	}
}

//Performing redirection operation when the special character is >>
void appendRedir(char *com) {
	int fd[2];
	char *cmdArr[2]; //Array which stores the tokenized strings from char *com
	int j = 0;

	//Tokenizing char *com with >>
	char *cmdToken = strtok(com, ">>");
	
	while(cmdToken != NULL) {
		cmdArr[j] = cmdToken;
		j++;
		cmdToken = strtok(NULL, ">>");
	}

	cmdArr[j] = NULL;

	//Invoking pipe
	if(pipe(fd) == -1) {
		exit(1);
	}

	//Forking a process to help with redirecting input and output
	int pid = fork();

	if(pid == 0) { //Child Process
		close(fd[0]);

		dup2(fd[1], 1);
		close(fd[1]);

		char *tokArr[100];
		int i = 0;

		//Tokenizing the first half of input i.e., before ">>" and performing execvp
		char *token = strtok(cmdArr[0], " ");
		
		while(token != NULL) {
			tokArr[i] = token;
			i++;
			token = strtok(NULL, " ");
		}

		tokArr[i] = NULL;

		//If the user input contains a relative path converting that into absolute path
		char *fp = malloc(20000);
		for(int f = 0; tokArr[f] != NULL; f++) {
			//strcpy(fp, "");
			fp = strstr(tokArr[f], "~/");
			if(fp != NULL) {
				char *folder = aPath(fp);
				tokArr[f] = folder;
			}
		}

		execvp(tokArr[0], tokArr);
	} else { //Parent Process
		wait(NULL); //parent waits for child

		close(fd[1]);

		//Trimming the filename so as to remove any leading or trailing whitespaces
		char *filename = trimString(cmdArr[1]);

		char *fnt = malloc(20000);
		
		//If the user input contains a relative path converting that into absolute path
		fnt = strstr(filename, "~/");
		if(fnt != NULL) {
			char *folder = aPath(fnt);
			strcpy(filename, folder);
		}

		//printf("\n FILENAME: %s \n", filename);

		//Opening the file in append mode to append characters read from input
		int fd3 = open(filename, O_CREAT | O_RDWR | O_APPEND, 0777);

		if(fd3 == -1) {
			perror("open");
			exit(EXIT_FAILURE);
		}

		char *buff = malloc(20000 * sizeof(char));

		//Reading the characters read from the input
		long int n = read(fd[0], buff, 20000);

		//Writing the characters read from input
		if(n > 0) {
			n = write(fd3, buff, n);
			//n = write(fd3, "\n", 1);
		}
		//printf("\n BYTES WRITTEN: %d \n", n);
		close(fd3);
		close(fd[0]);
		//exit(EXIT_SUCCESS);
	}
}

//Performing redirection operation when the special character is <
void inputRedir(char *com) {
	int fd[2];
	int j = 0;
	char *inputCmdArr[2];

	char *inputCmdToken = strtok(com, "<");

	while(inputCmdToken != NULL) {
		inputCmdArr[j] = inputCmdToken;
		j++;
		inputCmdToken = strtok(NULL, "<");
	}

	inputCmdArr[j] = NULL;

	if(pipe(fd) == -1) {
		exit(1);
	}

	int pid = fork();

	if(pid > 0) {
		int status;
		wait(&status);
	} else if(pid < 0) {
		printf("\n AN ERROR HAS OCCURED WHILE FORKING. EXITING... \n");
		exit(EXIT_FAILURE);
	} else {
		//Trimming the filename so as to remove any leading or trailing whitespaces
		char *filename = trimString(inputCmdArr[1]);

		char *fnt = malloc(20000);

		//If the user input contains a relative path converting that into absolute path
		fnt = strstr(filename, "~/");
		if(fnt != NULL) {
			char *folder = aPath(fnt);
			strcpy(filename, folder);
		}

		int fd3 = open(filename, O_RDONLY);

		if(fd3 == -1) {
			perror("open");
			return;
		}

		dup2(fd3, 0);

		char *inputArr[100];
		int i = 0;
		char *inputToken = strtok(inputCmdArr[0], " ");
		while(inputToken != NULL) {
			inputArr[i] = inputToken;
			i++;
			inputToken = strtok(NULL, " ");
		}
		inputArr[i] = NULL;

		execvp(inputArr[0], inputArr);
	}
}

//Performing redirection operation when the special character is >
void outputRedir(char *com) {
	int fd[2];
	char *cmdArr[2]; //Array which stores the tokenized strings from char *com
	int j = 0;

	//Tokenizing char *com with >>
	char *cmdToken = strtok(com, ">");
	
	while(cmdToken != NULL) {
		cmdArr[j] = cmdToken;
		j++;
		cmdToken = strtok(NULL, ">");
	}

	cmdArr[j] = NULL;

	//Invoking pipe
	if(pipe(fd) == -1) {
		exit(1);
	}

	//Forking a process to help with redirecting input and output
	int pid = fork();

	if(pid == 0) { //Child Process
		close(fd[0]);

		dup2(fd[1], 1);
		close(fd[1]);

		char *tokArr[100];
		int i = 0;

		//Tokenizing the first half of input i.e., before ">>" and performing execvp
		char *token = strtok(cmdArr[0], " ");
		
		while(token != NULL) {
			tokArr[i] = token;
			i++;
			token = strtok(NULL, " ");
		}

		tokArr[i] = NULL;

		//If the user input contains a relative path converting that into absolute path
		char *fp = malloc(20000);
		for(int f = 0; tokArr[f] != NULL; f++) {
			//strcpy(fp, "");
			fp = strstr(tokArr[f], "~/");
			if(fp != NULL) {
				char *folder = aPath(fp);
				tokArr[f] = folder;
			}
		}

		execvp(tokArr[0], tokArr);
	} else { //Parent Process
		wait(NULL); //parent waits for child

		close(fd[1]);

		//Trimming the filename so as to remove any leading or trailing whitespaces
		char *filename = trimString(cmdArr[1]);

		char *fnt = malloc(20000);
		
		//If the user input contains a relative path converting that into absolute path
		fnt = strstr(filename, "~/");
		if(fnt != NULL) {
			char *folder = aPath(fnt);
			strcpy(filename, folder);
		}

		//printf("\n FILENAME: %s \n", filename);

		//Opening the file in append mode to append characters read from input
		int fd3 = open(filename, O_CREAT | O_RDWR, 0777);

		if(fd3 == -1) {
			perror("open");
			return;
		} else {
			unlink(filename);
			fd3 = open(filename, O_CREAT|O_RDWR, 0777);
		}

		char *buff = malloc(20000 * sizeof(char));

		//Reading the characters read from the input
		long int n = read(fd[0], buff, 20000);

		//Writing the characters read from input
		if(n > 0) {
			n = write(fd3, buff, n);
			//n = write(fd3, "\n", 1);
		}
		//printf("\n BYTES WRITTEN: %d \n", n);
		close(fd3);
		close(fd[0]);
		//exit(EXIT_SUCCESS);
	}
}

//Function which executes single command when the special character is either '||' or '&&'
void performCondSpecialChar(char *com) {
	char *comArr[100];
	char *token = strtok(com, " ");
	int i = 0;

	while(token != NULL) {
		comArr[i] = token;
		i++;
		token = strtok(NULL, " ");
	}

	comArr[i] = NULL;

	execvp(comArr[0], comArr);
}

//Performing Conditional Execution when the special character is either '||' or '&&'
void execCondSpecialChar(char *com) {
	char *conComArr[8];
	char copyOfCom[200000]; //Maintains a copy of com array so that it can be used to tokenize
	char *conOpArr[6]; //Array which stores the operators '||' and '&&'
	char validCom[20000];

	strcpy(validCom, "");
	strcpy(validCom, com);

	//Validating total number of commands
	char isConditionComValid = validMaxCommands(validCom, "&&||");

	if(isConditionComValid == 'N') {
		printf("\n NOT VALID \n");
		return;
	}

	strcpy(copyOfCom, "");
	strcpy(copyOfCom, com);

	printf("%s\n", copyOfCom);
	int length = (int)strlen(copyOfCom);
	int opIndex = 0;

	//Parsing the command to find the operators '||' and '&&'
	for(int index = 0; index < length; index++) {
		if(((copyOfCom[index] == '|') && (copyOfCom[index + 1] == '|'))) {
			char *op = malloc(2 * sizeof(char *));
			strcpy(op, "");
			strcat(op, "||");
			conOpArr[opIndex] = op; //Storing the operators '||' and '&&'
			opIndex++;
		} else if(((copyOfCom[index] == '&') && (copyOfCom[index + 1] == '&'))) {
			char *op = malloc(2 * sizeof(char *));
			strcpy(op, "");
			strcat(op, "&&");
			conOpArr[opIndex] = op; //Storing the operators '||' and '&&'
			opIndex++;
		}
	}
	conOpArr[opIndex] = NULL;

	int i = 0;
	char *token = strtok(com, "&&||"); //Tokenizing with '|| and &&' to separate commands
	
	while(token != NULL) {
		conComArr[i] = token;
		i++;
		token = strtok(NULL, "&&||");
	}
	conComArr[i] = NULL;

	//int j = 0;
	int execFlag = 0;
	for(int j = 0; j < i; j++) {
		int pid = fork();

		if(pid == 0) {
			char validArgsCom[20000];

			strcpy(validArgsCom, "");
			strcpy(validArgsCom, conComArr[j]);

			//Validating the number of arguments within each command
			char validArgsFlag = validArguments(validArgsCom);
			if(validArgsFlag == 'N') {
				printf("\n The argc of each command should be between >=1 && <=5 \n");
				return;
			}

			performCondSpecialChar(conComArr[j]); //Calling this function which executes each command depending on operators
		} else if(pid < 0) {
			printf("\n ERROR OCCURED WHILE FORKING \n");
			exit(EXIT_FAILURE);
		} else {
			int status;
			waitpid(pid, &status, 0);
			
			execFlag = WIFEXITED(status) && WEXITSTATUS(status) == 0; //Checking if the child process has properly exited or not to handle conditional execution

			//Loop which handles conditional execution
			while(j < opIndex) {
				if(strcmp(conOpArr[j], "&&") && execFlag == 1) { //If Conditional '&&' is successful we will move to next command
					j++;
				} else if(strcmp(conOpArr[j], "||") && execFlag == 0) { //If Conditional '||' has failed we will move to next command
					j++;
				} else {
					break;
				}
			}	
		}
	}
}

//Function which executes a process in the background
void backgoundProcess(char *com) {
	//Array which stores the commands
	char *bgPr[2];
	int i = 0;

	//Tokenizing with space to separate out commands
	char *bgPrToken = strtok(com, " ");

	while(bgPrToken != NULL) {
		bgPr[i] = bgPrToken;
		i++;
		bgPrToken = strtok(NULL, " ");
	}

	bgPr[i] = NULL;

	//Forking a child process to run a process in the background
	int bgPid = fork();

	if(bgPid == 0) {
		//Changing the process group id of the background process to it's own process id
		setpgid(0, 0);

		//Executing the process
		execlp(bgPr[0], bgPr[0], NULL);
	} else if(bgPid < 0) {
		printf("\n ERROR OCCURED WHILE FORKING \n");
		exit(EXIT_FAILURE);
	} else {
		//Maintaining a list of the background process ids
		backgroundList[bgPrIndex] = bgPid;
		bgPrIndex++;
	}
}

//Function which executes single commands sequentially as it receives them
void execSequential(char *com) {
	char *sqComArr[6];
	int i = 0;

	//Tokenizing with space to separate out arguments within each command
	char *sqToken = strtok(com, " ");

	while(sqToken != NULL) {
		sqComArr[i] = sqToken;
		i++;
		sqToken = strtok(NULL, " ");
	}

	sqComArr[i] = NULL;

	//Converting the relative path in user input to absolute path if required
	char *fp = malloc(20000);
	for(int j = 0; sqComArr[j] != NULL; j++) {
		fp = strstr(sqComArr[j], "~/");
		if(fp != NULL) {
			char *folder = aPath(fp);
			sqComArr[j] = folder;
		} 
	}

	//Forking
	int pid = fork();

	if(pid == 0) {
		execvp(sqComArr[0], sqComArr);
	} else if(pid < 0) {
		printf("\n ERROR OCCURED WHILE FORKING \n");
		exit(EXIT_FAILURE);
	}
	wait(NULL);
	
}

//Performing Sequential Execution
void seqExec(char *com) {
	char validCom[20000];

	strcpy(validCom, "");
	strcpy(validCom, com);

	char isValid = validMaxCommands(validCom, ";");

	if(isValid == 'N') {
		printf("\n A maximum of 5 commands are supported. Please try again. \n");
		return;
	}

	char *sqArr[6];
	int i = 0;

	//Tokenizing with special character ';'
	char *seqToken = strtok(com, ";");

	//Storing the tokens in an array
	while(seqToken != NULL) {
		seqToken = trimString(seqToken); //Removing leading and trailing whitespaces
		sqArr[i] = seqToken;
		i++;
		seqToken = strtok(NULL, ";");
	}

	sqArr[i] = NULL;

	//Looping through the array and performing sequential execution of commands
	for(int j = 0; sqArr[j] != NULL; j++) {
		char validArgsCom[20000];

		strcpy(validArgsCom, "");
		strcpy(validArgsCom, sqArr[j]);

		char validArgsFlag = validArguments(validArgsCom);
		if(validArgsFlag == 'N') {
			printf("\n The argc of each command should be between >=1 && <=5 \n");
			return;
		}
		execSequential(sqArr[j]);
	}
}

void foregroundProcess() {
	int status;
	
	printf("Bringing process: %d into foreground\n", backgroundList[bgPrIndex - 1]);

	/*
	* Setting the process group id pf the Background Process to the STDIN file descriptor
	* with this we can bring the background process to foreground and shell24 waits for that to complete
	*/
	if(tcsetpgrp(0, backgroundList[bgPrIndex - 1]) == -1) {
		perror("tcsetpgrp");
		return;
	}

	//Waiting for the background process to complete it's execution
	waitpid(backgroundList[bgPrIndex - 1], &status, WUNTRACED);
	bgPrIndex--;
}

//When the command is newt this function spawns a new bash terminal window and shell24 will be running in that terminal
void spawnShell24() {
	//Forking a process
	int pid = fork();

	//Child process will spawn a new bash terminal
	if(pid == 0) { 
		execlp("x-terminal-emulator", "x-terminal-emulator", "-e", "./bash_shell",  NULL);
	} else {
		//wait(NULL);
	}
}

//Handling various Special Characters in the command
void options(char *com) {
	char validCmd[10000];

	strcpy(validCmd, "");
	strcpy(validCmd, com);

	//Checks if the command entered in valid or not (Error handling)
	char validCmdFlag = isValidCommand(validCmd);

	if(validCmdFlag == 'N') {
		printf("\n Please enter a valid command \n");
		return;
	}

	char *token;
	char *comType = malloc(100);

	if((token = strchr(com, '#')) != NULL) { //When the command contains the special character '#'
		txtConcat(com); //Text File Concatenation
	} else if((token = strstr(com, "&&")) != NULL) { //When the command conatins the special character '&&'
		execCondSpecialChar(com); //Conditional Execution
	} else if((token = strstr(com, "||")) != NULL) { //When the command conatins the special character '||'
		execCondSpecialChar(com); //Conditional Execution
	} else if((token = strchr(com, '|')) != NULL) { //When the command contains the special character '|'
		createPipes(com); //Piping
	} else if((token = strstr(com, ">>")) != NULL) { //When the command contains the special character '>>'
		appendRedir(com); //Redirection mode - append
	} else if((token = strchr(com, '<')) != NULL) { //When the command contains the special character '<'
		inputRedir(com); //Redirection mode - input
	} else if((token = strchr(com, '>')) != NULL) { //When the command contains the special character '>'
		outputRedir(com); //Redirection mode - output
	} else if((token = strchr(com, '&')) != NULL) { //When the command conatins the special character '&'
		backgoundProcess(com); //Background Process execution
	} else if((token = strchr(com, ';')) != NULL) { //When the command conatins the special character ';'
		seqExec(com); //Sequential Execution
	} else if(strcmp(com, "fg") == 0) {
		foregroundProcess(); //Brings a background process to the foreground
	} else { //When the command contains no special characters
		execCommand(com); //Single command execution
	}
}

int main(int argc, char *argv[]) {
	/*
	* Declaring these 2 handlers so as to handle the background and foreground processes.
	* These will ensure that the main process will not be stopped after the background process which 
	was brought to the foreground completes it's execution.
	*/
	signal(SIGTTIN, stdinHandler);
	signal(SIGTTOU, stdoutHandler);
	
	char cmd[10000];

	while(1) {
		printf("shell24$ ");
		fgets(cmd, sizeof(cmd), stdin);

		if(cmd[strlen(cmd) - 1] == '\n') {
			cmd[strlen(cmd) - 1] = '\0';
		}

		int count = 0;

		for(int i = 0; cmd[i] != '\0'; i++) {
			if(!isspace(cmd[i])) {
				count++;
				break;
			}
		}

		if(count == 0) {
			continue;
		}

		if(strcmp(cmd, "newt") == 0) {
			//printf("\n NEWT \n");
			spawnShell24();
		} else {
			//commandBuilder(cmd);
			options(cmd);
		}
	}
}