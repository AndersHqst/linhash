#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <fstream>
#include "binaryreader.hpp"

// Constructor for reading the binary file from the beginning
BinaryReader::BinaryReader(std::string filename, int buffersize)
{
	this->buffersize = buffersize;
	bytes = new char[8*buffersize];
	buffer64 = new uint64_t[buffersize];
	this->filename = filename;
	posset = false;
	buffer64count = 0;
}

// Constructor for reading binary file starting at pos
BinaryReader::BinaryReader(std::string filename, int buffersize, std::ifstream::pos_type pos) 
: BinaryReader(filename, buffersize) {
	this->pos = pos;
	posset = true;
}

BinaryReader::~BinaryReader()
{
	delete[] bytes;
	delete[] buffer64;
}

void BinaryReader::fillbuffer()
{
	binfile.open(filename, std::ios::binary | std::ios::in);
	if(posset) {
		binfile.seekg(pos);
	}
	if(!binfile.read(bytes, 8*buffersize)) {
		throw std::runtime_error("Error reading file");
	}
	pos = binfile.tellg();
	posset = true;
	binfile.close();

	// Convert bytes to uint64_t's
	for(int i = 0; i < buffersize; i++)
	{
		buffer64[i] = 0;
		
		// Manual load
		for(int j = 0; j < 8; j++)
		{
			buffer64[i] ^= ((uint64_t)bytes[8*i + j]) << 8*j;
	    }
		buffer64count = buffersize;
	}
}

uint64_t BinaryReader::next()
{
	if(buffer64count ==  0){
		fillbuffer();
	}
	buffer64count--;
	return buffer64[buffer64count];
}

uint64_t* BinaryReader::readarray(int n)
{
	uint64_t* numbers = new uint64_t[n];
	
	for(int i = 0; i < n; i++){
		numbers[i] = next();
	}

	return numbers;
}

uint64_t BinaryReader::getpos()
{
	return (uint64_t)pos;
}
