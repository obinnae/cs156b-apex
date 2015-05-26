#include <cmath>

#include "sgd.h"
#include "../DataAccessor/data_accessor.h"
#include "../baseline/baseline.h"
#include "update_parameters.h"

// DataAccessor d = new DataAccessor();


float * gradient(const float * const * u,
                 const float * const * v,
                 float ** w,
                 int ** r, 
                 entry_t e,
                 const DataAccessor * d,
                 Baseline *b,
                 int factor_length,
                 float lambda,
                 int num_neighbors,
                 bool isU,
                 float *factor_gradient) {

    /*
     * TODO: Change definition from Coordinate Gradient Descent
     * to Stochastic Gradient Descent. Will require signature change
     */

    int u_index = d->extract_user_id(e);
    int v_index = d->extract_movie_id(e);
    float rating = (float) d->extract_rating(e);

    float baseline_rating = (float) b->get_baseline(u_index, v_index);

    const float * const * factor;
    const float * const * non_factor;
    int factor_i, nfactor_i;

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

    error -= (1/ sqrt(num_neighbors)) *  0.001 *  weightSum(u_index, v_index, num_neighbors, w, r, d, b);

    //UPDATE Ws now

    // Make this k variable
    int * movie_indexes = r[v_index];

    updateWeights(u_index, num_neighbors, v_index, movie_indexes, w, d, b, 0.001 , lambda, error);

    /*for (int i = 0; i < factor_length; i++){
    	main_term[i] = non_factor[nfactor_i][i] * error;
    	regularization_term[i] = lambda * factor[factor_i][i] / d->get_num_entries();
    }*/
    
    for (int i = 0; i < factor_length; i++){
  		//factor_gradient[i] = regularization_term[i] - main_term[i];
        factor_gradient[i] = lambda * factor[factor_i][i] - non_factor[nfactor_i][i] * error;
    }
    
    return factor_gradient;
}


float * coordinateGradient(const float * const * u,
                           const float * const * v,
                           const int index,
                           const DataAccessor * d,
                           Baseline * b,
                           entry_t * user_movie_entries,
                           const int * non_factor_indexes,
                           const int num_non_factors,
                           const int factor_length,
                           float lambda,
                           bool isU,
                           int fold){


    // float baseline_rating = (float) b->get_baseline(u_index, v_index);

    // entry_t e = d->get_entry(index);
    // int u_index = d->extract_user_id(e);
    // int v_index = d->extract_movie_id(e);
    // float rating = (float) d->extract_rating(e);

    // int num_non_factors;

    const float * const * factor;
    const float * const * non_factor;
    float main_term[factor_length];
    std::fill( main_term, main_term + factor_length, 0 );
    float *factor_gradient = new float[factor_length];
    
    float regularization_term[factor_length];
    float baseline_rating;

    // int factor_i;
    // int nfactor_i;

    if (isU) {
        factor = u;
        non_factor = v;
    }
    else {
        factor = v;
        non_factor = u;
    }

    for (int j = 0; j < num_non_factors; j++){ //Need to find a way to only iterate through available vals
        /*
         * Loop that follows calculates the error arrising
         * from aproximating the rating using the factors
         */
        entry_t entry = user_movie_entries[j];
        int rating = d->extract_rating(entry);

        // ignore rating if it's in qual or in the validation set
        if (rating == 0 || d->get_validation_id(entry) == fold) continue;
        
        baseline_rating = b->get_baseline(d->extract_user_id(entry), d->extract_movie_id(entry));
        /*if (isU) {
            baseline_rating = (float) b->get_baseline(index, non_factor_indexes[j]);
        }
        else {
            baseline_rating = (float) b->get_baseline(non_factor_indexes[j], index);
        }*/

        float error = rating - baseline_rating;
        for (int i = 0; i < factor_length; i++){
            error -= factor[index][i] * non_factor[non_factor_indexes[j]][i];
        }


        for (int i = 0; i < factor_length; i++){
            main_term[i] += (non_factor[non_factor_indexes[j]][i] * error);
        }
    }
    for (int i = 0; i < factor_length; i++){
        regularization_term[i] = lambda * factor[index][i];
        factor_gradient[i] = regularization_term[i] - main_term[i];
    }

    return factor_gradient;
}