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
void update_latent_factors(float ** U, float ** V, DataAccessor * d, Baseline *b, int factors, int epochs, float lambda, float lrate) {
	bool isU;
	int index;
	int movie_id, user_id;
	float *step;
	entry_t e;
	
	//Loop for the chosen number of epochs
	for (int epoch = 0; epoch < epochs; epoch++) {
	  for (int k = 0; k < d->get_num_entries(); k++) {

  		//randomly select U or V
	  	isU = (rand() % 2) == 1;
 
  		//randomly select one index i of matrix
	  	do {
    		index = rand() % d->get_num_entries();
  	  	e = d->get_entry(index);
  	  } while (d->extract_rating(e) == 0);

      user_id = d->extract_user_id(e);
      movie_id = d->extract_movie_id(e);

//      std::cout << "Training on rating " << index << ", (user_id, movie_id) = (" << user_id << "," << movie_id << ")\n";

  		// calculate a gradient step (using Obi's code in sgd.cpp)
	  	step = gradient(U, V, index, d, b, factors,lambda, isU);

  		// take a gradient step
	  	if(isU)
	    {
		    for(int i = 0; i < factors; i++)
  				U[user_id][i] = U[user_id][i] - lrate * step[i];
  		}
  		else
	  	{
		  	for(int i = 0; i < factors; i++)
			  	V[movie_id][i] = V[movie_id][i] - lrate * step[i];
		  }
		
	  	delete[] step;

      if (k % 500000 == 0) {
	  		double avg_change = 0;
	  		for (int i = 0; i < factors; i++, avg_change += abs(lrate*step[i]/factors)) {}
		  	std::cout << "Iteration " << k
		  				<< ": Average change to " << (isU?"user ":"movie ")
		  				<< (isU?d->extract_user_id(e):d->extract_movie_id(e))
		  				<< " factors is " << avg_change << std::endl;
		  	//std::cout << avg_change << " ";
		  }

	  }
	  std::cout << "*** EPOCH " << epoch << " COMPLETE! ***\n";
	}
	std::cout << std::endl;
}

float calc_in_sample_error(float **U, float **V, int num_factors, DataAccessor *d, Baseline *b) {
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
    
    if (i % 10000000 == 0)
      std::cout << (float)i/d->get_num_entries()*100 << "%: " << (error/num_test_pts) << "\n";
    
  }
  
  return sqrt(error / num_test_pts);
}

void run_matrix_factorization(int factors, char * data_path, int epochs, float lambda, float lrate, char * qualPath, char * outputPath)
{
	// declare the number of epochs of SGD you want to do
	// # epochs = # iters * # factors

	DataAccessor d;
	d.load_data(data_path);
	
	Baseline b(&d);
	
	int num_users = d.get_num_users();
	int num_movies = d.get_num_movies();

	//declare the latent factors matrices
	float ** U = new float *[num_users];
	for(int i = 0; i < num_users; i++)
	{
		U[i] = new float[factors];
	}

	float ** V = new float *[num_movies];
	for(int i = 0; i < num_movies; i++)
	{
		V[i] = new float[factors];
	}
	
	srand(time(NULL));

	initialize_latent_factors(factors, U, V, num_users, num_movies);

	update_latent_factors(U, V, &d, &b, factors, epochs, lambda, lrate);
/*
  std::cout << "U = [";
  for (int i = 0; i < num_users; i++) {
    for (int j = 0; j < factors; j++) {
      std::cout << U[i][j] << " ";
    }
    std::cout << ";\n";
  }
  std::cout << "]\nV=[";
  for (int i = 0; i < num_movies; i++) {
    for (int j = 0; j < factors; j++) {
      std::cout << V[i][j] << " ";
    }
    std::cout << ";\n";
  }
  std::cout << "]\n";*/
    
  // Calculate in-sample error
  float error = calc_in_sample_error(U, V, factors, &d, &b);
  
  //create qual submission
  runMatrixFactorization(U, V, factors, qualPath, outputPath, &b);

  std::cout << "RMSE (in sample): " << error << std::endl;

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
