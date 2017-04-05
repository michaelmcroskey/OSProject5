CXX=		/usr/bin/gcc
CXXFLAGS=	-Wall -g -c
SHELL=		bash
PROGRAMS=	virtmem

all: virtmem

virtmem: main.o page_table.o disk.o program.o
	$(CXX) main.o page_table.o disk.o program.o -o virtmem

main.o: main.c
	$(CXX) $(CXXFLAGS) main.c -o main.o

page_table.o: page_table.c
	$(CXX) $(CXXFLAGS) page_table.c -o page_table.o

disk.o: disk.c
	$(CXX) $(CXXFLAGS) disk.c -o disk.o

program.o: program.c
	$(CXX) $(CXXFLAGS) program.c -o program.o

clean:
	rm -f *.o $(PROGRAMS)
	rm -rf *.dSYM
