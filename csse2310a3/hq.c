#include<stdio.h>
#include<csse2310a3.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<string.h>
#include<signal.h>
#include<stdbool.h>
#include<ctype.h>



struct Name {
    int jobid;
    int pid;
    char* namestr;
};

struct Arg {
    char** argvs;
    int argvsc;
};

typedef struct Name Name;
typedef struct Arg Arg;
typedef void Repprt(int** arr, int jid, int status, char* name);

char*  get_cmd();
char** parsecmd(char* cmd, char*** argvs, int* argvsc, int** arr);
void spawnchild(Arg args, int** arr, int jobid, Name* childnames); 
void hdler(int sigin);
void parsespawn(int** arr, char* cmd, Arg args);
void report(int** arr, int jobid, Arg args, Name* childnames, Repprt* repprt);
void sendsignal(int** arr, char* cmd, Arg args, int jobid);
void sleeeep(int** arr, char* cmd, Arg args, int jobid);
void send(int** arr, char* cmd, Arg args, int jobid);
void rcv(int** arr, char* cmd, Arg args, int jobid);
void eof(int** arr, char* cmd, Arg args, int jobid);
void cleanup(int** arr, char* cmd, int jobid);
void reportprint(int** arr, int jid, int status, char* name);

int main(int argc, char** argv) {
    int jobid = 0;
    //don't exit Ctrl - c (sigint)
    struct sigaction sa;
    sa.sa_handler = hdler;
    sa.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa, NULL);
    //arr jobID,pid,stdin,stdout,status,status
    int** arr = malloc(sizeof(int*) * 100);
    for (int i = 0; i < 5; i++) {
   	arr[i] = malloc(sizeof(int) * 10); 
    }
    Name childnames[80];
    Arg args;
    while (true) {
    // get input from user
	char* cmd = malloc(sizeof(char) * 430);
	cmd = get_cmd();
	if (strcmp(cmd, "EOFINPUT") == 0) {
	    cleanup(arr, cmd, jobid);
	    break;
	}
	
        args.argvs = split_space_not_quote(cmd, &args.argvsc);
	parsecmd(cmd, &args.argvs, &args.argvsc, arr);
	if (strcmp(args.argvs[0], "spawn") == 0) {
	    parsespawn(arr, cmd, args);
	    if (args.argvsc > 1) {
	    spawnchild(args, arr, jobid, childnames);
	    jobid++;
	    }
	} else if (strcmp(args.argvs[0],"report") == 0) {
	    report(arr, jobid, args, childnames, &reportprint);
	} else if (strcmp(args.argvs[0], "signal") == 0) {
	    sendsignal(arr, cmd, args, jobid);
	} else if (strcmp(args.argvs[0], "sleep") == 0) {
	    sleeeep(arr, cmd, args, jobid);
	} else if (strcmp(args.argvs[0], "send") == 0) {
	    send(arr, cmd, args, jobid);
	} else if (strcmp(args.argvs[0], "rcv") == 0) {
	    rcv(arr, cmd, args, jobid);
	} else if (strcmp(args.argvs[0], "eof") == 0) {
	    eof(arr, cmd, args,jobid);
	} else if (strcmp(args.argvs[0], "cleanup") == 0) {
	    cleanup(arr, cmd, jobid);
	}
    }
    
    
    return 0;
}

/**
 * fuction for hq sigal handling.
 */ 
void hdler(int sigin) {
}

/**
 * fuction for read a command from stdin.
 */ 
char*  get_cmd() {
    char* in = malloc(sizeof(char) * 130);
    do {
	printf("> ");    
	fflush(stdout);
	in = read_line(stdin);
	if (in == NULL) {
	    return "EOFINPUT";
	}
    } while (strcmp(in, "\0") == 0);
    //printf("%s\n",in);
    return in;
}

/**
 * fuction for spawning child
 */ 
void spawnchild(Arg args, int** arr, int jobid, Name* childnames) {
    int fd[2];//child read from fd[0](stdin)
    int fd2[2];//child write from fd2[1](stdout)
    pid_t pid;
    //int status;
    pipe(fd);
    pipe(fd2);
    printf("New Job ID [%d] created\n", jobid);
    if (pid = fork(), pid <= 0) {
    //arr jobID,pid,stdin,stdout,status
    //child
    	close(fd[1]);//child close write,only read from fd[0]
	close(fd2[0]);//child close read,only write to fd2[1]
	dup2(fd[0], STDIN_FILENO);
	close(fd[0]);
	dup2(fd2[1], STDOUT_FILENO);
	close(fd2[1]);
	execvp(args.argvs[1], args.argvs + 1);
	exit(99);
    } else {
    //parent
	close(fd[0]);//parent close read,only write to fd
	close(fd2[1]);//child close write,only read from fd2
    }
    //fd[1] is stdin to child.
    //fd2[0] is stdout from child.
    //arr jobID,pid,stdin,stdout,
   //printf("%d\n",status);
    arr[jobid][0] = jobid;
    arr[jobid][1] = pid;
    arr[jobid][2] = fd[1];//write(parent),stdin(child)
    arr[jobid][3] = fd2[0];//read(parent),stdout(child)
    //arr[jobid][4] = status;
    childnames[jobid].jobid = jobid;
    childnames[jobid].pid = pid;
    childnames[jobid].namestr = malloc(sizeof(char) * 50);
    strcpy((childnames[jobid].namestr), args.argvs[1]);
}

/**
 * fuction for checking cmd
 */ 
char**  parsecmd(char* cmd, char*** argvs, int* argvsc, int** arr) {
//printf("function parsecmd");
    //char** args.argvs; 
    //*args.argvs = split_space_not_quote(cmd, args.argvsc);
    
    char* cmdArr[8];
    cmdArr[0] = "spawn"; 
    cmdArr[1] = "report"; 
    cmdArr[2] = "signal"; 
    cmdArr[3] = "sleep"; 
    cmdArr[4] = "send"; 
    cmdArr[5] = "rcv"; 
    cmdArr[6] = "eof"; 
    cmdArr[7] = "cleanup"; 
    _Bool ivacmd = true;
    for (int i = 0; i < 8; i++) {
	if (strcmp(*argvs[0], cmdArr[i]) == 0) {
	//printf("%s\n",cmdArr[i]);
	ivacmd = false;
	}
    }
    if (ivacmd == true) {
	printf("Error: Invalid command\n");
    }
    return *argvs;
   
}

/**
 * fuction for checking spawn cmd
 */ 
void parsespawn(int** arr, char* cmd, Arg args) {
    if (args.argvsc <= 1) {
	printf("Error: Insufficient arguments\n");
    }
    
}

/**
 * fuction for report cmd
 */ 
void report(int** arr, int jobid, Arg args, Name* childnames, Repprt* repprt) {
    _Bool digitcheck = true;
    int status;
    if (args.argvsc == 1) {
	printf("[Job] cmd:status\n");
	for (int i = 0; i < jobid; i++) {//all pid
	    char* name = childnames[i].namestr;
	    if ((arr[i][4] == 0) || (!kill(arr[i][1], 0))) {
		waitpid(arr[i][1], &status, WCONTINUED | WNOHANG);
	        arr[i][4] = status;
	    } else if (arr[i][4] == 0) {
		waitpid(arr[i][1], &status, 0);
	        arr[i][4] = status;
	    }
	    status = arr[i][4];
	    repprt(arr, i, status, name);
	}
    } else if (args.argvsc > 1) {// specific pid
	if ((strlen(args.argvs[1]) == 0) || (atoi(args.argvs[1]) >= jobid)) {
	    digitcheck = false;
	}	
	for (int i = 0; i < strlen(args.argvs[1]); i++) {
	    if (isdigit(args.argvs[1][i]) == 0) {//if is not digit,return 0;
		digitcheck = false;
	    }
	}
	if (digitcheck) {
	    int jid = atoi(args.argvs[1]);//jid -- jobid 
	    char* name = childnames[jid].namestr;
	    if (arr[jid][4] == 0) { 
		waitpid(arr[jid][1], &status, WCONTINUED | WNOHANG);
	        arr[jid][4] = status;
      	    }
	    status = arr[jid][4];
	    printf("[Job] cmd:status\n");
	    repprt(arr, jid, status, name);
	} else {
	    printf("Error: Invalid job\n");
	} 
    }
}

/**
 * fuction for printing report 
 */
void reportprint(int** arr, int jid, int status, char* name){
    if (!kill(arr[jid][1], 0)) {
	printf("[%d] %s:running\n", jid, name);
    } else if (WIFEXITED(status)) {
	printf("[%d] %s:exited(%d)\n", jid, name, WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
	printf("[%d] %s:signalled(%d)\n", jid, name, WTERMSIG(status));
    }
}

/**
 * fuction for sending signal to a child
 */ 
void sendsignal(int** arr, char* cmd, Arg args, int jobid) {
    _Bool digitcheck = true;
    if (args.argvsc < 3) {
	printf("Error: Insufficient arguments\n");
    } else if (atoi(args.argvs[1]) > jobid - 1) {
	printf("Error: Invalid job\n");
    } else {
	for (int i = 0; i < strlen(args.argvs[1]); i++) {
	    if (isdigit(args.argvs[1][i]) == 0) {//if is not digit,return 0;
		digitcheck = false;
	    }
	}
	
	if (strlen(args.argvs[1]) == 0) {
	    digitcheck = false;
	}
	if (digitcheck) {

	    int job = atoi(args.argvs[1]);
	    int signum = atoi(args.argvs[2]);
	    if (signum <= 31 && signum >= 1) {
		kill(arr[job][1], signum);
	    } else {
		printf("Error: Invalid signal\n");
	    } 
	} else {
	    printf("Error: Invalid job\n");
	}
    }
}

/**
 * fuction for sleep cmd
 */ 
void sleeeep(int** arr, char* cmd, Arg args, int jobid) {
    _Bool digitcheck = true;
    
    //printf("%d args.argvsc", args.argvsc);
    if (args.argvsc < 2) {
	printf("Error: Insufficient arguments\n");
    } else {
	if (strlen(args.argvs[1]) == 0) {
	    digitcheck = false;
	}	
	for (int i = 0; i < strlen(args.argvs[1]); i++) {
	    if (isdigit(args.argvs[1][i]) == 0) {//if is not digit,return 0;
		digitcheck = false;
	    }
	}
	if (digitcheck) {
	    usleep(strtod(args.argvs[1], NULL)*1000000);
	} else {
	    printf("Error: Invalid sleep time\n");
	}
    }
    
}

/**
 * fuction for send string to child's stdin
 */ 
void send(int** arr, char* cmd, Arg args, int jobid) {
    _Bool digitcheck = true;
    if (args.argvsc < 3) {
	printf("Error: Insufficient arguments\n");
    } else { 
	if (atoi(args.argvs[1]) >= jobid) {
	    printf("Error: Invalid job\n");
	}
	if (strlen(args.argvs[1]) == 0) {
	    digitcheck = false;
	}	
	for (int i = 0; i < strlen(args.argvs[1]); i++) {
	    if (isdigit(args.argvs[1][i]) == 0) {//if is not digit,return 0;
		digitcheck = false;
	    }
	}
	if (digitcheck) {
	    int job = atoi(args.argvs[1]);
	    //for (int i = 0; i < args.argvsc - 1; i++) {
    	    dprintf(arr[job][2], "%s\n", args.argvs[2]);//}
	} else {
	    printf("Error: Invalid job\n");
	}
    }
}

/**
 * fuction for reading a line from a child
 */ 
void rcv(int** arr, char* cmd, Arg args, int jobid) {
    _Bool digitcheck = true;
    if (args.argvsc < 2) {
	printf("Error: Insufficient arguments\n");
    } else if (atoi(args.argvs[1]) > jobid - 1) {
	printf("Error: Invalid job\n");
    } else {

	if (strlen(args.argvs[1]) == 0) {
	    digitcheck = false;
	}	
	for (int i = 0; i < strlen(args.argvs[1]); i++) {
	    if (isdigit(args.argvs[1][i]) == 0) {//if is not digit,return 0;
		digitcheck = false;
	    }
	}
	if (digitcheck) {
		    //
	    int job = atoi(args.argvs[1]);
	    FILE* fptr = fdopen(arr[job][3], "r");
	    if (!is_ready(arr[job][3])) {
		printf("<no input>\n");
	    } else {
		char* oneline = read_line(fptr);
		if (oneline == NULL) {
		    printf("<EOF>\n");
		} else if (strcmp(oneline, "\0") == 0) {
		    printf("<no input>\n");
		} else {
		    printf("%s\n", oneline);
		}
	    }
	//
	} else {
	    printf("Error: Invalid job\n");
	}
    }
}

/**
 * fuction for closing child's file descriptor.
 */ 
void eof(int** arr, char* cmd, Arg args, int jobid) {
    _Bool digitcheck = true;
    if (args.argvsc < 2) {
	printf("Error: Insufficient arguments\n");
    } else if (atoi(args.argvs[1]) > jobid - 1) {
	printf("Error: Invalid job\n");
    } else {
	if (strlen(args.argvs[1]) == 0) {
	    digitcheck = false;
	}	
	for (int i = 0; i < strlen(args.argvs[1]); i++) {
	    if (isdigit(args.argvs[1][i]) == 0) {//if is not digit,return 0;
		digitcheck = false;
	    }
	}
	if (digitcheck) {
	    int job = atoi(args.argvs[1]);
	    close(arr[job][2]);
	} else {
	    printf("Error: Invalid job\n");
	}
    }
}

/**
 * fuction for killing all children.
 */ 
void cleanup(int** arr, char* cmd, int jobid) {
    int pid;  
    //token = strtok(cmd," ");
    for (int i = 0; i <= jobid - 1; i++) {
	if (!kill(arr[i][1], 0)) {
	    pid = kill(arr[i][1], 9); 
	    waitpid(pid, &arr[i][4], 0);
	}
    }
}






