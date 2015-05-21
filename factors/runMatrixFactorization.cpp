#include <string.h>
#include <iostream>
#include <fstream>
#include <cmath>
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

float  getResult(int *data, float **u, float **v, float ** w, int ** r, int k, DataAccessor *d, Baseline *b){
    float sum = 0;
    for(int i = 0; i < k; i++){
        sum += u[data[0]][i] * v[data[1]][i];
    }
    sum += b->get_baseline(data[0], data[1]);

    int user_id = data[0];
    int movie_id = data[0];

    float sum_ws = 0;
    for (int j = 0; j < 10; j++)
    {
        entry_t e_j = d->get_entry(user_id, r[movie_id][j]);
        int rating_j = d->extract_rating(e_j);
        sum_ws += w[movie_id][r[movie_id][j]] * (rating_j - b->get_baseline(user_id, r[movie_id][j]));
    }
        sum+= (1/ sqrt(10)) *  0.01 * sum_ws;
    return sum;
}


void runMatrixFactorization(float ** u, float **v, float ** w, int ** r, int k, char * inputFile, char *outputFile, DataAccessor * d, Baseline *b){
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

