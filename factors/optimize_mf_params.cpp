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
#include "matrix_factorization.h"

// Code stolen from http://stackoverflow.com/questions/717762/how-to-calculate-the-vertex-of-a-parabola-given-three-points
// but it's not that hard to derive manually....
float find_parabola_min(float x1, float y1, float x2, float y2, float x3, float y3) {
  float denom = (x1 - x2) * (x1 - x3) * (x2 - x3);
  float A     = (x3 * (y2 - y1) + x2 * (y1 - y3) + x1 * (y3 - y2)) / denom;
  float B     = (x3*x3 * (y1 - y2) + x2*x2 * (y3 - y1) + x1*x1 * (y2 - y3)) / denom;
  float C     = (x2 * x3 * (x2 - x3) * y1 + x3 * x1 * (x3 - x1) * y2 + x1 * x2 * (x1 - x2) * y3) / denom;

  float vx = -B / (2*A);
  float vy = C - B*B / (4*A);

  std::cout << "Fit to parabola: E = " << A << " x^2 + " << B << " x + " << C << std::endl;
  std::cout << "Parabola vertex @ (" << vx << ", " << vy << ")\n";
  return vx;
}

float optimize_lrate(float **U, float **V, int factors, int epochs, float lambda, float lrate, DataAccessor *d, Baseline *b, DataAccessor *d_p, Baseline *b_p) {
  float step = 9;
  for (int e = 0; e < epochs; e++) {
    float ratio = (1 + step/(e+1));
    float high_lrate = lrate * ratio;
    float low_lrate = lrate / ratio;

    std::cout << "\n*** Running mf with lrate = " << lrate << " / " << ratio << " = " << low_lrate << std::endl;
    float e_ll = single_fold_factorization(U, V, factors, 3, lambda, low_lrate, d, d_p, b, b_p);

    std::cout << "\n*** Running mf with lrate = " << lrate << std::endl << std::endl;
    float e_ml = single_fold_factorization(U, V, factors, 3, lambda, lrate, d, d_p, b, b_p);

    std::cout << "\n*** Running mf with lrate = " << lrate << " * " << ratio << " = " << high_lrate << std::endl;
    float e_hl = single_fold_factorization(U, V, factors, 3,  lambda, high_lrate, d, d_p, b, b_p);

    lrate = exp(find_parabola_min(log(high_lrate), e_hl, log(lrate), e_ml, log(low_lrate), e_ll));
    std::cout << "\n*** Minimum of parabolic-log fit @ " << lrate << std::endl << std::endl;
  }

  return lrate;
    
}


float optimize_lambda(float **U, float **V, int factors, int epochs, float lambda, float lrate, DataAccessor *d, Baseline *b, DataAccessor *d_p, Baseline *b_p) {
  float step = 9;
  for (int e = 0; e < epochs; e++) {
    float ratio = (1 + step/(e+1));
    float high_lambda = lambda * ratio;
    float low_lambda = lambda / ratio;

    std::cout << "\n*** Running mf with lambda = " << lambda << " / " << ratio << " = " << low_lambda << std::endl;
    float e_ll = single_fold_factorization(U, V, factors, 1, low_lambda, lrate, d, d_p, b, b_p);

    std::cout << "\n*** Running mf with lambda = " << lambda << std::endl;
    float e_ml = single_fold_factorization(U, V, factors, 1, lambda, lrate, d, d_p, b, b_p);

    std::cout << "\n*** Running mf with lambda = " << lambda << " * " << ratio << " = " << high_lambda << std::endl;
    float e_hl = single_fold_factorization(U, V, factors, 1,  high_lambda, lrate, d, d_p, b, b_p);

    /*if (e_ll < e_ml && e_ll < e_hl) {
      lambda = low_lambda;
    } else if (e_hl < e_ll && e_hl < e_ml) {
      lambda = high_lambda;
    }
    std::cout << "Choosing lambda = " << lambda << std::endl;*/
    lambda = exp(find_parabola_min(log(high_lambda), e_hl, log(lambda), e_ml, log(low_lambda), e_ll));
    std::cout << "\n*** Minimum of parabolic-log fit @ " << lambda << std::endl;
    if (lambda < low_lambda) lambda = low_lambda;
    if (lambda > high_lambda) lambda = high_lambda;
    std::cout << "*** Using lambda = " << lambda << std::endl << std::endl;
  }

  return lambda;
    
}


void optimize_params(int factors, char * data_path, char * probe_path, int epochs, float lambda, float lrate)
{ //Included probe path in signature

  // declare the number of epochs of SGD you want to do
  // # epochs = (# iters) / (# total entries in data file)

  DataAccessor d;
  d.load_data(data_path);

  DataAccessor d_p; // Creating 2nd DataAccesor to manage probe data
  d_p.load_data(probe_path);
  
  Baseline b(&d); // Baseline instantiation
  Baseline b_p(&d_p); // Baselines for probe data
  
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

  //float opt_lrate = optimize_lrate(U, V, factors, epochs, lambda, lrate, &d, &b, &d_p, &b_p);

  //std::cout << "Optimized lrate: " << opt_lrate << std::endl << std::endl;

  float opt_lambda = optimize_lambda(U, V, factors, epochs, lambda, lrate, &d, &b, &d_p, &b_p);
  
  std::cout << "Optimized lambda: " << opt_lambda << std::endl << std::endl;

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
  
  if (argc != 7) {
    std::cout << "Usage: run_matrix_factorization <train-data-file> <probe-data-file> <num-factors> <num-epochs> <lambda> <learning-rate> \n";
    exit(1);
  }
  data_path = argv[1];
  probe_path = argv[2];
  num_factors = atoi(argv[3]); //Incremented indices of args since probe has been inserted after data_path
  num_epochs = atoi(argv[4]);
  lambda = atof(argv[5]);
  lrate = atof(argv[6]);

  std::cout << "Optimizing lambda (standard SGD) with the following parameters:\n"
      << "\tData file: " << data_path << std::endl
      << "\tNumber of factors: " << num_factors << std::endl
      << "\tNumber of epochs: " << num_epochs << std::endl
      << "\tInitial Lambda: " << lambda << std::endl
      << "\tLearning rate: " << lrate << std::endl
      << "\tNumber of folds: " << num_folds << std::endl;

  optimize_params(num_factors, data_path, probe_path, num_epochs, lambda, lrate);

  std::cout << "\nMatrix factorization finished!\n";

}