ssu_sdup : ssu_sdup.o ssu_find-md5.o ssu_find-sha1.o ssu_help.o
	gcc ssu_sdup.o ssu_find-md5.o ssu_find-sha1.o ssu_help.o -o ssu_sdup

ssu_sdup.o: ssu_sdup.c
	gcc -c ssu_sdup.c 

ssu_find-fmd5.o: ssu_find-md5.c
	gcc -c ssu_find-md5.c -lcrypto

ssu_find-fsha1.o: ssu_find-sha1.c
	gcc -c ssu_find-sha1.c -lcrypto

ssu_help.c: ssu_help.c
	gcc -c ssu_help.c

clean :
	rm ssu_sdup.o
	rm ssu_find-md5.o
	rm ssu_find-sha1.o
	rm ssu_help.o
	rm ssu_sdup
