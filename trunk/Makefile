
CC=g++
CFLAGS=-I.
DEBUG=-g
DEPS=geom.h radio.h source.h utils.h
OBJ=geom.o radio.o source.o utils.o main.o
LIBS=-lm -lstdc++

#%.o: %.C $(DEPS)
#	$(CC) -c -o $@ $< $(CFLAGS)

.C.o:
	$(CC) -c $(DEBUG) $<

main: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm -f *.o *~ core



