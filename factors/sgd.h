#ifndef SGD_H_INCLUDED
#define SGD_H_INCLUDED	

#include <cstring>
#include "../baseline/baseline.h"
#include "../DataAccessor/data_accessor.h"

float * gradient(const float * const * u,
                 const float * const * v, 
                 const int index,
                 const DataAccessor * d,
                 Baseline *b,
                 const int factor_length,
                 float lambda,
                 bool isU);

float * coordinateGradient(const float * const * u,
                           const float * const * v,
                           const int index,
                           const DataAccessor * d,
                           Baseline * b,
                           const int factor_length,
                           float lambda,
                           bool isU);

#endif