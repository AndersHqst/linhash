#include <xmmintrin.h> 
#include <smmintrin.h>
#include <cstdint>
#include <cstdio>
#include <bitset>
#include <iostream>
#include "gf.hpp"

GF::GF(int kk, int ll, int* pp) : k(kk), l(ll), p(pp) {
	// For temporary results of clmul ops
	clres = new uint64_t[2*k];
}

GF::~GF() {
	delete[] clres;
}

int GF::fieldwordsize() const {
	return k;
}

uint64_t* GF::unassigned() const {
	return new uint64_t[k];
}

uint64_t* GF::zero() const {
	uint64_t* res = new uint64_t[k];
	for(int i = 0; i < k; i++) {
		res[i] = 0;
	}
	return res;
}

uint64_t* GF::identity() const {
	uint64_t* res = zero();
	res[0] = 1UL;
	return res;
}

uint64_t* GF::copyelement(uint64_t* a) const {
	uint64_t* res = new uint64_t[k];
	for(int i = 0; i < k; i++) {
		res[i] = a[i];
	}

	return res;
}

void GF::copyinto(uint64_t* a, uint64_t* res) const {
	for(int i = 0; i < k; i++) {
		res[i] = a[i];
	}
}

void GF::kindhash(uint64_t** a, uint64_t* z, int k, uint64_t* res) const {
	copyinto(a[0], res);
	for(int i = 1; i < k; i++) {
		mul(res, z,  res);
		add(res, a[i], res);
	}
}

void GF::mul(uint64_t* a, uint64_t* b, uint64_t* res) const {

	// Perform carry-less multiplication
	for(int i = 0; i < 2*k; i++) { clres[i] = 0; }
	
	uint128 tmp;
	for(int i = 0; i < k; i++) {
		for(int j = 0; j < k; j++) {
			tmp = GF::clmul(a[i], b[j]);
			clres[i+j] ^= tmp.lo64;
			clres[i+j+1] ^= tmp.hi64;
		}	
	}	
	
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
}

inline void GF::add(uint64_t* a, uint64_t* b, uint64_t* res) const  {
	for(int i = 0; i < k; i++) {
		res[i] = a[i] ^ b[i];
	}
}

inline uint128 GF::clmul(uint64_t a, uint64_t b) {
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

void GF::print(uint64_t* a) const {
	for(int i = k-1; i >= 0; i--) {
		printf("%016lX", a[i]);
	}
	printf("\n");
}

void GF::print(uint64_t** M, int n) const {
	for(int i = 0; i < n; i++) {
		print(M[i]);
	}
}

void GF::printbits(uint64_t** M, int n) const {
	for(int i = 0; i < n; i++) {
		printbits(M[i]);
	}
}

void GF::printbits(uint64_t* a) const {
	for(int i = k-1; i >= 0; i--) {
		std::bitset<64> row(a[i]);
		std::cout << row;
	}
	std::cout << std::endl;
}
