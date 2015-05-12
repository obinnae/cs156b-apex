#ifndef UPDATE_PARAMS_INCLUDE
#define UPDATE_PARAMS_INCLUDE

#include "../baseline/baseline.h"
#include "../DataAccessor/data_accessor.h"

void updateUserBaseline(int user_index,
						baseline * b,
						float rate,
						float lambda,
						float error
				   		);

void updateMovieBaseline(int unser_index,
						 baseline * b,
						 float rate,
						 float lambda,
						 float error
				   		 );

void updateWeights(int user_index,
				   int num_rated_movies,
				   int * movie_indexes,
				   float * w,
				   baseline * b,
				   int rating,
				   float lambda,
				   float error);

updateCoefficients(int user_index,
				   int num_rated_movies,
				   int * movie_indexes,
				   float * c,
				   float lambda,
				   float error);


#endif