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
using namespace std;

#define MAX_MOVIES 17770
#define MAX_USERS 458293


void initialize_latent_factors(int factors, float ** U, float ** V, int num_users, int num_movies) {
	//initialize matrix elements to random numbers between -.001 to .001
	for(int i = 0; i < num_users; i++)
		for (int j = 0; j < factors; j++)
			U[i][j] = ((float) rand() / (RAND_MAX) / 500.0) - 0.001;

	for(int i = 0; i < num_movies; i++)
		for (int j = 0; j < factors; j++)
			V[i][j] = ((float) rand() / (RAND_MAX) / 500.0) - 0.001;

}
void update_latent_factors(float ** U, float ** V, DataAccessor * d, Baseline *b, int factors, int epochs, float lambda, float lrate, int fold=-1){
  int index;

  entry_t e;
  int movie_id, user_id, rating;

	float *u_step = new float[factors];
  float *v_step = new float[factors];

  double avg_change = 0; // for printing out status updates
  int iters_since_update = 0;

  time_t t1, t2; // time each epoch for informational purposes

  //Loop for the chosen number of epochs
  t1 = time(NULL);
  for (int k = 0; k < d->get_num_entries(); k++) {
    // Select entry index
    index = k;

    // Don't need to check if rating in qual because qual is a separate data file
    // Extract entry information
    e = d->get_entry(index);
    rating = d->extract_rating(e);
    user_id = d->extract_user_id(e);
    movie_id = d->extract_movie_id(e);

    // Calculate gradient
    gradient(U, V, e, d, b, factors, lambda, u_step, v_step);

		// take a gradient step
	  for(int i = 0; i < factors; i++) {
			U[user_id][i] = U[user_id][i] - lrate * u_step[i];
      V[movie_id][i] = V[movie_id][i] - lrate * v_step[i];
		}

    if ((k & 0xF) == 0) {
      for (int i = 0; i < factors; avg_change += abs(u_step[i]) + abs(v_step[i]), i++);
      iters_since_update++;
    }

    if (k % 0x1FFFFF == 0x1FFFFF-1) {
	  	std::cout << "Iteration " << (k+1)
	  				<< ": Average |gradient| since last update: " << (avg_change/iters_since_update/factors/2) << std::endl;
      avg_change = 0;
      iters_since_update = 0;
	  }
  }
  t2 = time(NULL);

  std::cout << "Epoch time: " << difftime(t2, t1) << " sec\n";
}

float calc_in_sample_error(float **U, float **V, int num_factors, DataAccessor *d, Baseline *b, int fold=-1){
    /* TODO: update calc_in_sample_error to support k-fold. Should calculate error based on just that fold, or entire data set
     * if fold = -1 */
  float error = 0;
  int num_test_pts = 0;

  std::cout << "Calculating E_in...\n";

  entry_t e;
  int user_id, movie_id, rating;
  for (int i = 0; i < d->get_num_entries(); i++) {

    entry_t e = d->get_entry(i);
    rating = d->extract_rating(e);

    // If not a qual entry then calculate and accumulate error
    if (rating != 0) {
      user_id = d->extract_user_id(e);
      movie_id = d->extract_movie_id(e);

      // Increment error
      float rating_error = 0;
      for (int j = 0; j < num_factors; j++) {
        rating_error += U[user_id][j] * V[movie_id][j];
      }
      rating_error -= rating - b->get_baseline(user_id, movie_id);
      error += rating_error * rating_error;

      // Increment number of test points
      num_test_pts++;
    }

    if (i % 10000000 == 9999999)
      std::cout << (float)i/d->get_num_entries()*100 << "%: " << sqrt(error/num_test_pts) << "\n";

  }
  std::cout << "E_in = " << sqrt(error / num_test_pts) << " over " << num_test_pts << " test points.\n";

  return sqrt(error / num_test_pts);
}

float calc_out_sample_error(float **U, float **V, int num_factors, DataAccessor *p, Baseline *b_p, int fold=-1){
    /*
     * Now passing in reference to DataAccessor and Baseline objects for probe dataset
     * Uses these to get the actual ratings and baselines for predictions of probe dataset
     * Changed the names in the signature just to make it clearer which dataset we are operating on.
     */

    /* TODO: consider merging with calc_in_sample error, so error checking just does one pass */
    float error = 0;
    int num_test_pts = 0;

    std::cout << "Calculating E_out...\n";

    entry_t e;
    int user_id, movie_id, rating;

    for (int i = 0; i < p->get_num_entries(); i++) {
      // Checking for qual is no longer necessary because that's kept separate (as we should have done from the beginning...) 
      entry_t e = p->get_entry(i);
      rating = p->extract_rating(e);
        
      user_id = p->extract_user_id(e);
      movie_id = p->extract_movie_id(e);
      
      // Increment error
      float rating_error = 0;
      for (int j = 0; j < num_factors; j++) {
          rating_error += U[user_id][j] * V[movie_id][j];
      }
      rating_error -= rating - b_p->get_baseline(user_id, movie_id);
      error += rating_error * rating_error;

      // Increment number of test points
      num_test_pts++;
        

        if (i % 10000000 == 9999999)
          std::cout << (float)i/p->get_num_entries()*100 << "%: " << sqrt(error/num_test_pts) << "\n";

    }
    std::cout << "E_out: " << sqrt(error / num_test_pts) << " over " << num_test_pts << " test points.\n";

    return sqrt(error / num_test_pts);
}

void single_fold_factorization(float **U, float **V, int factors, int epochs, float lambda, float lrate, DataAccessor *d, DataAccessor * p, Baseline *b, Baseline * b_p) {

  float *errors = new float[epochs];
  for (int epoch = 0; epoch < epochs; epoch++) {
    errors[epoch] = 0;
  } // Setting up single fold factorization to now calculate out of sample error

  initialize_latent_factors(factors, U, V, d->get_num_users(), d->get_num_movies());

  for (int epoch = 0; epoch < epochs; epoch++) {

    update_latent_factors(U, V, d, b,factors, 1, lambda, lrate);
    calc_in_sample_error(U, V, factors, d, b);
    errors[epoch] += calc_out_sample_error(U, V, factors, p, b_p); //Calculating out of sample error with probes DataAccessor and Baselines

    std::cout << "*** EPOCH " << epoch << " COMPLETE! ***\n\n";
  }
}


void k_fold_factorization(float **U, float **V, int factors, int epochs, float lambda, float lrate, int folds, DataAccessor *d, Baseline *b) {

  /* DOES NOT WORK RIGHT NOW BECAUSE update_latent_factors() IGNORES THE FOLD ARGUMENT (FOR SPEED) */

  /* sum of errors at each epoch; init to all 0 */
  float *errors = new float[epochs];
  for (int epoch = 0; epoch < epochs; epoch++) {
    errors[epoch] = 0;
  }

  // Set number of validation sets in DataAccessor so validation IDs are sensible
  d->set_num_validation_sets(folds);

  // do matrix factorization <folds> times
  for (int fold = 0; fold < folds; fold++){
    initialize_latent_factors(factors, U, V, d->get_num_users(), d->get_num_movies());

    for (int epoch = 0; epoch < epochs; epoch++){
      update_latent_factors(U, V, d, b, factors, 1, lambda, lrate, fold);
      errors[epoch] += calc_out_sample_error(U, V, factors, d, b, fold);

      std::cout << "*** EPOCH " << epoch << " COMPLETE ***\n\n";
    }
  }

  /* Find epoch with least total error.
     This is the same as the epoch with the least average error */
  int bestEpoch = 0;
  for (int epoch = 0; epoch < epochs; epoch++){
      if (errors[bestEpoch] > errors[epoch]){
          bestEpoch = epoch;
      }
  }

  // run factorization for best # of epochs
  std::cout << "Best # epochs is" << (bestEpoch+1) << " epochs. Running factorization on full data set...\n";
  //single_fold_factorization(U, V, factors, bestEpoch+1, lambda, lrate, d, b);
  // Commented out because of signature mismatch and because right now the probe dataset is required

}


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

  //declare and allocate memory for the latent factors matrices
  float ** U = new float *[num_users];
  for(int i = 0; i < num_users; i++) {
    U[i] = new float[factors];
  }

  float ** V = new float *[num_movies];
  for(int i = 0; i < num_movies; i++) {
    V[i] = new float[factors];
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
  for (int i = 0; i < num_users; i++)
    delete[] U[i];
  for (int i = 0; i < num_movies; i++)
    delete[] V[i];
  delete[] U;
  delete[] V;

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
