#include <cstdio>
#include <cstring>
#include "gf.hpp"
#include "binaryreader.hpp"
#include "modexp.hpp"
#include "linearsolver.hpp"
#include "am.hpp"

void testbinaryreader() {
	BinaryReader reader("small.bin", 1000);

// 	while(true) {
// 		uint64_t* bigarray = reader.readarray(10000);
// 		for(int i = 0; i < 10; i++) {
// 			printf("%016lX\n", bigarray[i]);
// 		}
// 	}

 	printf("%016lX\n", reader.next());
	printf("%016lX\n", reader.next());
	printf("pos: %lu\n", reader.getpos());
}

void testgfmul()
{
	int k = 2;
	int l = 3;
	int p[3] = { 7, 2, 1 }; 
	GF field = GF(k, l, p);
	
	uint64_t a[] = {0x67dce43221b3c482, 0x5f16daf23e5041ed};
	uint64_t b[] = {0x2b05c90e1a794fc0, 0x48dee6c0fb29071a};
	
	uint64_t* res = field.unassigned();	
	field.print(a); 
	field.print(b);
	field.mul(a,b,res);
    field.print(res);
	
	ModExp modexp = ModExp(field, res, 8, 8);
	
	uint64_t* powres = field.unassigned();
	uint64_t z = 12345678987654321;
	modexp.power(z, powres);
	field.print(powres);
	
	uint64_t zz = 56567878;
	modexp.power(zz, powres);
	field.print(powres);
}

void simplelineartest() {
	int k = 1;
	int l = 3;
	int p[3] = { 4, 3, 1 }; 
	GF field = GF(k, l, p);

	int n = 4; 
	int r = 1;

	uint64_t** M = new uint64_t*[n];
	M[0] = field.unassigned();
	M[1] = field.unassigned();
	M[2] = field.unassigned();
	M[3] = field.unassigned();
	M[0][0] =  0x15UL;
	M[1][0] =  0x19UL;
	M[2][0] =  0xFUL;
	M[3][0] =  0xFUL;
	
	LinearSolver lin = LinearSolver(k);
	
	uint64_t** X = new uint64_t*[r];
	X[0] = field.unassigned();
	X[0][0] = 0xFFFFFFFFFFFFFFFF;

 	printf("Original matrix\n");
 	field.printbits(M, n);

	lin.randomsolution(M, n, X, r);
	
	// Print results
 	printf("\nPost Gaussian elmination and pivot deletion\n");
	field.printbits(M, n);
	printf("\nSolution matrix\n");
	field.printbits(X, r);
}

void testlinearsolver() {
	int k = 2;
	int l = 3;
	int p[3] = { 7, 2, 1 }; 
	GF field = GF(k, l, p);

	BinaryReader rand("test.bin", 1000);
	
	LinearSolver lin = LinearSolver(k);
	
	int n = 110;
	// Load a random (n x 128) matrix M 
	uint64_t** M1 = new uint64_t*[n];
	uint64_t** M2 = new uint64_t*[n];
	
	for(int i = 0; i < n; i++) {
		M1[i] = rand.readarray(k);
		M2[i] = field.copyelement(M1[i]);
	}
	
	int r = 10;
	uint64_t** X = new uint64_t*[r];
	for(int t = 0; t < r; t++) {
		X[t] = rand.readarray(k);
	}
	
	lin.randomsolution(M1, n, X, r);
	
	// Print results
 	printf("Original matrix\n");
 	field.printbits(M2, n);
 	printf("\nPost Gaussian elmination and pivot deletion\n");
	field.printbits(M1, n);
	printf("\nSolution matrix\n");
	field.printbits(X, r);
	printf("\nMX\n");
	uint64_t* res = field.zero();
	
	for(int i = 0; i < n; i++) {
		for(int t = 0; t < r; t++) {
			lin.setbit(res, t, lin.dotproduct(M2[i], X[t]));
		}
		field.printbits(res);
	}
}

void amtest() {
	amsettings settings;
	settings.datafile = "data.dat";
	AM am(settings);
}

int main(int argc, char** argv)
{
// 	testgfmul();
// 	testbinaryreader();	
// 	testlinearsolver();
// 	simplelineartest();
	amtest();
	return 0;	
}
