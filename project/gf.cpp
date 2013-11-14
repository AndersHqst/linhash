#include <xmmintrin.h> 
#include <smmintrin.h>
#include <cstdint>
#include <cstdio>
#include "gf.hpp"

GF::GF(int k, int l, int* p)
{
	this->k = k;
	this->l = l;
	this->p = p;
}

uint64_t* GF::mul(uint64_t* a, uint64_t* b)
{
	printf("k: %u\n", k);
	printf("l: %u\n", l);
	
	printf("contents of p[] \n");
	for(int i = 0; i < l; i++) {
		printf("p[%u] = %u\n", i, p[i]);
	}
	
	uint64_t* res = new uint64_t[k];

	// Perform carry-less multiplication
	uint64_t* clres = new uint64_t[2*k];
	for(int i = 0; i < 2*k; i++) { clres[i] = 0; }
	
	uint128 tmp;
	for(int i = 0; i < k; i++) {
		for(int j = 0; j < k; j++) {
			tmp = GF::clmul(a[i], b[j]);
			clres[i+j] ^= tmp.lo64;
			clres[i+j+1] ^= tmp.hi64;
		}	
	}	
	
	// Print result of clmul
	for(int i = 2*k-1; i >= 0; i--) {
		printf("%016llX", clres[i]);
	}
	printf("\n");


	// Modular reduction

	// m(x) = M(c(x)g(x)) stored in upper half of clres
	for(int i = 0; i < l; i++) {
		clres[k] ^= clres[2*k-1] >> (64 - p[i]);  	
	}
	
	// p(x) = L(g*(x)m(x)) stored in res, m(x) read from clres
	res[0] = clres[k];
	for(int i = 0; i < l; i++) { 
		res[0] ^= clres[k] << p[i];
    }

	for(int j = 1; j < k; j++) {
		res[j] = clres[k+j];
		for(int i = 0; i < l; i++) { 
			res[j] ^= (clres[k+j] << p[i]) ^ (clres[k+j-1] >> (64 - p[i]));
		}
	}
	
	// XOR p(x) with lower half of clres
	for(int j = 0; j < k; j++) {
		res[j] ^= clres[j];
	}

	delete[] clres;
	
	return res;
}

inline uint128 GF::clmul(uint64_t a, uint64_t b)
{
	uint128 res;
	__m128 tmpa;
	__m128 tmpb;
	
	// Load data into xmm registers
    __asm__(
		"PINSRQ $0x00, %2, %0;" // Move a into lower half of xmm register tmpa
		"PINSRQ $0x00, %3, %1;" // Move b into lower half of xmm register tmpx
		: // Output
		"=x"(tmpa), "=x"(tmpb)   
		: // Input 
		"r"(a), "r"(b)
	);
	
	// Perform carry-less multiplication and extract result from xmm into GPRs
	__asm__(
		"PCLMULQDQ $0x00, %3, %2;" // CLMUL and store result in tmpa
		"PEXTRQ $0x00, %2, %0;" // Extract low 64 bits of result into res.lo64
		"PEXTRQ $0x01, %2, %1;" // Extract high 64 bits of result into res.hi64
		: // Output
		"=r"(res.lo64), "=r"(res.hi64)   
		: // Input 
		"x"(tmpa), "x"(tmpb) 
	);

	return res;
}

void GF::print(uint64_t* a)
{
	for(int i = k-1; i >= 0; i--) {
		printf("%016llX", a[i]);
	}
}
