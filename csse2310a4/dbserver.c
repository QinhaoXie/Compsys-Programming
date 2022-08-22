#include<stdbool.h>
#include <sys/types.h>
#include <csse2310a4.h>
#include <csse2310a3.h>
#include <stringstore.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include "responses.h"

/*input infomation from command line arguments */
struct Inputs {
    char* authstr;
    int connection;
    const char* portnum;
    int argc;
};

/* statistic data for SIGHUP signal */
struct Statistic {
    int connectedNum;
    int completedNum;
    int authFailNum;
    int getNum;
    int putNum;
    int deleteNum;
    sigset_t set;
};

typedef struct Statistic Statistic;

/* pointers will be sent to client thread */
struct Myptrs {
    int sockfdptr;
    StringStore* storePub;
    StringStore* storePri;
    char* authStr;
    Statistic* statistic;
    pthread_mutex_t* mutex;
    char* method;
    char* address;
    char* body;           
};

typedef struct Inputs Inputs;

void* sig_thread(void* arg);
void process_connections(int fdServer, char* authstr, Statistic* statistic);
void* client_thread(void* arg);
int open_listen(const char* port, Inputs* inputs);
void valid_check(int argc, char** argv);
void read_args(int argc, char** argv, Inputs* inputs);

/*
 *   main
 * ---------------------------------------
 * main function for the database server
 * parameter:
 *     argc: number of command line arguments 
 *     argv: a vector that contains command line arguments
 * return:
 *     0: exit normally
 * */
int main(int argc, char** argv) {
 /* Block SIGHUP; xother threads created by main()
              will inherit a copy of the signal mask. */
    pthread_t thread;
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGHUP);
    Statistic statistic;
    statistic.connectedNum = 0;
    statistic.completedNum = 0;
    statistic.authFailNum = 0;
    statistic.getNum = 0;
    statistic.putNum = 0;
    statistic.deleteNum = 0;
    statistic.set = set;
    pthread_sigmask(SIG_BLOCK, &set, NULL);
    pthread_create(&thread, NULL, &sig_thread, (void *)&statistic);

    Inputs inputs;
    valid_check(argc, argv);
    read_args(argc, argv, &inputs);
    int fdServer;
    fdServer = open_listen(inputs.portnum, &inputs);
    fprintf(stderr, "%s\n", inputs.portnum);
    fflush(stderr);
    process_connections(fdServer, inputs.authstr, &statistic);
    return 0;
}

/* valid_check()
 * --------------------------------------
 *  valid_check function checks the arguments 
 *  passed into the program
 *  Parameters:
 *      argc: number of command line arguments 
 *      argv: a vector that contains command line arguments
 *  Return:
 *	void
 *  Errors:
 *      exit with code 1 if:
 *          ~ no authfile or connections number specified
 *          ~ the connections argument is not a non negative integer
 *          ~ the port number argument is not integer or its value 
 *          	is not 0 or number between [1024-65535]
 *          ~ too many arguments supplied
 *      exit with code 2 if:
 *          ~ the file cannot be opened for reading
 *          ~ the first line of the file is empty
 *
 * */
void valid_check(int argc, char** argv) {
    char* usageErr = "Usage: dbserver authfile connections [portnum]\n";
    const int minimumPort = 1024;
    const int maximumPort = 65535;
    if (argc < 3) { // only one argument is optional
        fprintf(stderr, usageErr);
        exit(1);
    }
    if (argc > 4) { //number of arguments should not be more than 4
        fprintf(stderr, usageErr);
        exit(1);
    }
    int connection = atoi(argv[2]);
    if (connection < 0) { //checking negative connection number
        fprintf(stderr, usageErr);
        exit(1);
    }
    if (argc == 4) { //argument should between [1024,65535] 
 	int portnum = atoi(argv[3]);
	if (portnum <= minimumPort || portnum >= maximumPort) {
	    fprintf(stderr, usageErr);
	    exit(1);
	}
		
    }
    //check whether connection and port number are digits
    for (int i = 2; i <= (argc - 1); i++) {
	for (int j = 0; j < strlen(argv[i]);j++) {
	    if (!isdigit(argv[i][j])) {
		fprintf(stderr, usageErr);
		exit(1);
	    } 
	}
    }
    char* path = argv[1];
    FILE* file = fopen(path, "r");
    if (file == NULL) { //annot open file
	fprintf(stderr, "dbserver: unable to read authentication string\n");
	exit(2);
	fclose(file);
    }
    if (read_line(file) == NULL) { // first line is empty
	fprintf(stderr, "dbserver: unable to read authentication string\n");
	exit(2);
	fclose(file);
    }
}

/* read_args()
 * -----------------------------------------------
 *  read arguments into the inputs struct
 *  this function will only be excuted after "valid_check()"
 *  Parameters:
 *      argc: number of command line arguments 
 *      argv: a vector that contains command line arguments
 *      inputs: a struct that store the command line arguments
 *  Return:
 *	void
 *
 * */
void read_args(int argc, char** argv, Inputs* inputs) {
    inputs->authstr = read_line(fopen(argv[1], "r"));
    inputs->connection = atoi(argv[2]);
    inputs->argc = argc;
    if (argc == 4) {
	inputs->portnum = argv[3];
    } else {
	inputs->portnum = "0";
    }
}

/* open_listten()
 * -----------------------------------------
 * Listens on given port. 
 * Parameters:
 *      port: the port passed in from command line
 *      inputs: a struct that store the command line arguments
 * Retrun:
 *     return the listening socket
 * Errors:
 *     failure when connot open sockets on given port.
 * */
int open_listen(const char* port, Inputs* inputs)
{
    struct addrinfo* ai = 0;
    struct addrinfo hints;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;   // IPv4
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;    // listen on all IP addresses

    int err;
    if ((err = getaddrinfo(NULL, port, &hints, &ai))) {
        freeaddrinfo(ai);
        fprintf(stderr, "%s\n", gai_strerror(err));
        return 1;   // Could not determine address
    }

    // Create a socket and bind it to a port
    int listenfd = socket(AF_INET, SOCK_STREAM, 0); // 0=default protocol (TCP)

    // Allow address (port number) to be reused immediately
    int optVal = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, 
	    &optVal, sizeof(int)) < 0) {
        perror("Error setting socket option");
        exit(1);
    }

    if (bind(listenfd, (struct sockaddr*)ai->ai_addr, 
	    sizeof(struct sockaddr)) < 0) {
       	fprintf(stderr, "dbserver: unable to open socket for listening\n"); 
	fflush(stdout);
	exit(3);
    }

    if (listen(listenfd, inputs->connection) < 0) {  // Up to given number of 
					//connection requests can queue
        perror("Listen");
        return 4;
    }
	
    // Have listening socket - return it
    return listenfd;
}

/*
 * process_connections()
 * -------------------------------------------
 *  Parameters:
 *      fdServer: the filedescriptor of the TCP conncetion
 *      authstr: first line read from the authentification file
 *      statistic: the statistic struct for counting connection
 *  Return:
 *      void
 *
 * */
void process_connections(int fdServer, char* authstr, Statistic* statistic)
{
    int fd;
    struct sockaddr_in fromAddr;
    socklen_t fromAddrSize;
    // Repeatedly accept connections and process data (capitalise)
    StringStore* store1 = stringstore_init();
    StringStore* store2 = stringstore_init();
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex,NULL);
    while (1) {
        fromAddrSize = sizeof(struct sockaddr_in);
	// Block, waiting for a new connection. (fromAddr will be populated
	// with address of client)
        fd = accept(fdServer, (struct sockaddr*)&fromAddr, &fromAddrSize);
        char* openSocketErr = 
		"dbserver: unable to open socket for listening\n";
	if (fd < 0) {
     	    fprintf(stderr, openSocketErr); 
      	    fflush(stdout);
            exit(3);
        }
     
	// Turn our client address into a hostname and print out both 
        // the address and hostname as well as the port number
        char hostname[NI_MAXHOST];
        int error = getnameinfo((struct sockaddr*)&fromAddr, 
                fromAddrSize, hostname, NI_MAXHOST, NULL, 0, 0);
        if (error) {
            fprintf(stderr, "Error getting hostname: %s\n", 
                    gai_strerror(error));
        } else {
        }
	statistic->connectedNum++;
	int* fdPtr = malloc(sizeof(int));
	*fdPtr = fd;
	Myptrs myptrs;
	myptrs.storePub = store1;
	myptrs.storePri = store2;
	myptrs.sockfdptr = fd;
	myptrs.authStr = authstr;
	myptrs.statistic = statistic;
	myptrs.mutex = &mutex;
	pthread_t threadId;
	pthread_create(&threadId, NULL, client_thread, &myptrs);
	pthread_detach(threadId);
    }
    pthread_mutex_destroy(&mutex);
}

/*
 * client_thread()
 * --------------------------
 * thread function for proccessing requests from client
 * the thread will be able to write or read from the database and
 * send back value.
 * 
 *
 * Parameters:
 *     arg: pointer receiving from pthread_create()
 * Return:
 *     void
 *
 * */
void* client_thread(void* arg) {
    Myptrs myptrs = *(Myptrs*)arg;
    int fd = myptrs.sockfdptr;
    Statistic* stat = myptrs.statistic;
    HttpHeader** headers; 
    while (get_HTTP_request(fdopen(fd, "r"), &(myptrs.method), 
	    &(myptrs.address), &headers, &(myptrs.body))) {             
	char* key = split_by_char(myptrs.address + 1, '/', 2)[1];
	bool unauthorized = false;
	bool public = public_validation(myptrs, headers,fd);
	bool private = private_validation(myptrs, headers, fd, &unauthorized);
	if (unauthorized) {
	    continue;
	    stat->authFailNum++;
	}
	if (strcmp(myptrs.method,"GET") == 0 && (public || private)) {
	    pthread_mutex_lock(myptrs.mutex);
	    get_response(fd, private, public, &stat->getNum,
		    myptrs.storePri, myptrs.storePub, key);
	    pthread_mutex_unlock(myptrs.mutex);
	} else if ((!strcmp(myptrs.method,"PUT")) && (public || private)) {
	    pthread_mutex_lock(myptrs.mutex);
	    if (public) { //public requests
		public_put_response(myptrs, fd, &stat->putNum, key);
	    } else if (private) { //private requests
		private_put_response(myptrs, fd, &stat->putNum, key);    
	    } else {
		internal_server_error(fd);
	    }
	    pthread_mutex_unlock(myptrs.mutex);
	} else if ((!strcmp(myptrs.method,"DELETE")) && (public || private)) {
	    pthread_mutex_lock(myptrs.mutex);
    	    if (public) { //public requests
		public_delete_response(myptrs, fd, 
			&stat->deleteNum, key);	
	    } else if (private) { //private requests
		private_delete_response(myptrs, fd, 
			&stat->deleteNum, key);	
	    }
	    pthread_mutex_unlock(myptrs.mutex);
	} else {
	    write_bad_request(fd);//("Got EOF or badly formed request\n"); 
	}
    } 
    close(fd); //should clean memory here..client disconnected
    stat->completedNum++;
    return NULL;	// could have called pthread_exit(NULL);
}

/*
 * sig_thread()
 * --------------------------------------------
 *  the function will be used to create a delicated thread for 
 *  signal handleing. upon receiving SIGHUP signal, the threaad will
 *  write statistics to stderr.
 *
 *  Parameters:
 *      arg: pointer passed from pthread_create(statistic struct)
 *  Return:
 *      void
 *
 * */
void* sig_thread(void* arg) {
    Statistic* statisticistic = (Statistic*)arg;
    const sigset_t set = statisticistic->set;
    int sig;
    for (;;) {
	sigwait(&set, &sig);
	fprintf(stderr, "Connected clients:%d\n",
		statisticistic->connectedNum);
	fprintf(stderr, "Completed clients:%d\n", 
		statisticistic->completedNum);
	fprintf(stderr, "Auth failures:%d\n", 
		statisticistic->authFailNum);
	fprintf(stderr, "GET operations:%d\n", 
		statisticistic->getNum);
	fprintf(stderr, "PUT operations:%d\n", 
		statisticistic->putNum);
	fprintf(stderr, "DELETE operations:%d\n", 
		statisticistic->deleteNum);
	fflush(stderr);
    }
}






