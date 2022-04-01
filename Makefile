MODULE := pam_websso

module:
	gcc -fPIC -fno-stack-protector -c ${MODULE}.c
	ld -x --shared -o ${MODULE}.so ${MODULE}.o

install:
	cp ${MODULE}.so /lib/x86_64-linux-gnu/security/

clean:
	-rm -f ${MODULE}.so

