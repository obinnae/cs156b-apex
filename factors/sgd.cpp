#include "sgd.h"
#include "../DataAccessor/data_accessor.h"
#include "../baseline/baseline.h"

// DataAccessor d = new DataAccessor();


float * gradient(const float * const * u,
                 const float * const * v, 
                 const int index,
                 const DataAccessor * d,
                 Baseline *b,
                 const int factor_length,
                 float lambda,
                 bool isU){

    /*
     * TODO: Change definition from Coordinate Gradient Descent
     * to Stochastic Gradient Descent. Will require signature change
     */

    entry_t e = d->get_entry(index);
    int u_index = d->extract_user_id(e);
    int v_index = d->extract_movie_id(e);
    float rating = (float) d->extract_rating(e);
    //Would rather use a single entry_t object above and discard after this step

    float baseline_rating = (float) b->get_baseline(u_index, v_index);

    const float * const * factor;
    const float * const * non_factor;
    int factor_i, nfactor_i;
    float main_term[factor_length];
    float regularization_term[factor_length];
    float *factor_gradient = new float[factor_length];

    if (isU) {
        factor = u;
        non_factor = v;
        factor_i = u_index;
        nfactor_i = v_index;
    }
    else {
        factor = v;
        non_factor = u;
        factor_i = v_index;
        nfactor_i = u_index;
    }
        /*
         * Loop that follows calculates the error arising
         * from aproximating the rating using the factors
         */

    float error = rating - baseline_rating; // Subtracting baseline from y here
    for (int i = 0; i < factor_length; i++){
        error -= u[u_index][i] * v[v_index][i];
    }

    for (int i = 0; i < factor_length; i++){
    	main_term[i] = non_factor[nfactor_i][i] * error;
    	regularization_term[i] = lambda * factor[factor_i][i] / d->get_num_entries();
    }
    
    for (int i = 0; i < factor_length; i++){
  		factor_gradient[i] = regularization_term[i] - main_term[i];
    }
    
    return factor_gradient;
}

// NEEDS SIGNIFICANT UPDATING.
float * coordinateGradient(const float * const * u,
                           const float * const * v,
                           const int index,
                           const DataAccessor * d,
                           Baseline * b,
                           const int factor_length,
                           float lambda,
                           bool isU){


    // float baseline_rating = (float) b->get_baseline(u_index, v_index);

    entry_t e = d->get_entry(index);
    int u_index = d->extract_user_id(e);
    int v_index = d->extract_movie_id(e);
    // float rating = (float) d->extract_rating(e);

    entry_t * user_movie_entries; //using this as general case for getting all movies associated with user
                                  // or all users associated with a specific movie
    int num_entries;

    const float * const * factor;
    const float * const * non_factor;
    float main_term[factor_length];
    std::fill( main_term, main_term + factor_length, 0 );
    float *factor_gradient = new float[factor_length];

    
    float regularization_term[factor_length];
    float baseline_rating;

    int * non_factor_indexes;
    int factor_i;
    int nfactor_i;

    if (isU) {
        factor = u;
        non_factor = v;
        num_entries = d->get_user_entries(u_index, user_movie_entries);
        factor_i = u_index;

        for (int k = 0; k < num_entries; k++){
            non_factor_indexes[k] = d->extract_movie_id(user_movie_entries [k]);
        }
    }
    else {
        factor = v;
        non_factor = u;
        num_entries = d->get_movie_entries(v_index, user_movie_entries);
        factor_i = v_index;

        for (int k = 0; k < num_entries; k++){
            non_factor_indexes[k] = d->extract_user_id(user_movie_entries [k]);
        }
    }

    for (int j = 0; j < num_entries; j++){ //Need to find a way to only iterate through available vals

        /*
         * Loop that follows calculates the error arrising
         * from aproximating the rating using the factors
         */

        float rating = (float) d->extract_rating(user_movie_entries[j]);
        if (isU) {
            baseline_rating = (float) b->get_baseline(factor_i, non_factor_indexes[j]);
        }
        else {
            baseline_rating = (float) b->get_baseline(non_factor_indexes[j], factor_i);
        }

        float error = rating - baseline_rating;
        for (int i = 0; i < factor_length; i++){
            error -= factor[factor_i][i] + non_factor[non_factor_indexes[j]][i];
        }
        for (int i = 0; i < factor_length; i++){
            main_term[i] += (non_factor[non_factor_indexes[j]][i] * error);
            regularization_term[i] = lambda * factor[factor_i][i] / d->get_num_entries();
        }
    }
    for (int i = 0; i < factor_length; i++){
        factor_gradient[i] = regularization_term[i] - main_term[i];
    }
    return factor_gradient;
}