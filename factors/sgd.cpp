#include "sgd.h"
#include "../DataAccessor/data_accessor.h"
#include "../baseline/baseline.h"

// DataAccessor d = new DataAccessor();


float * gradient(const float ** u,
                 const float ** v, 
                 const int index,
                 const DataAccessor * d,
                 const int factor_length,
                 float lambda,
                 bool isU){

    /*
     * TODO: Change definition from Coordinate Gradient Descent
     * to Stochastic Gradient Descent. Will require signature change
     */

    Baseline b = new Baseline(d);

    int u_index = d.user_id_from_entry_index(index);
    int v_index = d.extract_movie_id(get_entry(index));
    float rating = (float) d.extract_rating(get_entry(index));
    //Would rather use a single entry_t object above and discard after this step

    float baseline_rating = (float) b.get_baseline(u_index, v_index);

    float ** factor;
    float ** non_factor;
    float dot_prods[factor_length];
    std::fill( dot_prods, dot_prods + factor_length, 0 );
    float factor_gradient [factor_length];

    if isU{
        factor = u;
        non_factor = v;
    }
    else{
        factor = v;
        non_factor = u;
    }
        /*
         * Loop that follows calculates the error arrising
         * from aproximating the rating using the factors
         */

    float error = rating - b.baseline; // Subtracting baseline from y here
    for (int i = 0; i < factor_length; i++){
        error -= u[u_index][i] + v[i][v_index];
    }

    if isU {
    	for (int i = 0; i < factor_length; i++){
    		dot_prods[i] += (non_factor[i][v_index] * error);
    	}
    }
    else {
    	for (int i = 0; i < factor_length; i++){
    		dot_prods[i] += (non_factor[u_index][i] * error);
    	}
    }
    if isU {
    	for (int i = 0; i < factor_length; i++){
    		factor_gradient[i] = (lambda * factor[u_index][i]) - dot_prods[i];
    	}
    }
    else {
    	for (int i = 0; i < factor_length; i++){
    		factor_gradient[i] = (lambda * factor[i][v_index]) - dot_prods[i];
    	}
    }
    return factor_gradient;
}

float * coordinateGradient(const float ** u,
						   const float ** v,
						   const int index,
						   const int factor_length,
						   const int non_factor_width,
						   float lambda,
						   bool isU){


    float ** factor;
    float ** non_factor;
    float dot_prods[factor_length];
    std::fill( dot_prods, dot_prods + factor_length, 0 );
    float factor_gradient [factor_length];

    if isU{
        factor = u;
        non_factor = v;
    }
    else{
        factor = v;
        non_factor = u;
    }

    for (int j = 0, j < non_factor_width, j++){ //Need to find a way to only iterate through available vals

        /*
         * Loop that follows calculates the error arrising
         * from aproximating the rating using the factors
         */

        float error = d.get_entry(index, j);
        for (int i = 0; i < factor_length; i++){
            error -= u[index][i] + v[i][j];
        }
        if isU {
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
    if isU {
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