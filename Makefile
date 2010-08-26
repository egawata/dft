OPTION=-Wall -g
CC=gcc
GTKOPT=`pkg-config --cflags --libs gtk+-2.0`

all: wavfile.o readwav.c
	$(CC) $(OPTION) -o readwav readwav.c wavfile.o

dft: wavfile.o dft.c
	$(CC) $(OPTION) -o dft dft.c wavfile.o -lm

wavfile.o:	wavfile.h wavfile.c
	$(CC) $(OPTION) -c wavfile.c

fgraph: freqgraph.c
	$(CC) $(OPTION) -lm $(GTKOPT) -o freqgraph freqgraph.c  
	 
