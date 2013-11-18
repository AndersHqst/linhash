#ifndef BINARYREADERHEADERDEF
#define BINARYREADERHEADERDEF

#include <cstdint>
#include <cstring>
#include <fstream>

class BinaryReader
{
public:
    BinaryReader(std::string filename, int buffersize);
    BinaryReader(std::string filename, int buffersize, std::ifstream::pos_type pos);
	~BinaryReader();
	uint64_t next();
    uint64_t* readarray(int n);
	uint64_t getpos();
private:
	std::string filename;
	std::ifstream binfile;
	std::ifstream::pos_type pos;
	bool posset;

	char* bytes;
	uint64_t* buffer64;

	int buffer64count;
	int buffersize;

	void fillbuffer();
};

#endif
