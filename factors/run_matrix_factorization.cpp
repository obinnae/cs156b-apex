/*
 * Nancy Wen
 * April 21, 2015
 *
 * This program uses the gradient calculations to update the latent factors.
 */
#include <iostream>
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
	//initialize matrix elements to random numbers between 0 and 1
	for(int i = 0; i < num_users; i++)
		for (int j = 0; j < factors; j++)
			U[i][j] = ((float) rand() / (RAND_MAX));

	for(int i = 0; i < num_movies; i++)
		for (int j = 0; j < factors; j++)
			V[i][j] = ((float) rand() / (RAND_MAX));

/*	//prints out the matrix
	for(int i = 0; i < num_users; i++)
		for (int j = 0; j < factors; j++)
			cout << "[" << i << "," << j << "]" << U[i][j];
		cout << endl;

	for(int i = 0; i < num_movies; i++)
		for (int j = 0; j < factors; j++)
			cout << "[" << i << "," << j << "]" << V[i][j];
		cout << endl;
*/

}
void update_latent_factors(float ** U, float ** V, DataAccessor * d, Baseline *b, int factors, int epochs, float lambda, float lrate, int fold=-1){
   
    /* TODO update_latent_factors needs to support k-folds. Should run on entire data set if fold is -1 */
	bool isU;
	int index;
	int movie_id, user_id;
	float *step;
	entry_t e; // Might not need anymore
  entry_t * user_movie_entries = new entry_t[MAX_ENTRIES_PER_MOVIE];
  int * non_factor_indexes = new int[MAX_ENTRIES_PER_MOVIE];
  int num_non_factors;
	
  double avg_change = 0; // for printing out status updates

	//Loop for the chosen number of epochs
	for (int epoch = 0; epoch < epochs; epoch++) {
    //for (int k = 0; k < d->get_num_users() + d->get_num_movies(); k++) {
    for (int k = 0; k < d->get_num_users() + d->get_num_movies(); k++) {

  		//randomly select U or V
      int num_non_factors;
	  	//isU = (rand() % 2) == 1;
      
      //std::cout << "Selecting a " << (isU?"user":"movie") << " to modify...\n";
      if (k < d->get_num_users()) {
        //index = rand() % d->get_num_users();
        //std::cout << "  Selected user " << index << std::endl;
        isU = true;
        index = k;
        num_non_factors = d->get_user_entries(index, user_movie_entries);
        //std::cout << "  Retrieved " << num_non_factors << " ratings from user " << index << std::endl;
        for (int i=0; i < num_non_factors; i++) {
          non_factor_indexes[i]=d->extract_movie_id(user_movie_entries[i]);
        }
      } else {
        isU = false;
        index = k - d->get_num_users();
        //index = rand() % d->get_num_movies();
        num_non_factors = d->get_movie_entries(index, user_movie_entries);
        for (int i=0; i < num_non_factors; i++) {
          non_factor_indexes[i]=d->extract_user_id(user_movie_entries[i]);
        }
      }
      //std::cout << "Updating " << (isU?"user ":"movie ") << index << std::endl;
  		//randomly select one index i of matrix
	  	// do {
    // 		index = rand() % d->get_num_entries();
  	 //  	e = d->get_entry(index);
  	 //  } while (d->extract_rating(e) == 0); //Why this check?


      //user_id = d->extract_user_id(e);
      //movie_id = d->extract_movie_id(e); // <== REVISIT and see if used anywhere besides sgd

//      std::cout << "Training on rating " << index << ", (user_id, movie_id) = (" << user_id << "," << movie_id << ")\n";

  		// calculate a gradient step (using Obi's code in sgd.cpp)
      step = coordinateGradient(U, V, index, d, b, user_movie_entries, non_factor_indexes, num_non_factors, factors,lambda, isU, fold);

  		// take a gradient step
	  	if(isU)
	    {
		    for(int i = 0; i < factors; i++)
  				U[index][i] = U[index][i] - lrate * step[i] / num_non_factors;
  		}
  		else
	  	{
		  	for(int i = 0; i < factors; i++)
			  	V[index][i] = V[index][i] - lrate * step[i] / num_non_factors;
		  }

      for (int i = 0; i < factors; i++, avg_change += abs(step[i]) / num_non_factors) {}

      if (k % 0x1FFF == 0x1FFF-1) {
		  	std::cout << "Iteration " << (k+1)
		  				<< ": Average |gradient| over last 8191 iterations: " << (avg_change/0x1FFF/factors) << std::endl;
        avg_change = 0;
		  	//std::cout << avg_change << " ";
		  }

      delete[] step;
      //delete[] non_factor_indexes;

	  }
	}
	std::cout << std::endl;

  delete[] user_movie_entries;
  delete[] non_factor_indexes;
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

      // If its the fold we're leaving out, skip it
      if (d->get_validation_id(i) == fold){
          continue;
      }
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
    
    if (i % 10000000 == 0)
      std::cout << (float)i/d->get_num_entries()*100 << "%: " << (error/num_test_pts) << "\n";
    
  }
  std::cout << "100%: " << sqrt(error / num_test_pts) << std::endl;
  
  return sqrt(error / num_test_pts);
}

float calc_out_sample_error(float **U, float **V, int num_factors, DataAccessor *d, Baseline *b, int fold=-1){
    /* calculates out of sample erorr (error of entries that are equal to fold) */

    /* TODO: consider merging with calc_in_sample error, so error checking just does one pass */
    float error = 0;
    int num_test_pts = 0;

    std::cout << "Calculating E_out...\n";

    entry_t e;
    int user_id, movie_id, rating;

    for (int i = 0; i < d->get_num_entries(); i++) {

        //if it's not the fold, then it's in-sample, so don't check
        if (d->get_validation_id(i) != fold){
            continue;
        }
        
        entry_t e = d->get_entry(i);
        
        
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
        
        if (i % 10000000 == 0)
            std::cout << (float)i/d->get_num_entries()*100 << "%: " << (error/num_test_pts) << "\n";
        
    }
    std::cout << "100%: " << sqrt(error / num_test_pts) << std::endl;
    
    return sqrt(error / num_test_pts);
}


void run_matrix_factorization(int factors, char * data_path, int epochs, float lambda, float lrate, char * qualPath, char * outputPath, int folds=1)
{
	// declare the number of epochs of SGD you want to do
	// # epochs = # iters * # factors

	DataAccessor d;
	d.load_data(data_path);
  if (folds != -1) d.set_num_validation_sets(folds);
	
	Baseline b(&d); // Baseline instantiation
	
	int num_users = d.get_num_users();
	int num_movies = d.get_num_movies();

  int fold;
  int epoch; // current epoch

  /* sum of errors at each epoch; init to all 0 */
  float *errors = new float[epochs];
  for (epoch = 0; epoch < epochs; epoch++) {
    errors[epoch] = 0;
  }

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

  for (fold = 0; fold < folds; fold++){

    initialize_latent_factors(factors, U, V, num_users, num_movies);
    
    for (epoch = 0; epoch < epochs; epoch++){
      update_latent_factors(U, V, &d, &b, factors, 1, lambda, lrate, fold);
      errors[epoch] += calc_out_sample_error(U, V, factors, &d, &b, fold);
      std::cout << "*** EPOCH " << epoch << " COMPLETE! ***\n";
      
      entry_t *e = new entry_t[MAX_ENTRIES_PER_USER];

      // Calculate second derivative to get better learning rate

      delete[] e;


    }
  }
    
  // Calculate in-sample error

  //float error = calc_in_sample_error(U, V, factors, &d, &b);

  //std::cout << "RMSE (in sample): " << error << std::endl;
        
  /* calculate average error across folds for each epoch */
  int bestEpoch = 0;
  for (epoch = 0; epoch < epochs; epoch++){
      errors[epoch] /= folds;
      if (errors[bestEpoch] > errors[epoch]){
          bestEpoch = epoch;
      }
  }

  //create qual submission with best epoch
  initialize_latent_factors(factors, U, V, num_users, num_movies);
  update_latent_factors(U, V, &d, &b, factors, bestEpoch, lambda, lrate);
  runMatrixFactorization(U, V, factors, qualPath, outputPath, &b);

}



int main(int argc, char *argv[]) {
    char *data_path, *qualPath, *outputPath;
  int num_factors;
  int num_epochs;
  float lambda, lrate;
  
  if (argc != 8){
      std::cout << "Usage: run_matrix_factorization <data-file> <num-factors> <num-epochs> <lambda> <learning-rate> <qual_path> <output-file-path>\n";
    exit(1);
  }
  data_path = argv[1];
  num_factors = atoi(argv[2]);
  num_epochs = atoi(argv[3]);
  lambda = atof(argv[4]);
  lrate = atof(argv[5]);
  
  qualPath = argv[6];
  outputPath = argv[7];


  std::cout << "Running matrix factorization with the following parameters:\n"
      << "\tData file: " << data_path << std::endl
      << "\tNumber of factors: " << num_factors << std::endl
      << "\tNumber of epochs: " << num_epochs << std::endl
      << "\tLambda: " << lambda << std::endl
      << "\tLearning rate: " << lrate << std::endl; 

  run_matrix_factorization(num_factors, data_path, num_epochs, lambda, lrate, qualPath, outputPath);

  std::cout << "\nMatrix factorization finished!\n";
  
}
/* NOTES

Line 163 ...  Baseline instantiation? Where do we create instance of it. Same with DataAccesor

*/
