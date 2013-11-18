#ifndef AMHEADERDEF
#define AMHEADERDEF

#include <cstdint>
#include <cstring>
#include <unordered_set>
#include "gf.hpp"
#include "modexp.hpp"


struct amsettings {
	// File settings
	std::string datafile;
	std::string randfile;
	int buffersize;

	// Field settings
	int fieldwords;
	int polynomialterms;
	int* irreduciblepolynomial;
	
	// ModExp settings
	int charsize;
	int numchars;

	// AM settings
	int maxfalsepositive;
	int independencedegree;
	int levelwaste;

};

class AM
{
public:
	AM(amsettings settings);
	~AM();

	bool member(uint64_t z);
private:
	std::unordered_set<uint64_t>* loaddata(const std::string& filename);

	amsettings settings;
// 	GF field;
// 	ModExp exp;
	int levels;
	uint64_t* keyreduction;
	uint64_t* fieldreduction;
	uint64_t*** kindhash;
};

#endif
