IDIR =.
CC=gcc
CFLAGS=-I$(IDIR) -D_REENTRANT -lpthread -lcurl -ljansson -llog4c -g -Wall -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include -lglib-2.0

ODIR=obj
LDIR =./lib

LIBS=-lm

_DEPS = zmalloc.h ae.h anet.h fmacros.h sds.h configure.h md5.h myhash.h util.h hash_table.h read_conf.h parser.h io.h read_handler.h event.h store.h lrued_query.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = ae.o anet.o zmalloc.o sds.o cir_queue.o md5.o myhash.o hash_table.o  read_conf.o parser.o io.o read_handler.o event.o store.o main.o lrued_query.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

main: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~  main 
