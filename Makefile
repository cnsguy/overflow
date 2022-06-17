all: partial-overwrite

partial-overwrite: partial-overwrite.c
	$(CC) -o $@ $^ -O0 -fPIE -pie -Wl,-z,relro,-z,now -fstack-protector-all --param=ssp-buffer-size=8

clean:
	-rm partial-overwrite
