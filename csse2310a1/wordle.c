#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <csse2310a1.h>
#include <errno.h>
#include <ctype.h>
//function 1
//rerurn int[2], length and max.
int* read_argv_1(int argc, char** argv, int (*allisdigit)(char*));

void print_instruction(int atpRem, int wordLen);
//aisa:allisalpha function
int isvalid(char* word, char* dictionary, int wordLen, int (*aisa)(char*));
char* read_dictionary(int argc, char** argv);
void save_a_word(char* iptVal, char** atpArr, int atpRem, int totalAtp);
int allisdigit(char* str);
int allisalpha(char* str);
void hints(char* iptVal, char* word, char** atpArr, int atpRem, int totalAtp);
char* get_userinput(int wordLen, char* word);

//main fuction
int main(int argc, char** argv){
	//geting arguments
    int* actualArg = read_argv_1(argc, argv, &allisdigit);
	//initializing game
    int totalAtp = actualArg[1];
    int atpRem = actualArg[1];
    int gameOver = 0;
	//read dictionary
    char* dictionary = read_dictionary(argc, argv);
    printf("Welcome to Wordle!\n");
	//getword
    int wordLen = actualArg[0]; 
    char* word = get_random_word(wordLen);
    char* word1 = malloc(sizeof(char) * 53);
    strcpy(word1, word);
    strcat(word1, "\n");
	
	//saving_structure
    char** atpArr = malloc(sizeof(char*) * totalAtp);
    for (int i = 0; i < totalAtp;i++) {
    	atpArr[i] = malloc(sizeof(char) * (wordLen + 1));
    }
	//loop for the game
    while (!gameOver) {
	//test word
	print_instruction(atpRem, wordLen);
	//stdio
	char* iptVal = (char*)malloc(sizeof(char) * (wordLen + 50));	
	strcpy(iptVal, get_userinput(wordLen, word));
	if (strcasecmp(iptVal, word1) == 0) {
	    printf("Correct!");
	    gameOver = 1;
	} else {
	    if (isvalid(iptVal, dictionary, wordLen, &allisalpha)) {
		save_a_word(iptVal, atpArr, atpRem, totalAtp);
		hints(iptVal, word, atpArr, atpRem, totalAtp);
		atpRem--;
	    }
	}
	
	if (atpRem == 0) {
	    gameOver = 1;
    	    fprintf(stderr, "Bad luck - the word is \"%s\".\n",word);	    
    	    exit(3);
	
	}	
    }	
}
//end of main
















//rerurn int[2], length and max.
int* read_argv_1(int argc, char** argv, int (*aisd)(char*)){

    int length = 5,lenPlace = 0;
    int max = 6,maxPlace = 0;
    int flag = 0;
    char* usageMes = malloc(sizeof(char) * 90);
    strcat(usageMes, "Usage: wordle [-len word-length] [");
    strcat(usageMes, "-max max-guesses] [dictionary]\n");
//char* dictionary = "\0";
    //read arguments
    for (int i = 1; i < argc;i++) {
	if (strcmp(argv[i], "-len") == 0) {
	    flag++;
	    lenPlace = i;
	};
	if (strcmp(argv[i], "-max") == 0) {
 	    maxPlace = i;
 	    flag++;
	};
    }
	// validation for arg number too much
    if (argc > (flag * 2 + 2)) {
	// printf("number of flag: %d|%d|%d",flag,flag*2+2,argc);
	fprintf(stderr, usageMes);
	exit(1);
    }
	// validation for arg number too little 
    if (argc < flag * 2 + 1) {
	//printf("number of flag: %d|%d|%d",flag,flag*2+2,argc);
	fprintf(stderr, usageMes);
	exit(1);
    }
    for (int i = 1; i < argc; i++) {
	//validation
	//printf("1234");
	/*only 1 arg and = -max*/
	if (strcmp(argv[i], "-max") == 0 && argc == 2) {
	    fprintf(stderr, usageMes);
	    exit(1);
	}
	/*only 1 arg and = -len*/
	if (strcmp(argv[i], "-len") == 0 && argc == 2) {
	    fprintf(stderr, usageMes);
	    exit(1);
	}
	/*=-max and next is not digits*/
	if (strcmp(argv[i], "-max") == 0 && aisd(argv[i + 1]) == 0) {
	    fprintf(stderr, usageMes);
	    exit(1);
	}
	/*=-len and next is not digits*/
	if (strcmp(argv[i], "-len") == 0 && aisd(argv[i + 1]) == 0) {
	    fprintf(stderr, usageMes);
	    exit(1);
	}
	/*nither -len and -max nut start with '-'*/
	//_Bool argviislen=(strcmp(argv[i], "-len")) == 0;
	//_Bool argviismax=(strcmp(argv[i], "-max")) == 0;
	if (strcmp(argv[i], "-len") != 0 && strcmp(argv[i], "-max") != 0 && strncmp(argv[i], "-", 1) == 0) {
	    fprintf(stderr, usageMes);
	    exit(1);
	};
	/*   -len -max （no arg between)*/
	if (strcmp(argv[i - 1], "-len") == 0 && strcmp(argv[i], "-max") == 0) {
	    fprintf(stderr, usageMes);
	    exit(1);
	};
	/*   -max -len（no arg between) */
	if (strcmp(argv[i - 1], "-max") == 0 && strcmp(argv[i], "-len") == 0) {
	    fprintf(stderr, usageMes);
	    exit(1);
	};
	if (maxPlace >= 3) {
	/*   -max -max （no arg between)*/
	    if (strcmp(argv[maxPlace - 2], "-max") == 0 && strcmp(argv[maxPlace], "-max") == 0) {
		fprintf(stderr, usageMes);
		exit(1);
	    };
	}
	if (lenPlace >= 3) {
	/*   -len -len（no arg between)*/
	    if (strcmp(argv[lenPlace - 2], "-len") == 0 && strcmp(argv[lenPlace], "-len") == 0) {
		fprintf(stderr, usageMes);
		exit(1);
	    };
	}
	/* two identical arg   */
	if (strcmp(argv[i - 1], argv[i]) == 0) {
	    fprintf(stderr, usageMes);
	    exit(1);
	};
	/* empty arg   */
	if (strcmp(argv[i], "") == 0) {
	    fprintf(stderr, usageMes);
	    exit(1);
	};
	//end of validation
	
	if (strcmp(argv[i], "-len") == 0) {
	    //lenPlace = i;
	    length = atoi(argv[i + 1]);
	};
	if (strcmp(argv[i], "-max") == 0) {
	   //maxPlace = i;
	    max = atoi(argv[i + 1]);
	};
	//length and max value validation
	if (length < 3 || length > 9 || max < 3 || max > 9) {
    	    fprintf(stderr, usageMes);
    	    exit(1);
	}
	// end of validation
    }
    int* arg = (int*)malloc(sizeof(int) * 2);
    arg[0] = length;
    arg[1] = max;
    return arg;
}

//print message for new attemps
void print_instruction(int atpRem, int wordLen){
    if (atpRem > 1) {
	printf("Enter a %d letter word (%d attempts remaining):\n",wordLen, atpRem);
    } else {
	printf("Enter a %d letter word (last attempt):\n",wordLen);
    };
}

int isvalid(char* word, char* dictionary, int wordLen, int (*aisa)(char*)) {
//checking:
//should all be letter a-z
//should be the same length

    _Bool lastcharnewln = word[strlen(word) - 1] == '\n';      
    _Bool lennotequal = (strlen(word) - 1) != wordLen;
    if ((lastcharnewln && lennotequal) || (!lastcharnewln && (strlen(word)) != wordLen)) {
	printf("Words must be %d letters long - try again.\n",wordLen);
	return 0;

    } else if (aisa(word) == 0) {
	printf("Words must contain only letters - try again.\n");
	return 0;

    } else if (strcasestr(dictionary, word) == 0) {
	printf("Word not found in the dictionary - try again.\n");
    
	return 0;

    }

    return 1;
}

// read_dictionary fun
char* read_dictionary(int argc, char** argv){
    char* dictionary = (char*)malloc(sizeof(char) * 600);
    char buffer[300];
    char* path;
    int readCount = 0;
    char* usageMes = malloc(sizeof(char) * 90);
    strcat(usageMes, "Usage: wordle [-len word-length] [");
    strcat(usageMes, "-max max-guesses] [dictionary]\n");
//char* path = "/usr/share/dict/words";
//printf("argc in dict fun%d\n",argc%2);
    if (argc % 2 == 0) {
	if (strcmp(argv[argc - 1], "/usr/share/dict/words") == 0) {
	    fprintf(stderr, usageMes);
	    exit(1);
	}
	path = argv[argc - 1];
    } else if (argc % 2 != 0) {
	path = "/usr/share/dict/words";
    }

    FILE* fin = fopen(path, "r");
    if (fin == 0) {
	char* prefix = "wordle: dictionary file \"";
	char* suffix = "\" cannot be opened\n";
	char* mes = malloc(sizeof(char) * 30);
	strcat(mes, prefix);
	strcat(mes, path);
	strcat(mes, suffix);
	fprintf(stderr, "%s",mes);
//perror(mes);

	exit(2);
    }

    do {
	readCount++;
//fgets(buffer,30,fin);
	fread(buffer, 300, 1, fin);
//dictionary = realloc(dictionary,)
	dictionary = realloc(dictionary, sizeof(char) * 300 * (readCount + 2));
	dictionary = strcat(dictionary, buffer);
    } while (!feof(fin));

//printf("word_count of dictionary %d\n",word_count);
//printf("path: %s\n",path);
//printf("dictionary: %s\n",dictionary);
//printf("bufferdictionary: %s\n",buffer);

    return dictionary;
}

//save a word to atpArray function
void save_a_word(char* iptVal, char** atpArr, int atpRem, int totalAtp){
    for (int i = 0; i < strlen(iptVal);i++) {
	atpArr[totalAtp - atpRem][i] = iptVal[i];
    }
 // below are printing code
    /*  
    	printf("----history guessing\n");
 for (int i = 0; i < totalAtp;i++) {
    for (int k = 0; k < strlen(iptVal);k++) {
//		printf("k:%d, i: %d\n",k,i);
	       printf("|");
	    
	    printf("%c",atpArr[i][k]);
    }
	
   	printf("\n");
    }
  // printf("iptVal:%s,len:%d,totalAtp:%d",iptVal,strlen(iptVal),totalAtp);
    printf("-----The end of array\n"); 
    */
}

// checking digits 
int allisdigit(char* str){
    //return 0 if not all digit
    for (int i = 0; i < strlen(str);i++) { 
	if (isdigit(str[i]) == 0) {
	    return 0;
	}
    }

    return 1;
}

//checking letters ignore case
int allisalpha(char* str){
    //return 0 if not all digit
    for (int i = 0; i < strlen(str) - 1;i++) {
	if (isalpha(str[i]) == 0) {
	    return 0;
	}
    }

    return 1;
}

//get input from user
char* get_userinput(int wordLen, char* word){
    //stdio
    char buffer[50];
    char* iptVal = (char*)malloc(sizeof(char) * (wordLen + 50));	
   /*	 if (feof(stdin)){
		fprintf(stderr,"Bad luck - the word is \"%s\".\n",word);	    
		exit(3);
	 }else{	
		fgets(buffer,(wordLen+3),stdin);
		iptVal = buffer;
		return iptVal;
	 }

}  */ 
    
    if (fgets(buffer,(wordLen + 50),stdin) == NULL){
	fprintf(stderr, "Bad luck - the word is \"%s\".\n",word);	    
	exit(3);
	   
    } else {
	iptVal = buffer;
    }
    return iptVal;
}

//print the inputed guess with hints.
void hints(char* iptVal, char* word, char** atpArr, int atpRem, int totalAtp){
    int thisGuess = totalAtp - atpRem;
    for (int i = 0; i < strlen(word);i++) {
  	_Bool notsamecase1 = strchr(word, toupper(iptVal[i]));
  	_Bool notsamecase2 = strchr(word, tolower(iptVal[i]));
	if (tolower(iptVal[i]) == tolower(word[i])) {
	    atpArr[thisGuess][i] = toupper(iptVal[i]);
	} else if (notsamecase1 || notsamecase2) {
	    atpArr[thisGuess][i] = tolower(iptVal[i]);
	} else {
	    atpArr[thisGuess][i] = '-';
	};
    }

	/*if one letter in correct location, the other should be '-'*/
    for (int k = 0; k < strlen(word);k++) {
	if (atpArr[thisGuess][k] == toupper(atpArr[thisGuess][k])) {
	    for (int j = 0; j < strlen(word);j++) {
		if ((atpArr[thisGuess][j]) == tolower(atpArr[thisGuess][k])) {
		    atpArr[thisGuess][j] = '-';
		}
	    }
      	};
    }

	/*if no letter in correct location, the other latter should be '-'*/
    for (int a = 0; a < strlen(word);a++) {	
	// if ( atpArr[thisGuess][k]=toupper( atpArr[thisGuess][k])){
	for (int b = 0; b < strlen(word);b++) {
		  
	    if ((atpArr[thisGuess][b]) == (atpArr[thisGuess][a])) {
		if (a < b) {
		    atpArr[thisGuess][b] = '-';
		} else if (a > b) {
		    atpArr[thisGuess][a] = '-';
		}
	    }
	}
    };

//printf("----history guessing\n");
    for (int t = 0; t < strlen(word);t++) {
    	printf("%c",atpArr[thisGuess][t]);
    }
    printf("\n");
//printf("-----The end of array\n"); 
    

}





