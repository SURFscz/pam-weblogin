module:
	gcc -fPIC -fno-stack-protector -c *.c
	ld -x --shared -lcurl -lpam -lm -lc -o pam_websso.so *.o
	chmod 644 pam_websso.so

install: module
	sudo cp pam_websso.so /lib/x86_64-linux-gnu/security/

clean:
	-rm -f *.o
	-rm -f *.so
