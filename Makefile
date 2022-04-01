MODULE := pam_websso

module:
	gcc -fPIC -fno-stack-protector -lcurl -lpam -lc -c ${MODULE}.c curl.c
	ld -x --shared -o ${MODULE}.so ${MODULE}.o curl.o
	chmod 644 ${MODULE}.so

install: module
	sudo cp ${MODULE}.so /lib/x86_64-linux-gnu/security/

clean:
	-rm -f ${MODULE}.o
	-rm -f ${MODULE}.so
