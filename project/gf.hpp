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
	~GF();
	int fieldwordsize() const;
	void kindhash(uint64_t** a, uint64_t* z, int k, uint64_t* res) const;
    void mul(uint64_t* a, uint64_t* b, uint64_t* res) const;
	void add(uint64_t* a, uint64_t* b, uint64_t* res) const;
	uint64_t* unassigned() const;
	uint64_t* identity() const;
	uint64_t* zero() const;
	uint64_t* copyelement(uint64_t* a) const;
	void copyinto(uint64_t* a, uint64_t* res) const;
	void print(uint64_t* a) const;
	void print(uint64_t** M, int n) const;
	void printbits(uint64_t* a) const;
	void printbits(uint64_t** M, int n) const;
private:
	static inline uint128 clmul(uint64_t a, uint64_t b);
	const int k;
	const int l;
	const int* p;
	mutable uint64_t* clres;
};

#endif
