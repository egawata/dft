OPTION=-Wall -g -lm
CC=gcc
GTKOPT=`pkg-config --cflags --libs gtk+-2.0`
GLIBOPT=`pkg-config --cflags --libs glib-2.0`

OBJS=freqgraph.o ampdata.o freqdata.o graphview.o

freqgraph: $(OBJS)
	$(CC) $(OPTION) $(GTKOPT) $(GLIBOPT) -o $@ $(OBJS)

clean: 
	rm $(OBJS)
	rm freqgraph

.c.o:
	$(CC) -c $(OPTION) $(GLIBOPT) $(GTKOPT) -o $@ $<
.SUFFIXES: .c .o

