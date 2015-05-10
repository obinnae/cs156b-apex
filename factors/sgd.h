#ifndef SGD_H_INCLUDED
#define SGD_H_INCLUDED	

#include <cstring>
#include "../baseline/baseline.h"
#include "../DataAccessor/data_accessor.h"

#define MAX_FACTORS 200

// Stochastic gradient descent functions
float optimal_stepsize(const float * const * u,
                        const float * const * v,
                        entry_t e,
                        const DataAccessor *d,
                        Baseline *b,
                        const int factor_length,
                        float lambda,
                        bool isU,
                        float *steps);

float * gradient(const float * const * u,
                 const float * const * v, 
                 entry_t e,
                 const DataAccessor * d,
                 Baseline *b,
                 const int factor_length,
                 float lambda,
                 bool isU,
                 float *factor_gradient);

float grad_of_grad(const float * row,
                   int index1,
                   int index2,
                   float regularization_coef // regularization_coef = lambda / num_entries)
                   );

// Coordinate gradient descent functions
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
                           int fold = -1);

#endif