#include <stdio.h>
#include <cstdint>
#include <xmmintrin.h>
#include <smmintrin.h>
#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>

typedef uint64_t u64;

void print_hex(u64 a)
{
    printf("%16llX\n", a);
}

inline u64 mul_gf(u64 a, u64 x)
{
	u64 l, h, m, res;
	__m128 tmpa;
	__m128 tmpx;
	
	// Load data into xmm registers
    __asm__(
		"PINSRQ $0x00, %2, %0;" // Move a into lower half of xmm register tmpa
		"PINSRQ $0x00, %3, %1;" // Move x into lower half of xmm register tmpx
		: // Output
		"=x"(tmpa), "=x"(tmpx)   
		: // Input 
		"r"(a), "r"(x)
	);
	
	// Perform carry-less multiplication and extract result from xmm into GPRs
	__asm__(
		"PCLMULQDQ $0x00, %3, %2;" // CLMUL and store result in tmpa
		"PEXTRQ $0x00, %2, %0;" // Extract low 64 bits of result into ax.lo64
		"PEXTRQ $0x01, %2, %1;" // Extract high 64 bits of result into ax.hi64
		: // Output
		"=r"(l), "=r"(h)   
		: // Input 
		"x"(tmpa), "x"(tmpx) 
	);
	
	// print clmul result
	printf("%016llX%016llX\n", h, l);
	
	// Calculate ax mod (X^64 + X^4 + X^3 + X + 1) using the technique specified by Intels CLMUL white paper
	m = h ^ (h >> 60) ^ (h >> 61);
	res = m ^ (m << 1) ^ (m << 3) ^ (m << 4) ^ l; 
	
	return res;
}



void test_gf_mul()
{
	// Test 1
	u64 a1 = 0x81E37D20AEFFEA39;
	u64 b1 = 0x8BF26934A1D851B5;
	u64 r1 = 0x45E81213C1BAB6A8; // Verified using Maxima
	
	printf("a1, b1, result, independent result\n");
	print_hex(a1);
	print_hex(b1);
	print_hex(mul_gf(a1,b1));
	print_hex(r1);
	printf("\n");
	
	// Test 2
	u64 a2 = 0x84B4B27847A97E01;
	u64 b2 = 0xE541829B9E123096;
	u64 r2 = 0x41BD73F3385BF23D; // Verified using Maxima
	
	printf("a2, b2, result, independent result\n");
	print_hex(a2);
	print_hex(b2);
	print_hex(mul_gf(a2,b2));
	print_hex(r2);
	printf("\n");
}

int main(int argc, char** argv) 
{
	test_gf_mul();
    return 0;
}
