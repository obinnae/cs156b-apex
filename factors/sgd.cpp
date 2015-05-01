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
						   const int factor_length,
						   const int non_factor_width,
						   float lambda,
						   bool isU){


    const float * const * factor;
    const float * const * non_factor;
    float dot_prods[factor_length];
    std::fill( dot_prods, dot_prods + factor_length, 0 );
    float *factor_gradient = new float[factor_length];

    if (isU) {
        factor = u;
        non_factor = v;
    }
    else {
        factor = v;
        non_factor = u;
    }

    for (int j = 0; j < non_factor_width; j++){ //Need to find a way to only iterate through available vals

        /*
         * Loop that follows calculates the error arrising
         * from aproximating the rating using the factors
         */

        float error = static_cast<float>(d->extract_rating(d->get_entry(index, j)));
        for (int i = 0; i < factor_length; i++){
            error -= u[index][i] + v[i][j];
        }
        if (isU) {
        	for (int i = 0; i < factor_length; i++){
        		dot_prods[i] += (non_factor[i][j] * error);
        	}
        }
        else {
        	for (int i = 0; i < factor_length; i++){
        		dot_prods[i] += (non_factor[j][i] * error);
        	}
        }
    }
    if (isU) {
    	for (int i = 0; i < factor_length; i++){
    		factor_gradient[i] = (lambda * factor[index][i]) - dot_prods[i];
    	}
    }

    else {
    	for (int i = 0; i < factor_length; i++){
    		factor_gradient[i] = (lambda * factor[index][i]) - dot_prods[i];
    	}
    }
    return factor_gradient;
}