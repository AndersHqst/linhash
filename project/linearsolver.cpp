#include <cstdint>
#include <cstdio>
#include "linearsolver.hpp"
#include "gf.hpp"

struct position {
	int w;
	int j;
	position(int w, int j) : w(w), j(j) {}
	position() {}
};

LinearSolver::LinearSolver(int k) {
	this->k = k;
}

// Perform Gaussian elimination and return a set of random solution vectors to the system Mx = 0
// M is the matrix M[row][word]
// n is the number of rows of M
// res an address to the result !IMPORTANT needs to be loaded with random bits
// r is the number of solution columns
void LinearSolver::randomsolution(uint64_t** M, int n, uint64_t** res, int r) {
	int pivot;
	int rank = 0;
	bool pivotFound = false;
	uint64_t mask;
	position* pivotpos = new position[n];

	// For each column word
	for(int w = k - 1; w >= 0; w--) {
		// For each column bit within the word w
		for(int j = 63; j >= 0; j--) {
			mask = 1UL << j;

			// For each row (start from the bottom)
			for(int i = n - 1; i >= 0; i--) {
				// Check if the bit (64*w + i,j) is set
				if((M[i][w] & mask) != 0) {

					// Try setting a new pivot if we are not in the upper rank rows (leading 1 rows)
					if((rank <= i) && !pivotFound) {
						pivotFound = true;
						pivot = i;
						pivotpos[rank] = position(w, j);
					} else if(pivotFound){
						xorto(M[pivot], M[i]);
					}
				}
			}

			// Rearrange rows to echelon form
			if(pivotFound) {
				swaprows(M, pivot, rank);
				rank++;
				pivotFound = false;
			}
		}
	}

	// Construct solution vectors
	for(int i = 0; i < rank; i++) {
		// Set the pivot bit to 0 in the reduced row echelon form
		setbit(M[i], pivotpos[i].w, pivotpos[i].j, false);
		
		// Set the pivot values in each solution vector to reflect any potential free variables
		for(int t = 0; t < r; t++) {
			setbit(res[t], pivotpos[i].w, pivotpos[i].j, dotproduct(M[i], res[t]));
		}
	}
	delete[] pivotpos;
}

void LinearSolver::setbit(uint64_t* a, int i, bool val) {
	int w = i >> 6;
	int j = i & 0x3F;
	uint64_t mask = 1UL << j;
	if(val) {
		a[w] |= mask;
	} else {
		a[w] &= ~mask; // AND with 111111011111
	}
}

void LinearSolver::setbit(uint64_t* a, int w, int j, bool val) {
	uint64_t mask = 1UL << j;
	if(val) {
		a[w] |= mask;
	} else {
		a[w] &= ~mask; // AND with 111111011111
	}
}

void LinearSolver::swaprows(uint64_t** M, int i, int j) {
	uint64_t swap;
	for(int t = 0; t < k; t++) {
		swap = M[i][t];
		M[i][t] = M[j][t];
		M[j][t] = swap;
	}
}

bool LinearSolver::dotproduct(uint64_t* a, uint64_t* b) {
	uint64_t popcnt = 0;
	for(int t = 0; t < k; t++) {
		popcnt ^= __builtin_popcountll(a[t] & b[t]);
	}
	return (popcnt & 1UL) != 0;
}

void LinearSolver::xorto(uint64_t* a, uint64_t* res) {
	for(int t = 0; t < k; t++) {
		res[t] ^= a[t];
	}
}

bool LinearSolver::allzero(uint64_t* a, uint64_t** X, int r) {
	for(int i = 0; i < r; i++) {
		if(dotproduct(a, X[i])) {
			return false;
		}
	}
	return true;
}
