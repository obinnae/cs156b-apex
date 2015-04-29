#include <string.h>
#include <iostream>
#include <fstream>
#include "./baseline/baseline.h"
using namespace std;

int *parseLine(string line){
    int *data = new int [2];
    int sep1, sep2;
    sep1 = line.find_first_of(' ');
    sep2 = line.find_first_of(' ', sep1 + 1);
    data[0] = atoi(line.substr(0, sep1).c_str());
    data[1] = atoi(line.substr(sep1 + 1, sep2).c_str());
    return data;
}

double getResult(int *data, double **u, double **v, int k, Baseline *b){
    double sum = 0;
    for(int i = 0; i < k; i++){
        sum += u[data[0]][i] * v[i][data[1]];
    }
    sum += b->get_baseline(data[0], data[1]);
    return sum;
}


void runMatrixFactorization(double ** u, double **v, int k, char * inputFile, char *outputFile, Baseline *b){
    ofstream outFile;
    ifstream inFile;

    outFile.open(outputFile);
    inFile.open(inputFile);


    // loop through data
    string line;
    while(getline(inFile, line)){
	outFile << getResult(parseLine(line), u, v, k, b) << endl;
    }
}

