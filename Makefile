CC=gcc
LDFLAGS=-Wall
SINGLE_TORRENT=0
CFLAGS=-Wall -D SINGLE_TORRENT=$(SINGLE_TORRENT)
TARGETS= transmission-verify-torrent
INSTLOC=/usr/local/bin

all: ${TARGETS}
clean:
	-rm -f *.o ${TARGETS}
install: 
	cp -f transmission-verify-torrent ${INSTLOC}

transmission-verify-torrent: transmission-verify-torrent.o
transmission-verify-torrent.o: transmission-verify-torrent.c

