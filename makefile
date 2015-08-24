ep1sh: ep1sh.o ep1.o sjf.o
	gcc -o ep1sh ep1sh.o ep1.o sjf.o -lreadline -pthread -g

ep1sh.o: ep1sh.c
	gcc -c ep1sh.c -Wall -ansi

ep1.o: ep1.c
	gcc -c ep1.c -Wall -ansi

sjf.o: sjf.c
	gcc -c sjf.c -Wall -ansi

clean:
	rm -rf *.o
	rm -rf *~
	rm -rf a.out
	rm ep1sh
