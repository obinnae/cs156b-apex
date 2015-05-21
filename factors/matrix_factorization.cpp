
#include "matrix_factorization.h"

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

  entry_t *entries = new entry_t[ENTRY_BATCH_SIZE];

	float *u_step = new float[factors];
  float *v_step = new float[factors];

  double avg_change = 0; // for printing out status updates
  int iters_since_update = 0;

  time_t t1, t2; // time each epoch for informational purposes

  //Loop for the chosen number of epochs; takes 47-48 seconds @ 10 factors on train.cdta
  t1 = time(NULL);
  for (int batch_start = 0; batch_start < d->get_num_entries(); batch_start = batch_start + ENTRY_BATCH_SIZE) {
    int num_entries = d->get_entry_batch(batch_start, ENTRY_BATCH_SIZE, entries);
    for (int k = 0; k < num_entries; k++) {
    //for (int k = 0; k < d->get_num_entries(); k++) {
      // Select entry index
      //index = k;

      // Don't need to check if rating in qual because qual is a separate data file
      // Extract entry information
      //e = d->get_entry(index);
      e = entries[k];
      user_id = d->extract_user_id(e);
      movie_id = d->extract_movie_id(e);

      // Calculate gradient
      gradient(U, V, e, d, b, factors, lambda, u_step, v_step); // takes ~27 seconds per epoch @ 10 factors on train.cdta

  		// take a gradient step
  	  for(int i = 0; i < factors; i++) { // loop takes 13-14 seconds per epoch @ 10 factors on train.cdta
  			U[user_id][i] = U[user_id][i] - lrate * u_step[i];
        V[movie_id][i] = V[movie_id][i] - lrate * v_step[i];
  		}
    }
    if ((batch_start & 0xFF) == 0) {
      for (int i = 0; i < factors; avg_change += abs(u_step[i]) + abs(v_step[i]), i++);
      iters_since_update++;
    }

    if ((batch_start & 0x1FFFFF) == 0) {
      std::cout << "Iteration " << (batch_start + 1)
            << ": Average |gradient| since last update: " << (avg_change/iters_since_update/factors/2) << std::endl;
      avg_change = 0;
      iters_since_update = 0;
    }
  }
  t2 = time(NULL);

  delete[] u_step;
  delete[] v_step;
  delete[] entries;

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
  float old_error = 100; // a big number
  float new_error;

  initialize_latent_factors(factors, U, V, d->get_num_users(), d->get_num_movies());

  for (int epoch = 0; epoch < epochs; epoch++) {

    update_latent_factors(U, V, d, b,factors, 1, lambda, lrate);
    calc_in_sample_error(U, V, factors, d, b);
    new_error = calc_out_sample_error(U, V, factors, p, b_p);

    std::cout << "*** EPOCH " << epoch << " COMPLETE! ***\n\n";
    
    if (new_error > old_error) { // overfitting has occurred
      std::cout << "E_out has begun to increase! Halting matrix factorization to prevent overfitting...\n";
      break;
    }
    old_error = new_error;
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