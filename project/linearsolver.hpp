#ifndef LINEARSOLVERHEADERDEF
#define LINEARSOLVERHEADERDEF

#include <cstdint>

class LinearSolver
{
public:
	LinearSolver(int k);
	void randomsolution(uint64_t** M, int n, uint64_t** res, int r);
	bool dotproduct(uint64_t* a, uint64_t* b);
	void xorto(uint64_t* a, uint64_t* res);
	void setbit(uint64_t* a, int i, bool val);
	void setbit(uint64_t* a, int w, int j, bool val);
	bool allzero(uint64_t* a, uint64_t** X, int r);
private:
	void swaprows(uint64_t** M, int i, int j);
	int k;
};

#endif
