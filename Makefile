# Andrew Cox
# 26 May 2024

CC = gcc

CFLAGS = -Wall -g

FILES = PageSim.c

DEPEND = $(DERIV) MemSim.h PageSim.h

DERIV = ${FILES:.c=.o}

all: PageSim

PageSim: $(DEPEND) MemSim.o
	$(CC) -o PageSim $(CFLAGS) $(DERIV) MemSim.o

clean:
	rm -f $(DERIV) PageSim
