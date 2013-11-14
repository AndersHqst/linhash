#ifndef GFHEADERDEF
#define GFHEADERDEF

#include <cstdint>

typedef struct {
    uint64_t hi64;
	uint64_t lo64;
} uint128;

class GF
{
public:
    GF(int k, int l, int* p);
    uint64_t* mul(uint64_t* a, uint64_t* b);
	void print(uint64_t* a);
private:
	static inline uint128 clmul(uint64_t a, uint64_t b);
	int k;
	int l;
	int* p;
};

#endif
