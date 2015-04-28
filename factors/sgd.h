#ifndef SGD_H_INCLUDED
#define SGD_H_INCLUDED	

#include "../DataAccessor/data_accessor.h"

float * gradient(const float * const * u,
                 const float * const * v, 
                 const int index,
                 const DataAccessor * d,
                 const int factor_length,
                 float lambda,
                 bool isU);

float * coordinateGradient(const float * const * u,
                           const float * const * v,
                           const int index,
                           const int factor_length,
                           const int non_factor_width,
                           float lambda,
                           bool isU);

#endif