#include <stdio.h>
#include <cstdint>
#include <xmmintrin.h> // SSE intrinsics
#include <smmintrin.h>
#include <ipp.h> // Intel integrated performance primitives
#include <stdlib.h> // rand()
#include <bitset>
#include <iostream>
#include <string>
#include <fstream>

//typedef unsigned __int64 uint64;
typedef uint64_t uint64;

struct solution {
	uint64* C;
	int rank;
};

inline void swap(uint64* M, int i, int j) {
	uint64 tmp;
	tmp = M[i];
	M[i] = M[j];
	M[j] = tmp;
}

void print_matrix(uint64* M, int n) {
	for(int i = 0; i < n; i++){
		std::bitset<64> row (M[i]);
		std::cout << row << std::endl;
	}
}

// Determine rank and calculate pseudoinverse of matrix
solution solve(uint64* M, int n, int m)
{
	int rank = 0;
	int pivot;
	bool pivotFound = false;
	uint64 mask;
	uint64* C;

	// Create (n x n) identity
	uint64* I = new uint64[n];
	for(int i = 0; i < n; i++) {
		I[i] = 1ULL << (n - i - 1);
	}
		
	// For each column
	for(int j = m - 1; j != 0; j--) {
		mask = 1ULL << j;
	
		// For each row (start from the bottom)
		for(int i = n - 1; i >= 0; i--) {
			// Check if the bit (i,j) is set
			if((M[i] & mask) != 0) {
			
				// printf("Bit set at (%u, %u)\n", i + 1, m - j);
				
				// Try setting a new pivot if we are not in the upper rank rows (leading 1 rows)
				if((rank <= i) && !pivotFound) {
					pivotFound = true;
					pivot = i;
				} else if(pivotFound){
					M[i] = M[i] ^ M[pivot];
					I[i] = I[i] ^ I[pivot];
				}
			}
		}
		
		// Rearrange rows to echelon form
		if(pivotFound) {
			//printf("\nPivot: %u Rank: %u\n", pivot, rank);
			swap(M, pivot, rank);
			swap(I, pivot, rank);
			rank++;
			pivotFound = false;
		}
		
		// printf("\nM after column %u\n", m-j);
		// print_matrix(M, n);
	}
	
	// printf("\n\nM after Gaussian elimination\n");
	// print_matrix(M, n);
	
	// printf("\n\nI after Gaussian elimination\n");
	// print_matrix(I, n);
	
	// Construct pseudoinverse (m x n) if full rank
	if(rank == n) {
		// Initialize with zero rows
		C = new uint64[m];
		for(int r = 0; r < m; r++) { C[r] = 0; }

		// Insert rows from I
		for(int i = 0; i < n; i++) {
			// Note: lzcnt Seems to be floor(lg(M[i]))
			C[m - 1 - _lzcnt_u64(M[i])] = I[i];
		}
	}
	
	solution s;
	s.C = C;
	s.rank = rank;
	
	return s;
}

uint64 dot_product(uint64 a, uint64 b){
	return (_mm_popcnt_u64(a & b) & 1); 
}

// Matrix multiplication of (n x m) * (m x p) = (n x p)
uint64* matrix_mul(uint64* A, uint64* B, int n, int m, int p) {
	uint64* C = new uint64[n];
	for(int r = 0; r < n; r++) { C[r] = 0; }
	uint64 c;
	// For every column in B
	for(int j = p - 1; j >= 0; j--) {
		c = 0;
		// Construct column j of B
		for(int i = 0; i < m; i++) {
			c ^= ((B[i] & (1ULL << j)) >> j) << (m - 1 - i); // Extract bit in position j of B[i] and shift to position 0. Then shift it to the correct position in c.		
		}
		
		// printf("\nColumn %u\n", p - j);
		// print_matrix(&c, 1);
		
		// Compute column j of result
		for(int r = 0; r < n; r++) {
			C[r] ^= dot_product(A[r], c) << j; 
		}
	}
	return C;
}

uint64* copy_matrix(uint64* M, int n) {
	uint64* C = new uint64[n];
	for(int i = 0; i < n; i++) {
		C[i] = M[i];
	}
	return C;
}


uint64* read_binary(std::string filename, int n) {
	uint64* B = new uint64[n];
	char bytes[8*n];
	std::ifstream binfile;
	binfile.open(filename, std::ios::binary | std::ios::in);
	
	if(binfile.read(bytes, 8*n)) {
		for(int i = 0; i < n; i++) {
			B[i] = 0;
			// Manual load
			for(int j = 0; j < 8; j++) {
				B[i] ^= ((uint64)bytes[8*i + j]) << 8*j;
			} 
			//*((uint64*)bytes[8*i]); // Cast byte pointer to 8-byte pointer and dereference 
		}
	}
	
	binfile.close();
	
	return B;
}

void simple_test(){
		// Key matrix M (3 x 5)
	uint64* M = new uint64[3];
	M[0] = 0x0E;
	M[1] = 0x17;
	M[2] = 0x1A;
	
	// Value matrix V (3 x 4)
	uint64* V = new uint64[3];
	V[0] = 0xF; 
	V[1] = 0xB;
	V[2] = 0x3;
	
	uint64* O = copy_matrix(M, 3);
	
	solution s = solve(M, 3, 5);
	
	//printf("\nRank: %u\n", s.rank);
	
	printf("\nO\n");
	print_matrix(O, 3);
	
	printf("\nM\n");
	print_matrix(M, 3);
	
	printf("\nC\n");
	print_matrix(s.C, 5);
	
	uint64* MC = matrix_mul(O, s.C, 3, 5, 3);

	printf("\nM*C\n");
	print_matrix(MC, 3);
	
	// Create X for value matrix V
	uint64* X =  matrix_mul(s.C, V, 5, 3, 4);
	
	printf("\nX = C*V\n");
	print_matrix(X, 5);
	
	printf("\nV\n");
	print_matrix(V, 3);
	
	printf("\nO*X\n");
	print_matrix(matrix_mul(O, X, 3, 5, 4), 3);
	
	// Random matrix test
	uint64* binary = read_binary("2011-04-23.bin", 80);
	
	uint64* binM = binary;
	uint64* binMCopy = copy_matrix(binM, 64);
	uint64* binV = binary + 40;
	
	solution ss = solve(binary, 40, 64);
	
	uint64* binS = ss.C;
	uint64* binX = matrix_mul(binS, binV, 64, 40, 64);
	
	printf("\nbinV\n");
	print_matrix(binV, 40);
	
	uint64* MXbin = matrix_mul(binMCopy, binX, 40, 64, 64);
	printf("\nbinM*binX\n");
	print_matrix(MXbin, 40);
	
	// Equaility test
	bool equal = true;
	for(int i = 0; i < 40; i++) {
		if(MXbin[i] != binV[i]){
			equal = false;
		}
	}
	
	printf("\n\nCorrectness: %u\n\n", equal ? 1 : 0);
}

double avg_time(Ipp64u start, Ipp64u stop, int n)
{
	Ipp64u elapsed = stop - start;
	double avg = (double)elapsed / (double)n;
	return avg;
}

int main(int argc, char** argv) 
{
	// Performance test: solve 64x64 systems
	int rep = 1000;
	uint64* random = read_binary("rand.bin", 64*rep);
	solution ps;
	Ipp64u start, stop;
	int* ranks = new int[rep];
	
	start = ippGetCpuClocks();
	for(int i = 0; i < rep; i++) {
		ps = solve((random + 64*i), 64, 64);
		ranks[i] = ps.rank;
	}
	stop = ippGetCpuClocks();
	
	// for(int i = 0; i < rep; i++){
		// printf("%u\n", ranks[i]);
	// }
	
	printf("\nCycles to solve 64x64 on average: %.0f", avg_time(start, stop, rep));
	
    return 0;
}