#include<stdbool.h>
#include <stringstore.h>
#include <signal.h>

typedef struct Myptrs Myptrs;

bool public_validation(Myptrs myptrs, HttpHeader** headers, int fd); 
void write_bad_request(int fd);
void get_response(int fd, bool private, bool public,
	int* getNum, StringStore* storePri, StringStore* storePub, char* key);
void not_found_response(int fd);
void public_put_response(Myptrs myptrs, int fd, int* putNum, char* key);
void private_put_response(Myptrs myptrs, int fd, int* putNum, char* key);
void public_delete_response(Myptrs myptrs, int fd, 
	int* deleteNum, char* key);
void private_delete_response(Myptrs myptrs, int fd, 
	int* deleteNum, char* key);
void internal_server_error(int fd);
bool private_validation(Myptrs myptrs, 
	HttpHeader** headers, int fd, bool* unauthorized);
