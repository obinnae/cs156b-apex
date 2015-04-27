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
    int u_index = b.user_id_from_entry_index(index);
    int v_index = b.movie_id_from_entry_val(index);

    float baseline_rating = b.get_baseline()
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

        float error = d.get_entry(index, j) - b.get_baseline(u_index, v_index); // Subtracting baseline from y here
        for (int i = 0; i < factor_length; i++){
            error -= u[index][i] + v[i][j];
        }
        for (int i = 0; i < factor_length; i++){
            dot_prods[i] += (non_factor[i][j] * error);
        }
    }
    for (int i = 0; i < factor_length; i++){
        factor_gradient[i] = (lambda * factor[index][i]) - dot_prods[i];
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