#include  <stdio.h>
#include  <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pwd.h>


#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64
#define MAX_BACKGROUND_JOBS 64
volatile int command_counts = 1;

struct BackgroundJob {
    pid_t pid;
    char command[MAX_INPUT_SIZE];
};

struct BackgroundJob background_jobs[MAX_BACKGROUND_JOBS];
int num_background_jobs = 0;
int foreground_jobs_pid[MAX_NUM_TOKENS];
int num_foreground_jobs = 0;

void check_background_jobs() {
    int i;
    for (i = 0; i < num_background_jobs; i++) {
        pid_t pid = waitpid(background_jobs[i].pid, NULL, WNOHANG);
        if (pid == background_jobs[i].pid) {
            // The background process has finished
            printf("Background process %d (%s) has finished.\n", background_jobs[i].pid, background_jobs[i].command);
            // Remove the finished job from the list
            for (int j = i; j < num_background_jobs - 1; j++) {
                background_jobs[j] = background_jobs[j + 1];
            }
            num_background_jobs--;
            i--;
        }
    }
}

/* Splits the string by space and returns the array of tokens
*
*/
char **tokenize(char *line)
{
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  int i, tokenIndex = 0, tokenNo = 0;

  for(i =0; i < strlen(line); i++){

    char readChar = line[i];

    if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
      token[tokenIndex] = '\0';
      if (tokenIndex != 0){
	tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
	strcpy(tokens[tokenNo++], token);
	tokenIndex = 0; 
      }
    } else {
      token[tokenIndex++] = readChar;
    }
  }
 
  free(token);
  tokens[tokenNo] = NULL ;
  return tokens;
}

int arrayLength(char **tokens) {
	int i;
  int length = 0;
  for(i=0;tokens[i]!=NULL;i++){
			length++;
	}
  return length;
}

void signal_handler(int signum) {
	command_counts = 1;
	printf("\n$ ");
	fflush(stdout);

}

void kill_all_background_processes() {
    for (int i = 0; i < num_background_jobs; i++) {
        kill(background_jobs[i].pid, SIGINT); // Send SIGTERM signal to each background process
    }
}


void fork_child(char **tokens, int process_flag, int command_start_index) {
	// printf("%d process type", process_flag);
	
	int rc = fork();
	if (rc < 0) {
		fprintf(stderr, "fork failed\n");
		exit(1);
	} else if (rc == 0) {
		
		if (execvp(tokens[command_start_index], (char **)&tokens[command_start_index]) == -1) {
			perror("command not found");
			exit(1);

	}
	} else {
	// printf("inside parent before wait\n");
		if (process_flag == 0 || process_flag == 2) {
			int status;
			int rc_wait = waitpid(rc, &status, 0);
			printf("child process (%d) status: %d\n", rc_wait, status);
		} else if (process_flag == 1) {
			struct BackgroundJob bg_job;
			bg_job.pid = rc;
			strcpy(bg_job.command, tokens[0]);
			background_jobs[num_background_jobs++] = bg_job;
			setpgid(rc, rc);
		} else if (process_flag == 3) {
			foreground_jobs_pid[num_foreground_jobs++] = rc;
		}
				
	}
}

void wait_for_child(int child_pid) {
	waitpid(child_pid, NULL, 0);
}


int main(int argc, char* argv[]) {
	signal(SIGINT, signal_handler);
	char  line[MAX_INPUT_SIZE];            
	char  **tokens;              
	int i;
	
	
	


	while(1) {			
		int process_type = 0; //0 for regular foreground, 1 background, 2 sequential foreground, 3 parallel foreground
		int command_index_list[MAX_NUM_TOKENS];
		command_index_list[0] = 0;
		command_counts = 1;
		check_background_jobs();
		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line));
		printf("$ ");
		scanf("%[^\n]", line);
		getchar();

		// printf("Command entered: %s (remove this debug output later)\n", line);
		/* END: TAKING INPUT */

		line[strlen(line)] = '\n'; //terminate with new line
		tokens = tokenize(line);
		if (tokens[0] == NULL) {
			continue;
		}
		if (strcmp(tokens[0], "exit") == 0) {
			kill_all_background_processes();
			break;
		}
		if (strcmp(tokens[arrayLength(tokens) - 1], "&") == 0) {
			// printf(" & detected in the last token\n");
			process_type = 1;
			tokens[arrayLength(tokens) - 1] = '\0';
		}
		int length = arrayLength(tokens);
		for (int i = 0; i < length; i++) {
			if (strcmp(tokens[i], "&&") == 0) {
				process_type = 2;
				command_index_list[command_counts] = i + 1;
				command_counts++;
				tokens[i] = NULL;

			} else if (strcmp(tokens[i], "&&&") == 0) {
				process_type = 3;
				command_index_list[command_counts] = i + 1;
				tokens[i] = NULL;
				command_counts++;
			}
		}

		if (strcmp(tokens[0], "cd") == 0) {
			// printf("detected cd\n");
			char *new_dir = tokens[1];
			if (arrayLength(tokens) > 1 && tokens[1][0] == '~') {
				// printf("~ detected\n");
				
				const char *tilde_suffix = new_dir + 1;
				struct passwd *pw = getpwuid(getuid());
        if (pw != NULL && pw->pw_dir != NULL) {
            char *home_dir = pw->pw_dir;
            // printf("inside home: %s\n", home_dir);

            // Create a new directory path by concatenating the home directory and the suffix
            char *final_dir = malloc(strlen(home_dir) + strlen(tilde_suffix) + 1);
            if (final_dir != NULL) {
                strcpy(final_dir, home_dir);
                strcat(final_dir, tilde_suffix);
                new_dir = final_dir;
                // printf("final new dir is: %s\n", new_dir);
            } else {
                perror("malloc");
                return 1;
            }
        }
			}

			
			if (chdir(new_dir) == 0) {
				// printf("Current working directory changed to %s\n", tokens[1]);
			} else {
      perror("Shell");
  		}
			
			if (new_dir != tokens[1]) {
        free(new_dir);
    	}

		}

		

		else {
			// int rc = fork();
			// if (rc < 0) {
			// 	fprintf(stderr, "fork failed\n");
		  //  	continue;
			// } else if (rc == 0) {
			// if (execvp(tokens[0], tokens) == -1) {
			// 	perror("command not found");
			// 	return -1;

			// }
			// } else {
			// // printf("inside parent before wait\n");
			// 	if (background == 0) {
			// 		int status;
			// 		int rc_wait = waitpid(rc, &status, 0);
			// 		printf("child process (%d) status: %d\n", rc_wait, status);
			// 	} else {
			// 			struct BackgroundJob bg_job;
      //       bg_job.pid = rc;
      //       strcpy(bg_job.command, tokens[0]);
      //       background_jobs[num_background_jobs++] = bg_job;
      //       setpgid(rc, rc);
			// 	}
				
			// }

			


			for (int i = 0; i < command_counts; i++) {
				fork_child(tokens, process_type, command_index_list[i]);
			}
			if (process_type == 3) {
				for (int i = 0; i < num_foreground_jobs; i++) {
					waitpid(foreground_jobs_pid[i], NULL, 0);
				}
			}

			// fork_child(tokens, process_type, 0);

			
		}

		
		

   
   	
   	 //do whatever you want with the commands, here we just print them
		// printf("number of command: ");
		// printf("%d\n",command_counts);
		// for(i=0;tokens[i]!=NULL;i++){
		// 	printf("found token %s (remove this debug output later)\n", tokens[i]);
		// }

		for (int i = 0; i < command_counts; i++) {
			int current = command_index_list[i];
			while (tokens[current] != NULL) {
				printf("tokens[%d]: %s\n", i, tokens[current]);
				current++;
    		}
			printf("%s\n", tokens[command_index_list[i]]);
		}
       
		// Freeing the allocated memory	
		for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		}
		free(tokens);

	}
	return 0;
}
