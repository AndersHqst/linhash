# Compile notes

## Set icpc env. variables
Open a terminal window in this folder and type: "source icpc.sh"
to set the intel c++ compiler environment variables.

Use the command "make" to compile.

Execute the program using "./main"

## Compile to assembly, with gcc
gcc -S -c hello.cpp

Creates assembly code in a hello.s file

## Compile to C/assembly, with gcc
gcc -c -g -Wa,-a,-ad hello.cpp > foo.lst

Creates assembly code listed with the c code in foo.lst


