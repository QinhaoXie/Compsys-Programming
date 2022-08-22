all: dbclient dbserver stringstore.o libstringstore.so
dbserver: dbserver.c libstringstore.so 
	gcc -og -pthread -pedantic -Wall -std=gnu99 -I/local/courses/csse2310/include -L/local/courses/csse2310/lib -lcsse2310a4 -lcsse2310a3 -lstringstore -o dbserver dbserver.c responses.c
dbclient:dbclient.c
	gcc -og  -pedantic -Wall -std=gnu99 -I/local/courses/csse2310/include -L/local/courses/csse2310/lib -lcsse2310a4 -lcsse2310a3 -o dbclient dbclient.c
# Turn stringstore.c into stringstore.o
stringstore.o: stringstore.c
	gcc -fPIC -Wall -pedantic -std=gnu99 -o stringstore.o -c stringstore.c
#turn stringstore.o into shared library libstringstore.so
libstringstore.so: stringstore.o
	gcc -shared -o $@ stringstore.o
