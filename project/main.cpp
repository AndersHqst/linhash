#include <cstdio>
#include "gf.hpp"

int main(int argc, char* argv)
{
	int k = 2;
	int l = 3;
	int p[3] = { 7, 2, 1 }; 
	GF field(k, l, p);
	
	uint64_t a[] = {0x67dce43221b3c482, 0x5f16daf23e5041ed};
	uint64_t b[] = {0x2b05c90e1a794fc0, 0x48dee6c0fb29071a};
	
	//int k = 1;
	//int l = 3;
	//int p[3] = { 4, 3, 1 }; 
	//GF field(k, l, p);
	
	//uint64_t a[] = { 0x81E37D20AEFFEA39 }; 
	//uint64_t b[] = { 0x8BF26934A1D851B5 };
	
	field.print(a); 
	printf("\n");
	field.print(b);
	printf("\n");
    field.print(field.mul(a,b));
	printf("\n");
	//for(int i = 0; i < 100; i++){
	//	a[0] ^= i;
	//	b[1] ^= i;
	//	field.print(field.mul(a,b));
	//	printf("\n");
	//}	
	
	return 0;	
}
