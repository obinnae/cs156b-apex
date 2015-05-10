#include "sgd.h"
#include "../DataAccessor/data_accessor.h"
#include "../baseline/baseline.h"

// DataAccessor d = new DataAccessor();

float optimal_stepsize(const float * const * u,
                        const float * const * v,
                        entry_t e,
                        const DataAccessor *d,
                        Baseline *b,
                        const int factor_length,
                        float lambda,
                        bool isU,
                        float *steps) {
    // Optimal step size is determined by:
    //   k*gradient
    // where k is
    //   (D*D) / (D*gradient)
    // with D = (second derivative matrix)(gradient)
    // and (*) is the dot product

    gradient(u, v, e, d, b, factor_length, lambda, isU, steps);

    float k;
    float k_numer = 0, k_denom = 0;

    float grad_of_grad_matrix[MAX_FACTORS][MAX_FACTORS];
    float D[MAX_FACTORS];

    // Calculate second derivatives and put in matrix
    const float * row = isU ? v[d->extract_movie_id(e)] : u[d->extract_user_id(e)];
    
    // Calculate D
    // It's easier to calculate D using the (equivalent) formula
    //   D = (row * gradient)(row) + lambda/num_entries*gradient
    // where (*) is the dot product
    float row_times_gradient = 0;
    for (int i = 0; i < factor_length; i++)
        row_times_gradient += row[i] * steps[i];

    for (int i = 0; i < factor_length; i++)
        D[i] = row_times_gradient * row[i] + lambda / d->get_num_entries() * steps[i];

    // Calculate numerator and denominator of k
    for (int i = 0; i < factor_length; i++) {
        k_numer = k_numer + D[i] * D[i];
        k_denom = k_denom + D[i] * steps[i];
    }

    // Calculate factor to penalize big changes
/*    float regularizer = 0;
    for (int i = 0; i < factor_length; i++)
        regularizer += steps[i]*steps[i];*/

    k = k_numer / k_denom;

/*
    if (d->extract_entry_index(e) % 1000000 == 0) {
        std::cout << "k = " << k_numer << " / (" << k_denom << " + " << regularizer << ") = " << k << std::endl;
    }*/

    // Modify steps vector by multiplying by k
    for (int i = 0; i < factor_length; i++)
        steps[i] = steps[i] * k;

    return k;
}

float * gradient(const float * const * u,
                 const float * const * v, 
                 entry_t e,
                 const DataAccessor * d,
                 Baseline *b,
                 int factor_length,
                 float lambda,
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

    for (int i = 0; i < factor_length; i++){
        factor_gradient[i] = lambda * factor[factor_i][i] / d->get_num_entries() - non_factor[nfactor_i][i] * error;
    }
    
    return factor_gradient;
}

float grad_of_grad(const float * row,
                   int index1,
                   int index2,
                   float regularization_coef // regularization_coef = lambda / num_entries)
                   ) {
    // Calculates the second derivative of the error with respect to two specified factors
    // row is the row of the unchanging matrix associated with the entry being optimized for
    // index1 and index2 are the indices of the two factors to which the 2nd deriv is taken.
  
    if (index1 == index2)
        return regularization_coef + row[index1] * row[index1];
    else
        return row[index1] * row[index2];

  

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