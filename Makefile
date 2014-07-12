
all:
	gcc -o ttyusb ttyusb.c hexdump.c

clean:
	rm ttyusb