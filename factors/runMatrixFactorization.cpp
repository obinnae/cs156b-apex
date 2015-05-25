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

                                    //runMatrixFactorization(U, V, w, r, factors, output_path, &d, &p, &q, &b, &b_p, &b_q);
void runMatrixFactorization(float ** u,
                            float ** v,
                            float ** w,
                            int ** r,
                            int k,
                            char * probe_output_path,
                            char * qual_output_path,
                            DataAccessor * d,
                            DataAccessor * p,
                            DataAccessor * q,
                            Baseline *b){
    ofstream probeOutFile;
    ofstream qualOutFile;

    probeOutFile.open(probe_output_path);

    int * data = new int [2];

    for (int i = 0; i < p->get_num_entries(); i++)
    {
        entry_t e_p = p->get_entry(i);
        data[0] = p->extract_user_id(e_p);
        data[1] = p->extract_movie_id(e_p);
        probeOutFile << getResult(data, u, v, w, r, k, d, b) << endl;
    }
    probeOutFile.close();

    qualOutFile.open(qual_output_path);
    for (int i = 0; i < q->get_num_entries(); i++)
    {
        entry_t e_q = q->get_entry(i);
        data[0] = q->extract_user_id(e_q);
        data[1] = q->extract_movie_id(e_q);
        qualOutFile << getResult(data, u, v, w, r, k, d, b) << endl;
    }
    qualOutFile.close();
}

