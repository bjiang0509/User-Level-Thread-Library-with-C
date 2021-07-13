# Target library
lib := libuthread.a
objs := queue.o context.o uthread.o sem.o preempt.o 
CC := gcc
CFLAGS := -Wall -Werror
CFLAGS += -g

all: $(lib)

queue.o: queue.c queue.h
	$(CC) $(CFLAGS) -c queue.c

context.o: context.c private.h
	$(CC) $(CFLAGS) -c context.c 

uthread.o: uthread.c uthread.h private.h queue.h
	$(CC) $(CFLAGS) -c uthread.c  

sem.o: sem.c sem.h private.h queue.h
	$(CC) $(CFLAGS) -c sem.c

preempt.o: preempt.c private.h
	$(CC) $(CFLAGS) -c preempt.c

## static library
$(lib): $(objs)
	ar rcs $(lib) $(objs)

clean:
	rm -f  *.o  *.d $(lib)
