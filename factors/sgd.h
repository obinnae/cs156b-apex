#ifndef SGD_H_INCLUDED
#define SGD_H_INCLUDED	

#include <cstring>
#include "../baseline/baseline.h"
#include "../DataAccessor/data_accessor.h"

float * gradient(const float * const * u,
                 const float * const * v,
                 float ** w,
                 int ** r,
                 entry_t e,
                 const DataAccessor * d,
                 Baseline *b,
                 const int factor_length,
                 float lambda,
                 int num_neighbors,
                 bool isU,
                 float *factor_gradient);

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