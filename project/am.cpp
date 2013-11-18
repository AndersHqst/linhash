#include <cstdint>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstring>
#include "modexp.hpp"
#include "gf.hpp"
#include "am.hpp"

typedef std::unordered_set<uint64_t> keyset;

AM::AM(amsettings settings) {
	keyset* keys = loaddata(settings.datafile);
	printf("number of keys in set: %u\n", keys->size());
}

AM::~AM() {

}

keyset* AM::loaddata(const std::string& filename) {
	keyset* data = new keyset();
	std::ifstream datafile(filename);

	uint64_t a;
	while (datafile >> std::hex >> a)
	{
		data->insert(a);
	}
	
	datafile.close();
	return data;
}



