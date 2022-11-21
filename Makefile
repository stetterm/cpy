CC = gcc
CFLAGS = -g -std=c11 -pthread

DEPS = consumer.h producer.h
OBJ = cpy.o consumer.o producer.o

%.o: %.c $(DEPS) 
	$(CC) -c -o $@ $< $(CFLAGS)

cpy: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -rf *.o
	rm -f cpy
