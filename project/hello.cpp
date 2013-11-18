#include <iostream>
#include <cstring>
#include <fstream>

using namespace std;

int main(int argc, char** argv)
{
	ifstream myfile("smalldata.dat");
	int a;
	while(myfile >> hex >> a) {
		cout << a << endl;
	}
	return 0;	

}
