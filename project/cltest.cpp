#include <cstdint>
#include <cstdio>
#include <xmmintrin.h> 
#include <smmintrin.h>

typedef struct {
    uint64_t hi64;
    uint64_t lo64;
} uint128;

inline uint128 clmul(uint64_t a, uint64_t b)
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

int main(int argc, char** argv)
{
    uint128 res = clmul(0xf, 0xe);
    // Print first 16 from longs in the struct
    printf("%016llX%016llX\n", res.hi64, res.lo64);
    
    return 0;   
}