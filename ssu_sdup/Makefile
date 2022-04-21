ssu_sdup : ssu_sdup.o ssu_help.o
	gcc ssu_sdup.o ssu_help.o -o ssu_sdup -lcrypto

ssu_sdup.o : ssu_sdup.c
	gcc -c ssu_sdup.c

ssu_help.o : ssu_help.c
	gcc -c ssu_help.c

clean :
	rm ssu_sdup.o
	rm ssu_help.o
	rm ssu_sdup
