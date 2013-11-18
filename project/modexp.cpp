#include <cstdint>
#include <cstdio>
#include "modexp.hpp"
#include "gf.hpp"

ModExp::ModExp(const GF& gf, uint64_t* base, int charsize, int numchars) : field(gf), charsize(charsize), numchars(numchars) 
{
	// Fill in the tables with precomputed powers of the base
	// tables[charpos][charval][word of base to this power]
	tables = new uint64_t**[numchars];
	int entries = 1 << charsize;
	uint64_t* nextpower = field.copyelement(base);	
	
	for(int i = 0; i < numchars; i++) {	
		tables[i] = new uint64_t*[entries];
		tables[i][0] = field.identity();
		
		for(int j = 1; j < entries; j++) {
			tables[i][j] = field.unassigned();
			field.mul(nextpower, tables[i][j-1], tables[i][j]);
		}

		for(int t = 0; t < charsize; t++) {
			field.mul(nextpower, nextpower, nextpower); // repeated squaring
		}
	}
	delete[] nextpower;
}

ModExp::~ModExp() {
	// Seems like you have to delete everyting manually
	for(int i = 0; i < numchars; i++) {
		for(int j = 0; j < (1 << charsize); j++) {
			delete[] tables[i][j];
		}
		delete[] tables[i];
	}
	delete[] tables;
}


void ModExp::power(uint64_t z, uint64_t* res) {
	uint64_t mask = ~(~0UL << charsize);
	field.copyinto(tables[0][z & mask], res);

	for(int i = 1; i < numchars; i++) {
		field.mul(tables[i][(z >> i*charsize) & mask], res, res);
	}
}

