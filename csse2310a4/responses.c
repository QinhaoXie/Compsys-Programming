#include<stdbool.h>
#include <stringstore.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <csse2310a4.h>


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

typedef struct Myptrs Myptrs;

/*public_validation()
 * ------------------------------------------
 *  test for whether a request is valid(public)
 *  Parameters:
 *      myptr: a struct that store informations for client thread
 *      headers: http headers
 *      fd: TCP file descriptor for writing data
 *  Return:
 *  	public: return true if these request data are valid
 *  	for a public database   
 *
 * */
bool public_validation(Myptrs myptrs, HttpHeader** headers, int fd) {
    bool public = true;
    if (strcmp(myptrs.method, "GET") == 0 && headers[0]) {
	public = false;
    }
    if (strcmp(myptrs.method, "DELETE") == 0 && headers[0]) {
	public = false;
    }
    if (strncmp(myptrs.address, "/public", 7) != 0) {
	    public = false;
    }
    if (headers[0]) {
	if (strcmp(headers[0]->name, "Authorization") == 0) {
	    if (headers[1]) {
		if (atoi(headers[1]->value) != strlen(myptrs.body)) {
		    public = false;
		}
	    }
	} else {
    	    if (atoi(headers[0]->value) != strlen(myptrs.body)) {
    		public = false;
    	    }
	}
    } else {
    }
	return public;
}

/*private_validation()
 * ----------------------------
 *  test for whether a request is valid(private)
 *  Parameters:
 *      myptrs: a struct that store informations for client thread
 *      headers: http headers
 *      fd: TCP file descriptor for writing data
 *      unauthorized: will be true if a 401 is responded
 *  Return:
 *  	private: return true if these request data are valid
 *  	for a private database   
 *
 * */
bool private_validation(Myptrs myptrs, HttpHeader** headers,
	int fd, bool* unauthorized) {
    char** splitedadd;
    char* headStr = "Unauthorized";
    
    splitedadd = split_by_char(myptrs.address + 1, '/', 2);
    if (strcmp(splitedadd[0], "private") == 0) {
	if (headers[0]) {
	    if (strcmp(headers[0]->value, myptrs.authStr) == 0) {
		if (strcmp(myptrs.method, "GET") == 0 && headers[1]) {
		    return false;
		}
		if (strcmp(myptrs.method, "DELETE") == 0 && headers[1]) {
		    return false;
		}
		if (headers[1]) {
		    if (atoi(headers[1]->value) != strlen(myptrs.body)) {
			return false;
		    }
		}

		return true;
	    } else {
	    //private but not correct
		char* respstr = construct_HTTP_response(401,headStr, NULL, "");
		int numBytes = strlen(respstr); 
		write(fd, respstr, numBytes);
		*unauthorized = true;
	    }
	} else {
	//private but no auth
	    int statisticus = 401;
	    char* respstr=construct_HTTP_response(statisticus
		    , headStr, NULL, "");
	    int numBytes = strlen(respstr); 
	    write(fd, respstr, numBytes);
	    *unauthorized = true;
    	    return false;
	}
    }
    return false;
}

/*write_bad_request()
 * ---------------------------------------
 *  write bad_request respose(400) to client
 *  Parameters:
 *      fd: TCP file descriptor for writing data
 *  Return:
 *      void
 *
 * */
void write_bad_request(int fd) {
    int status = 400;
    char* respstr=construct_HTTP_response(status,"Bad Request", NULL, "");
    int numBytes = strlen(respstr); 
    write(fd, respstr, numBytes);
}

/*not_found_response()
 * ---------------------------------------
 *  write not_found_response(404) to client
 *  Parameters:
 *      fd: TCP file descriptor for writing data
 *  Return:
 *      void
 *
 * */
void not_found_response(int fd) {
    int status = 404;
    char* statusExplanation = "Not Found";
    char* respstr=construct_HTTP_response(status, statusExplanation, NULL,"");
    int numBytes = strlen(respstr); 
    write(fd, respstr, numBytes);

}

/*internal_server_error()
 * ---------------------------------------
 *  write internal_server_error(500) to client
 *  Parameters:
 *      fd: TCP file descriptor for writing data
 *  Return:
 *      void
 *
 * */
void internal_server_error(int fd) {
    int status = 500;
    char* statusExp = "Internal Server Error";
    char* resp=construct_HTTP_response(status, statusExp, NULL,"");
    int numBytes = strlen(resp); 
    write(fd, resp, numBytes);

}

/*get_response()
 * --------------------------------------------
 * write a get response to client if the request is a valid 
 * public or private request
 * Parameters:
 *      fd: TCP file descriptor for writing data
 *      private: true if it is a valid private request 
 *      public: true if it is a valid public request
 *      getNum: the statistic number of successful get response
 *      storePri: private database
 *      storePub: public database
 *      key: the key for retrieve data from database
 * Return:
 *     void
 *
 * */
void get_response(int fd, bool private, bool public, int* getNum,
	StringStore* storePri, StringStore* storePub, char* key) {
    const char* dbbody = NULL;
    if (public) {
	dbbody = stringstore_retrieve(storePub, key);
    } else if (private) {
	dbbody = stringstore_retrieve(storePri, key);
    }
    if (dbbody != NULL) {
	int status = 200;
	HttpHeader** headers = malloc(sizeof(int));
	char* statusExplanation = "OK";
	char* resp = construct_HTTP_response(status, 
		statusExplanation, headers, dbbody);
	fflush(stdout);
	int numBytes = strlen(resp); 
	write(fd, resp, numBytes);
	(*getNum)++;
    } else {
	int status = 404;
	char* statusExplanation = "Not Found";
	char* respstr=construct_HTTP_response(status,
		statusExplanation, NULL,"");
	int numBytes = strlen(respstr); 
	write(fd, respstr, numBytes);
    }
}

/*public_put_response()
 * ---------------------------------------
 *  write public_put_response(200) to client
 *  Parameters:
 *      myptrs: a struct that store informations for client thread
 *      fd: TCP file descriptor for writing data
 *      putNum: the statistic number of successful put response
 *      key: a key for retrieving data from database
 *  Return:
 *      void
 *
 * */
void public_put_response(Myptrs myptrs, int fd, int* putNum, char* key) {
    if (stringstore_add(myptrs.storePub, key, myptrs.body)) {
	int status = 200;
	char* statusExplanation = "OK";
	char* resp = construct_HTTP_response(status, 
		statusExplanation, NULL, "");
	int numBytes = strlen(resp); 
	write(fd, resp, numBytes);
	(*putNum)++;//put requests plus ong
    }
}

/*private_put_response()
 * ---------------------------------------
 *  write private_put_response(200) to client
 *  Parameters:
 *      myptrs: a struct that store informations for client thread
 *      fd: TCP file descriptor for writing data
 *      putNum: the statistic number of successful put response
 *      key: a key for retrieving data from database
 *  Return:
 *      void
 *
 * */
void private_put_response(Myptrs myptrs, int fd, int* putNum, char* key) {
    if (stringstore_add(myptrs.storePri, key, myptrs.body)) {
	int status = 200;
	char* statusExplanation = "OK";
	char* resp = construct_HTTP_response(status,
		statusExplanation, NULL, "");
	int numBytes = strlen(resp); 
	write(fd, resp, numBytes);
	(*putNum)++;//put requests plus one
    }
}

/* public_delete_response()
 * ---------------------------------------
 *  write public_delete_response(200) to client
 *  Parameters:
 *      myptrs: a struct that store informations for client thread
 *      fd: TCP file descriptor for writing data
 *      deleteNum: the statistic number of successful delete response
 *      key: a key for retrieving data from database
 *  Return:
 *      void
 *
 * */
void public_delete_response(Myptrs myptrs, int fd, 
	int* deleteNum, char* key) {
    if (stringstore_delete(myptrs.storePub, key)) {
	int status = 200;
	char* statusExplanation = "OK";
	char* resp=construct_HTTP_response(status, 
		statusExplanation, NULL, "");
	int numBytes = strlen(resp); 
	write(fd, resp, numBytes);
	(*deleteNum)++;
    } else {
	not_found_response(fd);
    }
    
}

/* private_delete_response()
 * ---------------------------------------
 *  write private_delete_response(200) to client
 *  Parameters:
 *      myptrs: a struct that store informations for client thread
 *      fd: TCP file descriptor for writing data
 *      deleteNum: the statistic number of successful delete response
 *      key: a key for retrieving data from database
 *  Return:
 *      void
 *
 * */
void private_delete_response(Myptrs myptrs, int fd, 
	int* deleteNum, char* key) {
    if (stringstore_delete(myptrs.storePri, key)) {
	int status = 200;
	char* statusExplanation = "OK";
	char* resp=construct_HTTP_response(status, 
		statusExplanation, NULL, "");
	int numBytes = strlen(resp); 
	write(fd, resp, numBytes);
	(*deleteNum)++;//deleted requests plus one
    } else {
	not_found_response(fd);
    }

}


