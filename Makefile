module:
	gcc -fPIC -fno-stack-protector -lcurl -lpam -c *.c
	ld -x --shared -o pam_websso.so *.o
	chmod 644 pam_websso.so

install: module
	sudo cp ${MODULE}.so /lib/x86_64-linux-gnu/security/

clean:
	-rm -f *.o
	-rm -f *.so
