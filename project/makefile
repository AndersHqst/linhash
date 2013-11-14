CFLAGS = -std=c++0x 

main : gf.o main.o
	icpc $(CFLAGS) gf.o main.o -o main

main.o : main.cpp
	icpc $(CFLAGS) -c -o main.o main.cpp

gf.o : gf.cpp gf.hpp
	icpc $(CFLAGS) -c -o gf.o gf.cpp

clean:
	rm -rf *.o
