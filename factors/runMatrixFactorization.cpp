#include <string.h>
#include <iostream>
#include <fstream>
#include "../baseline/baseline.h"
using namespace std;

int *parseLine(string line){
    int *data = new int [2];
    int sep1, sep2;
    sep1 = line.find_first_of(' ');
    sep2 = line.find_first_of(' ', sep1 + 1);
    data[0] = atoi(line.substr(0, sep1).c_str()) - 1;
    data[1] = atoi(line.substr(sep1 + 1, sep2).c_str()) - 1;
    return data;
}

float  getResult(int *data, float **u, float **v, float ** w, int ** r, int k, Baseline *b){
    float sum = 0;
    for(int i = 0; i < k; i++){
        sum += u[data[0]][i] * v[data[1]][i];
    }
    sum += b->get_baseline(data[0], data[1]);
    for (int i = 0; i < 10; i++)
    {
        sum+= 0.01 * w[data[1]][r[data[1]][i]];
    }
    return sum;
}


void runMatrixFactorization(float ** u, float **v, float ** w, int ** r, int k, char * inputFile, char *outputFile, Baseline *b){
    ofstream outFile;
    ifstream inFile;

    outFile.open(outputFile);
    inFile.open(inputFile);


    // loop through data
    string line;
    while(getline(inFile, line)){
	outFile << getResult(parseLine(line), u, v, w, r, k, b) << endl;
    }
}

