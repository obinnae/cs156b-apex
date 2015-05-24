#include <string.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include "../baseline/baseline.h"
#include "update_parameters.h"

using namespace std;

int * parseLine(string line){
    int *data = new int [2];
    int sep1, sep2;
    sep1 = line.find_first_of(' ');
    sep2 = line.find_first_of(' ', sep1 + 1);
    data[0] = atoi(line.substr(0, sep1).c_str()) - 1;
    data[1] = atoi(line.substr(sep1 + 1, sep2).c_str()) - 1;
    return data;
}

float getResult(int *data, float **u, float **v, float ** w, int ** r, int k, DataAccessor *d, Baseline *b){
    
    int user_id = data[0];
    int movie_id = data[1];
    float sum = 0;

    for(int i = 0; i < k; i++){
        sum += u[user_id][i] * v[movie_id][i];
    }

    sum += b->get_baseline(user_id, movie_id);

    sum+= (1/ sqrt(10)) *  0.001 * weightSum(user_id, movie_id, 10, w, r, d, b);
    return sum;
}


void runMatrixFactorization(float ** u, float ** v, float ** w, int ** r, int k, char * inputFile, char * outputFile, DataAccessor * d, Baseline *b){
    ofstream outFile;
    ifstream inFile;

    outFile.open(outputFile);
    inFile.open(inputFile);


    // loop through data
    string line;
    while(getline(inFile, line)){
	outFile << getResult(parseLine(line), u, v, w, r, k, d, b) << endl;
    }
}

