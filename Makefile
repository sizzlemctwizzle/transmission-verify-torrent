CC=gcc
LDFLAGS=-Wall
SINGLE_TORRENT=0
CFLAGS=-Wall -D SINGLE_TORRENT=$(SINGLE_TORRENT)
TARGETS= transmission-verify-torrent

all: ${TARGETS}
clean:
	-rm -f *.o ${TARGETS}
install: 
	cp transmission-verify-torrent /usr/local/bin

transmission-verify-torrent: transmission-verify-torrent.o
transmission-verify-torrent.o: transmission-verify-torrent.c

