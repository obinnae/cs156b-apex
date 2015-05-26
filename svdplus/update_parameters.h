#ifndef UPDATE_PARAMS_INCLUDE
#define UPDATE_PARAMS_INCLUDE

#include "../baseline/baseline.h"
#include "../DataAccessor/data_accessor.h"

// void updateUserBaseline(int user_index,
// 						baseline * b,
// 						float rate,
// 						float lambda,
// 						float error
// 				   		);

// void updateMovieBaseline(int unser_index,
// 						 baseline * b,
// 						 float rate,
// 						 float lambda,
// 						 float error
// 				   		 );

void updateWeights(int user_index,
				   int k,
				   int movie_index,
				   int * movie_indexes,
				   float ** w,
				   const DataAccessor * d,
				   Baseline * b,
				   float rate,
				   float lambda,
				   float error);

float weightSum(int user_index,
				int movie_index,
				int k,
				float ** w,
				int ** r,
				const DataAccessor * d,
				Baseline * b
				);

#endif