
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <cmath>

#define MAX_MOVIES 17770
#define MAX_USERS 458293

// Using batches can save ~3-4seconds per epoch
#define ENTRY_BATCH_SIZE 1048576

void initialize_latent_factors(int factors, float ** U, float ** V, int num_users, int num_movies);
void update_latent_factors(float ** U, float ** V, DataAccessor * d, Baseline *b, int factors, int epochs, float lambda, float lrate, int fold=-1);
float calc_in_sample_error(float **U, float **V, int num_factors, DataAccessor *d, Baseline *b, int fold=-1);
float calc_out_sample_error(float **U, float **V, int num_factors, DataAccessor *p, Baseline *b_p, int fold=-1);
void single_fold_factorization(float **U, float **V, int factors, int epochs, float lambda, float lrate, DataAccessor *d, DataAccessor * p, Baseline *b, Baseline * b_p);
void k_fold_factorization(float **U, float **V, int factors, int epochs, float lambda, float lrate, int folds, DataAccessor *d, Baseline *b);