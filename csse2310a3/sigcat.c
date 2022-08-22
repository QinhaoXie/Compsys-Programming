#include<stdio.h>
#include<csse2310a3.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<string.h>
#include<signal.h>
/**
 * tostdout = 1 -> set output stream to stdout
 * tostdout = 0 -> set output stream to stderr 
 * */
static int tostdout = 1;

char*  pinput(FILE* stream);
void printsig(int status);

int main(int argv, char** argc){
    char* in = malloc(sizeof(char) * 440);
    FILE* stream = stdout;
    struct sigaction sa1;
    for (int i = 1; i < 31;i++) {
	if (i != 9 && i != 19) {
	    sa1.sa_handler = printsig;
	    sa1.sa_flags = SA_RESTART;
	    sigaction(i, &sa1, NULL);
	}
    }
    do {
    	in = pinput(stream);
    } while (in[0] != '\0');
    return 0;
}

/**
 * this function is going to read inputs from stdio
 * and emit it to either stdout(when tostdout = 1)
 * or stderr(when tostdout = 0)
 * */
char* pinput(FILE* stream) {
    char* in = malloc(sizeof(char) * 430);
    fgets(in, 400, stdin);
    if (tostdout) {
    	stream = stdout;
    } else {
	stream = stderr;
    }
    if (!(in[0] == '\0')) {
	fprintf(stream, "%s", in);
    }
    fflush(stdout);
    fflush(stderr);
    return in;
}

/**
 * this is the sa_handler that print the signal received
 * and change output stream if
 * SIGUSR1 or SIGUSR2 is received
 * */
void printsig(int status) {
    printf("sigcat received %s\n", strsignal(WTERMSIG(status)));
    if (status == 10) {
	tostdout = 1;	    
    } else if (status == 12) {
	tostdout = 0;	    
    }
    fflush(stdout);
}

