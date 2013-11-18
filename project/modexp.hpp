#ifndef MODEXPHEADERDEF
#define MODEXPHEADERDEF

#include <cstdint>
#include "gf.hpp"

class ModExp
{
public:
	ModExp(const GF& field, uint64_t* base, int charsize, int numchars);
	~ModExp();
	void power(uint64_t z, uint64_t* res); 
private:
	uint64_t*** tables;
	const int charsize;
	const int numchars;
	const GF& field;
};

#endif
