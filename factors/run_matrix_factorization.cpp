/*
 * Nancy Wen
 * April 21, 2015
 *
 * This program uses the gradient calculations to update the latent factors.
 */
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <cmath>

#include "sgd.h"
#include "../baseline/baseline.h"
#include "../DataAccessor/data_accessor.h"
#include "runMatrixFactorization.cpp"
#include "matrix_factorization.h"
using namespace std;



void run_matrix_factorization(int factors, char * data_path, char * probe_path, int epochs, float lambda, float lrate, char * qualPath, char * outputPath, int folds=-1)
{ //Included probe path in signature

  // declare the number of epochs of SGD you want to do
  // # epochs = (# iters) / (# total entries in data file)

  DataAccessor d;
  d.load_data(data_path);

  DataAccessor p; // Creating 2nd DataAccesor to manage probe data
  p.load_data(probe_path);
  
  Baseline b(&d); // Baseline instantiation
  Baseline b_p(&p); // Baselines for probe data
  
  int num_users = d.get_num_users();
  int num_movies = d.get_num_movies();

  // Declare and allocate memory for latent factor matrices
  // We allocate an entire block of memory for the two matrices so that
  // cache misses are reduced.
  float *factors_mem = new float[(num_users + num_movies) * factors];

  // Declare and allocate memory for easier access into the latent factor matrices
  // This allows factors to be accessed in the same way as before.
  float ** U = new float *[num_users];
  for(int i = 0; i < num_users; i++) {
    U[i] = factors_mem + i * factors;
  }

  float ** V = new float *[num_movies];
  for(int i = 0; i < num_movies; i++) {
    V[i] = factors_mem + (i + num_users) * factors;
  }

  srand(time(NULL));

  // calculate U and V
  if (folds <= 1) {
    single_fold_factorization(U, V, factors, epochs, lambda, lrate, &d, &p, &b, &b_p); // Added probe dataAccessor and Baseline object to call
  } else {
    k_fold_factorization(U, V, factors, epochs, lambda, lrate, folds, &d, &b);
  }

  // create qual submission using latent factors
  runMatrixFactorization(U, V, factors, qualPath, outputPath, &b);

  // Clean up after yourself
  delete[] U;
  delete[] V;
  delete[] factors_mem;

}



int main(int argc, char *argv[]) {
  char *data_path, *probe_path, *qualPath, *outputPath; //Added probe variable for reading in probe param
  int num_factors;
  int num_epochs;
  float lambda, lrate;
  int num_folds;
  
  if (argc == 9) { // Changed Counts on argument length checks to accomodate for probe
    // Also adjusted indices of arguments following probe
    num_folds = -1;
    qualPath = argv[7];
    outputPath = argv[8];
  } else if (argc == 10){
    num_folds = atoi(argv[7]);
    qualPath = argv[8];
    outputPath = argv[9];
    std::cout << "Error: k-fold factorization isn't functional right now. Validation shold be performed using the probe data set. Exiting...\n";
    exit(1);
  } else {
    std::cout << "Usage: run_matrix_factorization <train-data-file> <probe-data-file> <num-factors> <num-epochs> <lambda> <learning-rate> [<#-folds>] <qual_path> <output-file-path>\n";
    /*
     * Modified usage message to clarify the extra command line arg
     * Also renamed old data-file arg to differentiate between train-data and probe-data
     */
    exit(1);
  }
  data_path = argv[1];
  probe_path = argv[2];
  num_factors = atoi(argv[3]); //Incremented indices of args since probe has been inserted after data_path
  num_epochs = atoi(argv[4]);
  lambda = atof(argv[5]);
  lrate = atof(argv[6]);

  std::cout << "Running matrix factorization (standard SGD) with the following parameters:\n"
      << "\tData file: " << data_path << std::endl
      << "\tNumber of factors: " << num_factors << std::endl
      << "\tNumber of epochs: " << num_epochs << std::endl
      << "\tLambda: " << lambda << std::endl
      << "\tLearning rate: " << lrate << std::endl
      << "\tNumber of folds: " << num_folds << std::endl;

  run_matrix_factorization(num_factors, data_path, probe_path, num_epochs, lambda, lrate, qualPath, outputPath, num_folds);

  std::cout << "\nMatrix factorization finished!\n";

}
/* NOTES

Line 163 ...  Baseline instantiation? Where do we create instance of it. Same with DataAccesor

*/
