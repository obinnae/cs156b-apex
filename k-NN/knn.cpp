/*
 * Timothy Chou
 * May 20, 2015
 *
 * This program implements k-nearest neighbor model
 *
 */

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <cmath>

#include "../DataAccessor/data_accessor.h"
using namespace std;

#define MAX_MOVIES 17770
#define MAX_USERS 458293

typedef struct Tuple{
    /* structs to hold a user id and the closeness metric
     * val should be from 0 to 1 */
    int id;
    double val;
}Tuple;

void run_knn(char *data_path, char *probe_path, char *qualPath, char *outputPath, int k, int q, int num_users, int num_moves);
void trainKnn(int k, int q, Tuple **list, int num_users, DataAccessor *d);
double calcCloseness(int user1, int user2, DataAccessor *d);
void testKnn(int k, Tuple **list, DataAccessor *d);
void outputKnn(int k, Tuple **list, char *qualPath, char *outputPath, DataAccessor *d);
int *parseLine(string line);
double getPrediction(int k, int userIdx, int movieIdx, Tuple **list, DataAccessor *d);
double getResult(int k, int *data, Tuple **list, DataAccessor *d);


Tuple** init(int k, int num_users){
    /* create a empty matrix with k entries per user */
    Tuple **list = new Tuple *[num_users];
    for (int i = 0; i < num_users; i++){
        list[i] = new Tuple[k];
    }
    return list;
}

void run_knn(char *data_path, char *probe_path, char *qualPath, char *outputPath, int k, int q, int num_users, int num_movies){
    /* initiate an empty matrix for the nearest neighbors */
    Tuple **list = init(k, num_users);

    /* data accessors for the data and probes */
    DataAccessor d;
    d.load_data(data_path);
    DataAccessor p;
    p.load_data(probe_path);

    /* train our knn on data_path */
    trainKnn(k, q, list, num_users, &d);

    /* test knn on probe */
    testKnn(k, list, &p);


    /* create a entry file based on qual */
    outputKnn(k, list, qualPath, outputPath, &d);
    
}

void trainKnn(int k, int q, Tuple **list, int num_users, DataAccessor *d){
    /* trains a knn model 
     *
     * k is the number of nearest neighbors for each user
     * q is the number of users we examine as potential nearest neighbors.
     *   it is generally smaller than numUsers as we can just use the
         users with the most movies to measure clustering
     * list is the matrix of knn
     * num_users is the number of users
     * d is the data accessor class 
     */
    
    /* bounds check */
    if (q > num_users){
        q = num_users;
    }

    entry_t *user_entries;
    
    /* ids of our top q users who have watched the most movies */
    
    int* Q = new int[q];
    int* numMoviesWatched = new int[q];
    int qSize = 0;
    int curMin = 1000000000;
    for (int i = 0; i < num_users; i++){
        if (qSize < q){
            Q[qSize] = i;
            numMoviesWatched[qSize] = d->get_user_entries(i, user_entries);
            if (numMoviesWatched[qSize] < curMin){
                curMin = numMoviesWatched[qSize];
            }
            qSize++;
        }else {
            int moviesWatched = d->get_user_entries(i, user_entries);
            if (moviesWatched > curMin){
                /*add to Q and remove smallest entry of Q */

                int validMin = 0;
                while (numMoviesWatched[validMin] > moviesWatched){
                    validMin++;
                }
                for (int j = validMin; j < q; j++){
                    if (numMoviesWatched[j] < numMoviesWatched[validMin]){
                        validMin = j;
                    }
                }
                
                Q[validMin] = i;
                numMoviesWatched[validMin] = moviesWatched;
                curMin = numMoviesWatched[0];
                for (int j = 0; j < q; j++){
                    if (curMin > numMoviesWatched[j]){
                        curMin = numMoviesWatched[j];
                    }
                }
            }
        }
    }

    
    /* loop through users and fine correlation metric with it and the top-Q
     * then keep the top-K of those */
    Tuple *closeness = new Tuple[q];
    
    for (int i = 0; i < num_users; i++){
        for (int j = 0; j < q; j++){
            closeness[j].val = calcCloseness(i, Q[j], d);
            closeness[j].id = Q[j];
        }

        /* copy over top-K */
        //sort(closeness);
        for (int j = 0; j < k; j++){
            /* TODO: implement thresholding, so not just naively
             * top K. Sometimes want less */

            list[i][j] = closeness[j];
        }
    }
    
}

double calcCloseness(int user1, int user2, DataAccessor *d){
    /* calculates closeness metric between two users 
     * currently uses pearson correlation coefficient */
    Tuple *closenessMeasure = new Tuple;
    entry_t *entry;
    int numEntries = d->get_user_entries(user1, entry);
    double avg1 = 0; /*replace this with maybe
                     * avg = d->getUserAvg(user1);
                     */
    double covariance = 0;
    double sd1 = 0;
    double sd2 = 0;
    int numSharedMovies = 0;
    int rating1, rating2;
    
    double avg2 = 0; /* same, but use d->getUserAvg(user2); */
    
    for (int i = 0; i < numEntries; i++){
        /* check if user2 also has this movie rated */
        if (d->has_entry(user2, d->extract_movie_id(entry[i]))){
            numSharedMovies++;
            rating1 = d->extract_rating(entry[i]);
            rating2 = d->extract_rating(d->get_entry(user2, d->extract_movie_id(entry[i])));
            
            covariance += (rating1 - avg1) * (rating2 - avg2);
            sd1 += pow(rating1 - avg1, 2);
            sd2 += pow(rating2 - avg2, 2);
            
        }
    }
    
    return 1/numSharedMovies * covariance / (sqrt(1/numSharedMovies * sd1) * sqrt(1/numSharedMovies * sd2));
}

void testKnn(int k, Tuple **list, DataAccessor *d){
    /* tests a knn model */
    entry_t entry;
    double error;
    for (int i = 0; i < d->get_num_entries(); i++){
        entry = d->get_entry(i);
        int userIdx = d->extract_user_id(entry);
        int movieIdx = d->extract_movie_id(entry);
        
     
        error += pow(getPrediction(k, userIdx, movieIdx, list, d) - d->extract_rating(entry),2);

    }
    std::cout << "E_int: " << sqrt(error / d->get_num_entries()) << "\n";
                
                    
}

void outputKnn(int k, Tuple **list, char *qualPath, char *outputPath, DataAccessor *d){
    /* outputs the results of knn prediction on qualPath into outputPath */
    ofstream outFile;
    ifstream inFile;
    
    outFile.open(outputPath);
    inFile.open(qualPath);
    
    /* loop through data */
    string line;
    while(getline(inFile, line)){
        outFile << getResult(k, parseLine(line), list, d) << endl;
    }
}

int *parseLine(string line){
    int *data = new int[2];
    int sep1, sep2;
    sep1 = line.find_first_of(' ');
    sep2 = line.find_first_of(' ', sep1 + 1);
    data[0] = atoi(line.substr(0, sep1).c_str()) - 1;
    data[1] = atoi(line.substr(sep1 + 1, sep2).c_str()) - 1;
    return data;
}

double getResult(int k, int *data, Tuple **list, DataAccessor *d){
    return getPrediction(k, data[0], data[1], list, d);
}



double getPrediction(int k, int userIdx, int movieIdx, Tuple **list, DataAccessor *d){
    double totalWeight = 0;
    double rating = 0;
    double avg1, avg2;
    avg1 = 0; /*replace with
               * avg1 = d->getUserAvg(userIdx);
               */
    for (int i = 0; i < k; i++){
        int neighborIdx = list[userIdx][i].id;
        if (d->has_entry(neighborIdx, movieIdx)){
            avg2 = 0; /* replace with
                       * avg2 = d->getUserAvg(neighborIdx);
                       */
            totalWeight += list[userIdx][i].val;
            rating += (d->extract_rating(d->get_entry(neighborIdx, movieIdx)) - avg2) * list[userIdx][i].val;
        }
    }
    rating /= totalWeight;
    rating += avg1;
    return rating;

}


int main(int argc, char *argv[]){
    char *data_path, *probe_path, *qualPath, *outputPath;

    int k, q;

    if (argc != 7){
        std::cout << "Usage: knn <train-data-file> <probe-data-file> <qual-file-path> <output-file-path> <# of nearest neighbors> <# of most-active users to examine>\n";
        exit(1);
    }


    data_path = argv[1];
    probe_path = argv[2];
    qualPath = argv[3];
    outputPath = argv[4];
    k = atoi(argv[5]);
    q = atoi(argv[6]);

    std::cout << "Running kNN with the following parameters: \n"
              << "\tData file: " << data_path << std::endl
              << "\tNumber of nearest neighbors: " << k << std::endl;

    run_knn(data_path, probe_path, qualPath, outputPath, k, q, MAX_USERS, MAX_MOVIES);

    std::cout << "\nk-NN finished!\n";
    
    return 0;
}


    
        

