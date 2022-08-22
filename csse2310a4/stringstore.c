#include <assert.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct StringStore StringStore;

/* A node in stringstore database */
struct StringStore {
    char* key;
    char* value;
    StringStore* next;
    StringStore* last;
    StringStore* firstNode;
};

// Create a new StringStore instance, and return a pointer to it
StringStore* stringstore_init(void) {
    StringStore* store = malloc(sizeof(StringStore));
    store->firstNode = NULL;
    store->last = NULL;
    store->next = NULL;
    store->value = NULL;
    store->key=NULL;
    return store;
} 

// Delete all memory associated with the given StringStore, and return NULL
StringStore* stringstore_free(StringStore* store) {
    if (store->firstNode == NULL) {
	// only one node, free one node
   	free(store->key); 
   	free(store->value); 
   	free(store->next);
   	free(store->last); 
   	free(store->firstNode); 
	free(store);
    } else {
	//free until last node
	StringStore* currentNode = store->firstNode; 
	while(currentNode->next != NULL) { 
	    StringStore* nextNode = currentNode->next;
	    free(currentNode->key); 
	    free(currentNode->value); 
	    free(currentNode->next);
	    free(currentNode->last); 
	    free(currentNode->firstNode); 
	    currentNode=nextNode;
	}
	//free last node
	free(currentNode->key); 
	free(currentNode->value); 
	free(currentNode->next);
	free(currentNode->last); 
	free(currentNode->firstNode); 
	free(currentNode);
    }
    return NULL;
}

// Add the given 'key'/'value' pair to the StringStore 'store'.  
// The 'key' and 'value' strings are copied with strdup() before being 
// added to the database. Returns 1 on success, 0 on failure (e.g. if
// strdup() fails).
int stringstore_add(StringStore* store, const char* key, const char* value) {
    if (store == NULL) {
	return 0;
    }
    StringStore* currentNode;
    if (store->firstNode ==NULL) {
	currentNode = store;
    } else {
	currentNode = store->firstNode;
    }
    while(currentNode->next != NULL ) {
	if (strcmp(currentNode->key, key) != 0) {
	    break;
	}
	currentNode = (*store).next;
    }
    if (currentNode->next == NULL ) {
	//it is the last node
	if (currentNode->key == NULL) {
	    //last node is empty
	    currentNode->key = strdup(key); 
	    currentNode->value = strdup(value);
	    if (currentNode->last==NULL) {
		currentNode->firstNode = currentNode; 
	    }
	    return 1;
	} else {
	    //it is last node, key didnt match
	    if (strcmp(currentNode->key, key) != 0) {
		StringStore* newNode = stringstore_init();
		newNode->key = strdup(key);
		newNode->value = strdup(value);
		newNode->last = currentNode;
		(*currentNode).next = newNode;
		return 1;
	    } else {
		//it is last node, key match
		currentNode->value = strdup(value); 
		return 1;
	    }
	}
    } else {
	if (strcmp(currentNode->key, key) == 0) {
	//it is not last node, key match
	    currentNode->value = strdup(value);
	    return 1;
	}
    }
    return 0;
}

// Attempt to retrieve the value associated with a particular 'key' in the 
// StringStore 'store'.  
// If the key exists in the database, return a const pointer to corresponding 
// value string.
// If the key does not exist, return NULL
const char* stringstore_retrieve(StringStore* store, const char* key) {
    if (store == NULL) {
	return NULL;
    }
    StringStore* currentNode;
    if (store->firstNode == NULL) {
	currentNode = store;
    } else {
	currentNode = store->firstNode;
    }

    while(currentNode->next != NULL ) {
	if (strcmp(currentNode->key, key) != 0) {
	    break;
	}
	currentNode = (*store).next;
    }
    if (currentNode->next == NULL ) {
	//it is the last node
	if (currentNode->key == NULL) {
	    //last node is empty,has no key
	    return NULL;
	} else {
	    //it is last node, key exist and didnt match
	    if (strcmp(currentNode->key, key) != 0) {
		return NULL;
	    } else {
	    //it is last node key match
		if (currentNode->value == NULL) {
		    return NULL;
		} else {
		    return (const char*)strdup(currentNode->value); 
		
		}
	    }
	}
    } else {
	if (strcmp(currentNode->key, key) == 0) {
	//it is not last node, key match
	    if (currentNode->value == NULL) {
		return NULL;
	    } else {
		return (const char*)strdup(currentNode->value); 
	    }
	}
    }

    return NULL;
}

// Attempt to delete the key/value pair associated with a particular 'key' in 
// the StringStore 'store'.
// If the key exists and deletion succeeds, return 1.
// Otherwise, return 0
int stringstore_delete(StringStore* store, const char* key) {
    if (store == NULL) {
	return 0;
    }
    StringStore* currentNode;
    if (store->firstNode == NULL) {
	currentNode = store;
    } else {
	currentNode = store->firstNode;
    }

    while(currentNode->next != NULL ) {
	if (strcmp(currentNode->key, key) != 0) {
	    break;
	}
	currentNode = (*store).next;
    }
    if (currentNode->next == NULL ) {
	//it is the last node
	if (currentNode->key == NULL) {
	    //last node is empty,has no key
	    return 0;
	} else {
	    //it is last node, key exist and didnt match
	    if (strcmp(currentNode->key, key) != 0) {
		return 0;
	    } else {
	    //it is last node key match
		currentNode->value = NULL;
		return 1;
	    }
	}
    } else {
	if (strcmp(currentNode->key, key) == 0) {
	//it is not last node, key match
	    currentNode->value = NULL;
	    return 1;
	}
    }

    return 0;

}


