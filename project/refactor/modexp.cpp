#include <stdio.h>
#include <cstdint>
#include <xmmintrin.h> // SSE intrinsics
#include <smmintrin.h>
#include <ipp.h> // Intel integrated performance primitives
#include <stdlib.h> // rand()

typedef uint64_t u64;

// Prototypes
u64 mul_gf(u64 a, u64 x);
u64 repeat_square_gf(int k, u64 x);
u64 mod_exp(u64** tables, int charsize, u64 n);
void print_hex(u64 a);

// Multiplies two elements in GF(2^64) and returns the result
u64 mul_gf(u64 a, u64 x)
{
	u64 l, h, res;
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
	
	// Calculate ax mod (X^64 + X^4 + X^3 + X + 1) using the technique specified by Intels CLMUL white paper
	res = h ^ (h >> 60) ^ (h >> 61);
	res = res ^ (res << 1) ^ (res << 3) ^ (res << 4) ^ l; 
	
	return res;
}

u64 repeat_square_gf(int k, u64 x)
{
	u64 res = x;
	for(int i = 0; i < k; i++){
		res = mul_gf(res,res);
	}
	return res;
}

// Calculate g(x)^n mod (X^64 + X^4 + X^3 + X + 1).  
u64 mod_exp(u64** tables, int charsize, u64 n) 
{
	// Mask to extract lower bits
	u64 mask = ~(~0 << charsize); 
	int numtables = 64/charsize;
	u64 res = tables[0][n & mask];
	
	for(int i = 1; i < numtables; i++)
	{
		res = mul_gf(res, tables[i][(n >> i*charsize) & mask]);
	}
	
	return res;	
}

// Helper functions
void print_hex(u64 a)
{
	printf("%016I64X\n", a);
}

double avg_time(Ipp64u start, Ipp64u stop, int n)
{
	Ipp64u elapsed = stop - start;
	double avg = (double)elapsed / (double)n;
	return avg;
}

int main(int argc, char** argv) 
{
	// Base polynomial for exponentiation
	u64 base = 0x967A2A6581F71CAD; // right to left: x^63 + x^60 + x^58 + x^56 + .... 

	// Number of bits in each division of the u64 representation
	int charsize = 8;
	// Number of tables
	int numtables = 64/charsize;
	// Number of entries in each table
	int entries = 1 << charsize;
	
	// Precompute tables to speed up modular exponentiation 
	u64** tables = new u64*[numtables];
	u64 nextpower = base;
	for(int i = 0; i < numtables; i++)
	{
		tables[i] = new u64[entries];
		tables[i][0] = 1; // Identity element
		for(int j = 1; j < entries; j++)
		{
			tables[i][j] = mul_gf(nextpower, tables[i][j-1]); 
		}
		nextpower = repeat_square_gf(charsize, nextpower);
	}
	
	//print_hex(mod_exp(tables, charsize, 534098345943));
	
	// Performance test
	Ipp64u start, stop;
	int n = 1000000;
	u64* results = new u64[n];
	u64* powers = new u64[n];
	for(int i = 0; i < n; i++){
		powers[i] = (((u64)rand()) << 32) ^ ((u64)rand()); 
	}
	
	start = ippGetCpuClocks();
	for(volatile u64 j = 0; j < n; j++){
		results[j] = mod_exp(tables, charsize, powers[j]);
	}
	stop = ippGetCpuClocks();
	
	double avgclocks = avg_time(start, stop, n);
	
	printf("Average CPU cycles per exponentiation: %.2f\n", avgclocks);
	printf("Total cycles for %u exponentiations: %d", n, stop-start);
	
    return 0;
}