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

float  getResult(float **u, float **v, int k, int index, DataAccessor *d, Baseline *b){
    entry_t e = d->get_entry(index);
    int user_id = d->extract_user_id(e);
    int movie_id = d->extract_movie_id(e);

    float sum = 0;
    for(int i = 0; i < k; i++){
        sum += u[user_id][i] * v[movie_id][i];
    }
    sum += b->get_baseline(user_id, movie_id);
    return sum;
}


void runMatrixFactorization(float ** u, float **v, int k, DataAccessor *data, Baseline *b, char *outputFile){
    ofstream outFile;

    outFile.open(outputFile);


    // loop through data
    for (int i = 0; i < data->get_num_entries(); i++) {
        outFile << getResult(u, v, k, i, data, b) << endl;
    }
}

