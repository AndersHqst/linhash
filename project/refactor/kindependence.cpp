#include <stdio.h>
#include <cstdint>
#include <xmmintrin.h>
#include <smmintrin.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <ipp.h>
#include <string>
#include <stdlib.h>

typedef uint64_t u64;
typedef uint32_t u32; 

typedef struct {
    u64 hi64;
	u64 mi64;
	u64 lo64;	
} us192;

typedef struct {
    u64 hi64;
	u64 lo64;
} us128;

// Prototypes
inline us192 mul_add(us128 a0, u64 x, us128 a1);
inline us128 add128(us128 a, us128 b);
inline u64 mul_gf(u64 a, u64 x);

us128 kind_hash(int k, us128* a, u64 x);
u64 kind_hash_gf(int k, u64* a, u64 x);

void print_hex(u64 a);
void print_hex(us128 a);
void print_hex(us192 a);


// Computes a0*x + a1
// a0 and a1 are unsigned 96-bit integers 
// x is a 64-bit unsigned integer.
inline us192 mul_add(us128 a0, u64 x, us128 a1)
{
	// Compute result in 32 bit blocks to account for overflow

	const u64 LOW32 = 0xFFFFFFFF;
	us192 r;
	u64 m1, m2, m3, m4, b;
	
	// Notation: 32-bit blocks 
	// a1 =         [ a12 a11 a10 ]
	// a0 =         [ a02 a01 a00 ]
	// x  =              [ x1  x0 ]
	// r  = [  r4  r3  r2  r1  r0 ]
	
	// Create 64-bit variables for the 32-bit blocks
	u64 a12, a11, a10, a02, a01, a00, x1, x0;
	
	a12 = a1.hi64 & LOW32;
	a11 = a1.lo64 >> 32;
	a10 = a1.lo64 & LOW32;
	
	a02 = a0.hi64 & LOW32;
	a01 = a0.lo64 >> 32;
	a00 = a0.lo64 & LOW32;
	
	x1 = x >> 32;
	x0 = x & LOW32;
	
	// r0
	m1 = a00*x0;
	r.lo64 = (m1 & LOW32) + a10;   
	
	// r1
	m2 = a01*x0;
	m3 = a00*x1;
	
	b = (r.lo64 >> 32) + (m1 >> 32) + (m2 & LOW32) + (m3 & LOW32) + a11;
	r.lo64 = (r.lo64 & LOW32) ^ (b << 32); 
	
	// r2
	m1 = a02*x0;
	m4 = a01*x1;
	
	r.mi64 = (b >> 32) + (m2 >> 32) + (m1 & LOW32) + (m3 >> 32) + (m4 & LOW32) + a12; 
	
	// r3
	m2 = a02*x1;
	
	b = (r.mi64 >> 32) + (m1 >> 32) + (m4 >> 32) + (m2 & LOW32);
	r.mi64 = (r.mi64 & LOW32) ^ (b << 32);
		
	// r4
	r.hi64 = (b >> 32) + (m2 >> 32); 
	
	return r;
}

// http://software.intel.com/sites/products/documentation/doclib/iss/2013/compiler/cpp-lin/GUID-5100C4FC-BC2F-4E36-943A-120CFFFB4285.htm
inline us128 add128(us128 a, us128 b)
{
	us128 res;
	
	// AT&T syntax: OPCODE SOURCE DESTINATION
    __asm__(
		// Add a.lo64 and b.lo64 and store it in res.lo64.
		"addq %2, %0;" // DEST <- SRC + DEST
		// Add a.hi64 and b.hi64 and add the value of the carry flag. Store in res.hi64. 
		"adcq %3, %1;" // DEST <- SRC + DEST + CF
		: // Output
		"=r"(res.lo64), "=r"(res.hi64) 
		: // Input 
		"emr"(b.lo64),  "emr"(b.hi64), 
		"0"(a.lo64),    "1"(a.hi64) // values are assigned to same location as output operand 0 and 1, respectively.
	);      
	
	return res;
}

// a mod 2^89-1
inline us128 mod_m89(us192 a)
{
	// Add the leading bits to the lower 89 bits
	us128 lead, res;
	
	// Copy lower 89 bits of a into the result
	res.lo64 = a.lo64;
	res.hi64 = a.mi64 & 0x1FFFFFF;
	
	// lead = [ a.hi64[64:26] a.hi64[25:1] a.mi64[64:26] ]
	lead.lo64 = (a.hi64 << 39) ^ (a.mi64 >> 25); 
	lead.hi64 = a.hi64 >> 25;
		
	res = add128(res, lead);
	
	// Repeat in case of carries (can only happen once)
	lead.lo64 = (res.hi64 >> 25) & 0x1; // Get the 90'th bit (26'th bit of res.hi64)
	lead.hi64 = 0;
	
	// Clear potential overflow bit
	res.hi64 = res.hi64 & 0x1FFFFFF;
	
	res = add128(res, lead);	
	
	return res;
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
	
	// Calculate ax mod (X^64 + X^4 + X^3 + X + 1) using the technique specified by Intels CLMUL white paper
	m = h ^ (h >> 60) ^ (h >> 61);
	res = m ^ (m << 1) ^ (m << 3) ^ (m << 4) ^ l; 
	
	return res;
}

// Calculates the value of a_k_1*x^(k-1) + a_k_2*x^(k-2) + ... + a_1*x + a0 mod M89   
inline us128 kind_hash(int k, us128* a, u64 x)
{
	us128 res = *a;
	for(int i = 1; i < k; i++){
		res = mod_m89(mul_add(res, x, *(a+i)));
	}
	return res;	
}

// Calculates the value of a_k_1*x^(k-1) + a_k_2*x^(k-2) + ... + a_1*x + a0 mod (X^64 + X^4 + X^3 + X + 1)  
inline u64 kind_hash_gf(int k, u64* a, u64 x)
{
	u64 res = *a;
	for(int i = 1; i < k; i++){
		res = mul_gf(res, x) ^ *(a+i);
	}
	return res;	
}

// Hard coded 5-independence
inline u64 fiveind_hash_gf(u64 a0, u64 a1, u64 a2, u64 a3, u64 a4, u64 x)
{
	u64 res;
    res = mul_gf(a4, x) ^ a3;
	res = mul_gf(res, x) ^ a2;
	res = mul_gf(res, x) ^ a1;
	res = mul_gf(res, x) ^ a0;
	return res;	
}

inline us128 fiveind_hash(us128 a0, us128 a1, us128 a2, us128 a3, us128 a4, u64 x)
{
	us128 res;
	res = mod_m89(mul_add(a4, x, a3));
	res = mod_m89(mul_add(res, x, a2));
	res = mod_m89(mul_add(res, x, a1));
	res = mod_m89(mul_add(res, x, a0));
	return res;	
}

inline u64 tabulation(u64** a, u64 x)
{
	u64 LOW8 = 0xFF;
	u64 res = 0;
	
	for(int i = 0; i < 8; i++)
	{
		res = res ^ a[i][((x >> i*8) & LOW8)];
	}
	return res;
}

// -----------------
// TEST
// -----------------

void print_hex(u64 a)
{
	printf("%048I64X\n", a);
}

void print_hex(us128 a)
{
	printf("%032I64X", a.hi64);
	printf("%016I64X\n", a.lo64);
}

void print_hex(us192 a)
{
	printf("%016I64X", a.hi64);
	printf("%016I64X", a.mi64);
	printf("%016I64X\n", a.lo64);
}

void test_mul_add()
{
	// a   = 0x43F2A284A2BF678AD354FF2
	// b   = 0xAB276F4F4DDE2E34
	// a*b = 0x2D6D8A109BED425FF2BBAA264F29697E10DB928
	us128 a;
	a.lo64 = 0x4A2BF678AD354FF2;
	a.hi64 = 0x00000000043F2A28; 
	u64 b  = 0xAB276F4F4DDE2E34;
	us128 zero;
	zero.lo64 = 0;
	zero.hi64 = 0;
	
	printf("a, b, a*b, correct result\n");
	print_hex(a);
	print_hex(b);
	print_hex(mul_add(a, b, zero));
	printf("%048s", "2D6D8A109BED425FF2BBAA264F29697E10DB928");
		
	// c = 0xE3AF765FE331AADD4561FFF;
	// a*b + c = 0x2D6D8A109BED4260D66B2086325B145B563D927
	us128 c;
	c.lo64 = 0xFE331AADD4561FFF;
	c.hi64 = 0xE3AF765;
	
	printf("\na, b, c, a*b + c, correct result\n");
	print_hex(a);
	print_hex(b);
	print_hex(c);
	print_hex(mul_add(a, b, c));
	printf("%048s", "2D6D8A109BED4260D66B2086325B145B563D927");
	
}

void test_mod_m89()
{
	us192 a;
	a.lo64 = 0;
	a.mi64 = ~0;
	a.hi64 = 0;
	printf("a, a mod m89, correct answer\n");
	print_hex(a);
	print_hex(mod_m89(a));
	printf("%048s\n\n", "1FFFFFF0000007FFFFFFFFF");
	
	a.lo64 = ~0;
	a.mi64 = ~0;
	a.hi64 = 0;
	printf("carry test \n");
	printf("a, a mod M89, correct answer\n");
	print_hex(a);
	print_hex(mod_m89(a));
	printf("%048s\n\n", "7FFFFFFFFF");
	
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

double avg_time(Ipp64u start, Ipp64u stop, int n)
{
	Ipp64u elapsed = stop - start;
	double avg = (double)elapsed / (double)n;
	return avg;
}

void hashes_to_file(std::string filename, u32* out, int n, bool write)
{
	if(write){
		// Use the values to make sure the the code is actually executed
		std::ofstream outfile;
		outfile.open(filename);
		for (int i = 0; i < n; i++) {
			outfile << out[i] << std::endl;
		}
		outfile.close();	
	}
}


// Should be expanded to test randomized input held up against some verified output
void hash_test()
{
	// BAD STYLE
	us128 a[3] = {{0,5}, {0,7}, {0, 6}};
	u64 x = 20;
	print_hex(kind_hash(3, a, x));
}

void hash_test_gf()
{
	u64 a[3] = { 0xFFF, 0xA545E, 0x6546545};
	u64 x = 0x123147686EEE;
	print_hex(kind_hash_gf(3, a, x));
}

void full_test(int maxk, int n, bool write)
{
	// Load random numbers
	u64 a64[12] = 
	{
		0xF3213B754DDE9885,
		0x50521C2BD23EC6AE,
		0x0EAAD2B1D02BABB9,
		0x9A0F82FE42ACAF86,
		0x8C853D551A9C2500,
		0xFDE927E4F0137AB6,
		0xBC66098E47FB4128,
		0x2162FAD57BF5F1CF,
		0xFFF0798079E18953,
		0xF9089C1A6083C9A6,
		0x967A2A6581F71CAD,
		0x1B3ABA2ED5340CD4
	};
	
	us128 a128[6];
	us128 m;
	for(int i = 0; i < 6; i++)
	{
		m.lo64 = a64[2*i];
		m.hi64 = a64[2*i +1];
		a128[i] = m;
	}
	
	// Random numbers for tabulation hashing
	u64** tables = new u64*[8];
	for(int t = 0; t < 8; t++){
		tables[t] = new u64[255];
		for(int i = 0; i < 255; i++){
			tables[t][i] = (((u64)rand()) << 32) ^ ((u64)rand()); 
		}
	}
	
	// 2D array for clock results
	double** res; // layout: [k][func]
	res = new double*[maxk-1];
	
	// Hash output
	u32* out = new u32[n];
	
	Ipp64u start, stop;
	
	// Test each approach for k = 2,3,..,maxk
	for(int k = 2; k <= maxk; k++)
	{
		res[k-2] = new double[4];
		
		// M89
		start = ippGetCpuClocks();
		for (volatile u64 j = 0; j < n; j++) {
			out[j] = (u32)(kind_hash(k, a128, j).lo64);
		}
		stop = ippGetCpuClocks();
		res[k-2][0] = avg_time(start, stop, n);
		hashes_to_file("m89_" + std::to_string(k) + ".txt", out, n, write);
		
		// GF
		start = ippGetCpuClocks();
		for (volatile u64 j = 0; j < n; j++) {
			out[j] = (u32)(kind_hash_gf(k, a64, j));
		}
		stop = ippGetCpuClocks();
		res[k-2][1] = avg_time(start, stop, n);
		hashes_to_file("gf_" + std::to_string(k) + ".txt", out, n, write);
		
		// MULTADDSHIFT
		start = ippGetCpuClocks();
		for (volatile u64 j = 0; j < n; j++) {
			out[j] = (u32)(a64[0]*j + a64[1]);
		}
		stop = ippGetCpuClocks();
		res[k-2][2] = avg_time(start, stop, n);
		hashes_to_file("mas_" + std::to_string(k) + ".txt", out, n, write);
		
		// TABULATION
		start = ippGetCpuClocks();
		for (volatile u64 j = 0; j < n; j++) {
			out[j] = tabulation(tables, j);
		}
		stop = ippGetCpuClocks();
		res[k-2][3] = avg_time(start, stop, n);
		hashes_to_file("tab_" + std::to_string(k) + ".txt", out, n, write);
	}
	
	// print results
	printf("%10s %10s %10s %10s %10s\n", "k", "M89", "GF", "TAB", "MAS");
	for(int k = 2; k <= maxk; k++)
	{
		printf("%10u %10.2f %10.2f %10.2f %10.2f\n", k, res[k-2][0], res[k-2][1], res[k-2][3], res[k-2][2]);
	}
}

void five_test(int n, bool write)
{
	// Load random numbers
	u64 a64[12] = 
	{
		0xF3213B754DDE9885,
		0x50521C2BD23EC6AE,
		0x0EAAD2B1D02BABB9,
		0x9A0F82FE42ACAF86,
		0x8C853D551A9C2500,
		0xFDE927E4F0137AB6,
		0xBC66098E47FB4128,
		0x2162FAD57BF5F1CF,
		0xFFF0798079E18953,
		0xF9089C1A6083C9A6,
		0x967A2A6581F71CAD,
		0x1B3ABA2ED5340CD4
	};
	
	us128 a128[6];
	us128 m;
	for(int i = 0; i < 6; i++)
	{
		m.lo64 = a64[2*i];
		m.hi64 = a64[2*i +1];
		a128[i] = m;
	}
	
	double m89_avg;
	double gf_avg;
	Ipp64u gf_elapsed;
	Ipp64u m89_elapsed;
	
	u64 a0, a1, a2, a3, a4;
	a0 = a64[0];
	a1 = a64[1];
	a2 = a64[2];
	a3 = a64[3];
	a4 = a64[4];
	
	us128 aa0, aa1, aa2, aa3, aa4;
	aa0 = a128[0];
	aa1 = a128[1];
	aa2 = a128[2];
	aa3 = a128[3];
	aa4 = a128[4];
	
	
	// Hash output
	u32* out = new u32[n];
	
	Ipp64u start, stop;
	
	start = ippGetCpuClocks();
	for (volatile u64 j = 0; j < n; j++) {
		out[j] = (u32)(fiveind_hash(aa0, aa1, aa2, aa3, aa4, j).lo64);
	}
	stop = ippGetCpuClocks();
	m89_elapsed = stop - start;
	m89_avg = avg_time(start, stop, n);
	hashes_to_file("m89_five.txt", out, n, write);
		
	start = ippGetCpuClocks();
	for (volatile u64 j = 0; j < n; j++) {
		out[j] = (u32)fiveind_hash_gf(a0, a1, a2, a3, a4, j);
	}
	stop = ippGetCpuClocks();
	gf_elapsed = stop - start;
	gf_avg = avg_time(start, stop, n);
	hashes_to_file("gf_five.txt", out, n, write);
	
	// print results
	printf("%10s %10s %10s\n", " ", "M89", "GF");
	printf("%10s %10d %10d\n", "Cycles", m89_elapsed, gf_elapsed);
	printf("%10s %10.2f %10.2f\n", "Avg", m89_avg, gf_avg);
}

// maxk n write
int main(int argc, char** argv) 
{
//	int maxk = atoi(argv[1]);
//	int n = atoi(argv[2]);
//	bool write = atoi(argv[3]) == 1 ? true : false;
//	full_test(maxk, n, write);
	test_gf_mul();
    return 0;
}
