#include<stdio.h>
#include<csse2310a3.h>
#include<string.h>
#include <unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include <netdb.h>
#include <errno.h>
#define GNU_SOURCE

/* a input information struct for arguments from command line*/
struct Inputs {
    const char* portNum; // portnumber
    char* key; 
    const char* value;
    int argc; // number of command line arguments
};

/*a struct that store TCP connection FILE* and file descriptors */
struct Fileptrs {
    FILE* from; // TCP FILE* to read from
    FILE* to; // TCP FILE* to write to
    int fdw; // TCP socket file discritor to write to
    int fdr; // TCP socket file discritor to read from
};

typedef struct Inputs Inputs;
typedef struct Fileptrs Fileptrs;

void read_args(int argc, char** argv, Inputs* inputs);
int valid_input(int argc, char** argv);
void connection(Inputs inputs, Fileptrs* filePtrs);
void send_message(Inputs inputs, Fileptrs filePtrs);

/*  main()
 * -------------------------------------------
 *  the main funtion of the whole program
 *  Parameters:
 *      argc: number of command line arguments 
 *      argv: a vector that contains command line arguments
 *  Return:
 *      return 0 when exit normally
 *
 * */
int main(int argc, char** argv) {
    Inputs inputs;
    Fileptrs filePtrs;
    if (valid_input(argc, argv)) {
	read_args(argc, argv, &inputs);
    }
    connection(inputs, &filePtrs);
    send_message(inputs, filePtrs);
    return 0;
}

/**
 * read_args()
 * ------------------------------------------
 * read arguments from argv.
 * Parameters:
 *      argc: number of command line arguments 
 *      argv: a vector that contains command line arguments
 *      inputs: a struct that store the command line arguments
 *  Return:
 *      void
 */
void read_args(int argc, char** argv, Inputs* inputs) {
    inputs->argc = argc;
    inputs->portNum = argv[1];
    inputs->key = argv[2];
    if (argc == 4) {
	inputs->value = argv[3];
    }
}

/**
 * valid_input()
 * ----------------------------------------
 * checking invalid input formats.
 * Parameters:
 *      argc: number of command line arguments 
 *      argv: a vector that contains command line arguments
 *  Return:
 *      return 1 when all arguments are valid
 *  Errors:
 *     exit with 1 when:
 *        ~ number of arguments are less then 2
 *        ~ key contain \n or space
 *
 */
int valid_input(int argc, char** argv) {
    if (argc <= 2) {
	fprintf(stderr, "Usage: dbclient portnum key [value]\n");
	exit(1);
    }
    if (strstr(argv[2], "\\n") == NULL) {
    } else {
	fprintf(stderr, "dbclient: key must not contain spaces or newlines\n");
	exit(1);	
    }
    
    if (strstr(argv[2]," ") == NULL) {
    } else {
	fprintf(stderr, "dbclient: key must not ");
	fprintf(stderr, "contain spaces or newlines\n");
	exit(1);	
    }

    return 1;
}

/**
 * connection()
 *--------------------------------------------
 * connect to server.
 * Parameters:
 *     inputs: a struct that store the command line arguments
 *     fptrs: a struct that store TCP connection FILE* and file descriptors
 * Return:
 *     void
 * Error:
 *     exit with code 3 when connection fail a message will 
 *     be printed to stderr
 *         ~ 
 */
void connection(Inputs inputs, Fileptrs* filePtrs) {
    struct addrinfo* ai = 0;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;        // IPv4  for generic could use AF_UNSPEC
    hints.ai_socktype = SOCK_STREAM;
    int err;
    if ((err = getaddrinfo("localhost", inputs.portNum, &hints, &ai))) {
        freeaddrinfo(ai);
        fprintf(stderr,
		"dbclient: unable to connect to port %s\n", inputs.portNum);
        exit(2);   // could not work out the address
    }
		
    int fd = socket(AF_INET, SOCK_STREAM, 0); // 0 == use default protocol
    if (connect(fd, (struct sockaddr*)ai->ai_addr, sizeof(struct sockaddr))) {
        fprintf(stderr,
		"dbclient: unable to connect to port %s\n", inputs.portNum);
	exit(2);
    }
    // fd is now connected
    // we want separate streams (which we can close independently)
    int fd2 = dup(fd);
    filePtrs->to = fdopen(fd, "w");
    filePtrs->from = fdopen(fd2, "r");
    filePtrs->fdw = fd;
    filePtrs->fdr = fd2;
}

/**
 * send_message()
 * --------------------------------------------
 * send get or put requests to server.
 * Parameters:
 *     inputs: a struct that store the command line arguments
 *     fptrs: a struct that store TCP connection FILE* and file descriptors
 * Return:
 *     void
 *
 */
void send_message(Inputs inputs, Fileptrs filePtrs) {
    if (inputs.argc == 3) {
	char* getStr = malloc(sizeof(char) * 80);
	strcat(getStr, "GET /public/");
	strcat(getStr, inputs.key);
	strcat(getStr, " HTTP/1.1\r\n\r\n");
	write(filePtrs.fdw, getStr, strlen(getStr));
	char buffer[80];
	fgets(buffer, 79, filePtrs.from);  //200 ok
	fflush(stdout);

	if (strcmp(buffer, "HTTP/1.1 200 OK\r\n") != 0) {
	    exit(3);
	} else {
	    //"recv response!!"
	}
	fgets(buffer, 79, filePtrs.from);
	int len = 0;
	sscanf(buffer, "Content-Length: %d", &len);
	fgets(buffer, 79, filePtrs.from);
	fgets(buffer, len + 1, filePtrs.from);
	fprintf(stdout, "%s\n", buffer);
	fflush(stdout);

    }

    if (inputs.argc == 4) {
	char* valuelen = malloc(sizeof(char) * 5);
	sprintf(valuelen, "%ld", strlen(inputs.value));
	char* putStr = malloc(sizeof(char) * 140);
	strcpy(putStr, "PUT /public/");
	strcat(putStr, inputs.key);
	strcat(putStr, " HTTP/1.1\r\nContent-Length: ");
	strcat(putStr, valuelen);
	strcat(putStr, "\r\n\r\n");
	strcat(putStr, inputs.value);
	write(filePtrs.fdw, putStr,strlen(putStr));
	fflush(filePtrs.to);
	char buffer[80];
	fgets(buffer, 79, filePtrs.from);
	if (strcmp(buffer, "HTTP/1.1 200 OK\r\n") != 0) {
	    exit(4);
	} else {
	    //"recv response!!"
	}
    }

}


