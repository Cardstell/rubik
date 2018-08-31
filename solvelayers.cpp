#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string>

using namespace std;

const int gDSU[] = {0,1,2,3,4,5,6,7,8};
const int gDSR[] = {8,5,2,17,14,11,26,23,20};
const int gDSF[] = {6,7,8,15,16,17,24,25,26};
const int gDSD[] = {24,25,26,21,22,23,18,19,20};
const int gDSL[] = {0,3,6,9,12,15,18,21,24};
const int gDSB[] = {2,1,0,11,10,9,20,19,18};

struct cube {
    char L='0',R='0',U='0',D='0',F='0',B='0';
} cubes[27];

void RAC(string colors) {
	if (cubes[4].U != colors[1]) {
		
	}
}

string toAlg(string alg) {

}

void cubeFromString(string cube) {

}

string solvelayers(string cube) {
	cubeFromString(cube);
}

int main() {
	string cube;
	ifstream file;
	file.open("cache/shared");
	file>>cube;
	file.close();
	solvelayers(cube);
	return 0;
}