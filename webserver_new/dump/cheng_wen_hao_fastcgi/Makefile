main:main.o fcgi.o 
		cc main.o fcgi.o  -o main -std=c99
main.o:fcgi.h
		cc -c  main.c -std=c99
fcgi.o:fcgi.h
		cc -c fcgi.c -std=c99
.PHONY:clean
clean:
		rm *.o main
