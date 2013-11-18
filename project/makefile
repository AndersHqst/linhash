CC = g++
CFLAGS = -msse4.1 -mpopcnt -std=c++0x -g

main : main.o gf.o binaryreader.o modexp.o linearsolver.o am.o
	$(CC) $(CFLAGS) -o main main.o gf.o binaryreader.o modexp.o linearsolver.o am.o

main.o : main.cpp
	$(CC) $(CFLAGS) -o main.o -c main.cpp

am.o : am.hpp am.cpp
	$(CC) $(CFLAGS) -o am.o -c am.cpp
	
modexp.o : modexp.hpp modexp.cpp gf.o
	$(CC) $(CFLAGS) -o modexp.o -c modexp.cpp

gf.o : gf.hpp gf.cpp
	$(CC) $(CFLAGS) -o gf.o -c gf.cpp

linearsolver.o : linearsolver.hpp  linearsolver.cpp
	$(CC) $(CFLAGS) -o linearsolver.o -c linearsolver.cpp
	
binaryreader.o : binaryreader.hpp binaryreader.cpp
	$(CC) $(CFLAGS) -o binaryreader.o -c binaryreader.cpp

clean:
	rm -rf *.o
	rm -rf *.s

