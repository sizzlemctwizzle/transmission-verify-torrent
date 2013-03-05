CC=gcc
LDFLAGS=-Wall
CFLAGS=-Wall
TARGETS= transmission-verify-torrent

all: ${TARGETS}
clean:
	-rm -f *.o ${TARGETS}
install: 
	cp transmission-verify-torrent /usr/local/bin

transmission-verify-torrent: transmission-verify-torrent.o
transmission-verify-torrent.o: transmission-verify-torrent.c

