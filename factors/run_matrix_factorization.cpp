/*
 * Nancy Wen
 * April 21, 2015
 *
 * This program uses the gradient calculations to update the latent factors.
 */
#include <iostream>
#include <cstdlib>

#include "sgd.h"
#include "../DataAccessor/data_accessor.h"

using namespace std;

#define NUM_MOVIES 17770
#define NUM_USERS 458293

void run_matrix_factorization(int factors, char * data_path, int iterations, float lambda, float lrate)
{
	//declare the number of iterations of SGD you want to do
	int iterations = 10;
	float lambda = 0.1;

	DataAccessor d = new DataAccessor();
	d.load_data(data_path);

	//declare the latent factors matrices
	float ** U = new float *[NUM_USERS];
	for(int i = 0; i < NUM_USERS; i++)
	{
		U[i] = new float[factors];
	}

	float ** V = new float *[NUM_MOVIES];
	for(int i = 0; i < NUM_MOVIES; i++)
	{
		V[i] = new float[factors];
	}

	initialize_latent_factors(factors, U, V);

	update_latent_factors(U, V, &d, iterations, lambda, lrate)

}

void initialize_latent_factors(int factors, float ** U, float ** V) {
	//initialize matrix elements to random numbers between 0 and 1
	for(int i = 0; i < NUM_USERS; i++)
		for (int j = 0; j < factors; j++)
			U[i][j] = ((float) rand() / (RAND_MAX));

	for(int i = 0; i < NUM_MOVIES; i++)
		for (int j = 0; j < factors; j++)
			V[i][j] = ((float) rand() / (RAND_MAX));

/*	//prints out the matrix
	for(int i = 0; i < NUM_USERS; i++)
		for (int j = 0; j < factors; j++)
			cout << "[" << i << "," << j << "]" << U[i][j];
		cout << endl;

	for(int i = 0; i < NUM_MOVIES; i++)
		for (int j = 0; j < factors; j++)
			cout << "[" << i << "," << j << "]" << V[i][j];
		cout << endl;
*/

}

void update_latent_factors(float ** U, float ** V, DataAccessor * d, int iterations, float lambda, float lrate) {
	bool isU;
	int index;
	float step;

	//Loop for the chosen number of iterations
	for(int k = 0; k < iterations; k++) {

		//randomly select U or V
		isU = (rand() % 2) == 1;

		//randomly select one index i of matrix
		if(isU)
		{
			index = rand() % NUM_USERS;
		}
		else
		{
			index = rand() % NUM_MOVIES;
		}

		// calculate a gradient step (using Obi's code in sgd.cpp)
		float * step = gradient(U, V, d, index, factors,lambda, isU);

		// take a gradient step
		if(isU)
		{
			for(int i = 0; i < factors; i++)
				U[index][i] = U[index][i] - lrate * step[i];
		}
		else
		{
			for(int i = 0; i < factors; i++)
				V[index][i] = V[index][i] - lrate * step[i];
		}

	}
}

int main(int argc, char *argv[]) {
  char *data_path;
  int num_factors, num_iters;
  float lambda, lrate;

  if (argc != 6) {
    std::cout << "Usage: run_matrix_factorization <data-file> <num-factors> <num-iters> <lambda> <learning-rate>\n";
    exit(1);
  }
  data_path = argv[1];
  num_factors = atoi(argv[2]);
  num_iters = atoi(argv[3]);
  lambda = atof(argv[4]);
  lrate = atof(argv[5]);

  std::cout << "Running matrix factorization with the following parameters:\n"
      << "\tData file: " << data_path
      << "\tNumber of factors: " << num_factors
      << "\tNumber of iterations: " << num_iters
      << "\tLambda: " << lambda
      << "\tLearning rate: " << lrate << std::endl; 

  run_matrix_factorization(num_factors, data_path, num_iters, lambda, lrate);

  std::cout << "\nMatrix factorization finished!\n";
}
