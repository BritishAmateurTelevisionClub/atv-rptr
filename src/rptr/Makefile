all:	rptr

clean:
	rm -f *.o *.inc *.so *.c~ *.h~
	indent -linux -c 60 -brf -l 132

INCLUDEFLAGS2=-lpthread -I/opt/vc/include -I/opt/vc/include/interface/vmcs_host/linux -I/opt/vc/include/interface/vcos/pthreads -I..
LIBFLAGS2=-L/opt/vc/lib -fcommon -lbcm_host -lpthread  -ljpeg -lrt -lm -ldl -lpigpiod_if2

rptr: main.c 
	gcc -Wall $(INCLUDEFLAGS2) -o  rptr \
		look-ups.c \
		listeners.c \
		timing.c \
		font/font.c \
		font/dejavu_sans_18.c \
		font/dejavu_sans_20.c \
		font/dejavu_sans_22.c \
		font/dejavu_sans_24.c \
		font/dejavu_sans_26.c \
		font/dejavu_sans_28.c \
		font/dejavu_sans_30.c \
		font/dejavu_sans_32.c \
		font/dejavu_sans_72.c \
		main.c $(LIBFLAGS2) 

install:
	cp rptr ../../bin/