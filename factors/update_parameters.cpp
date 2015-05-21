#include <iostream>
#include "update_parameters.h"
#include "../baseline/baseline.h"
#include "../DataAccessor/data_accessor.h"

#include <cmath>

// void updateUserBaseline(int user_index,
// 						baseline * b,
// 						float rate,
// 						float lambda,
// 						float error
// 				   		){
// 	float user_baseline = b->getUserBaseline(user_index); // Function doesn't exist (at least not yet)
// 	float updated_user_baseline = user_baseline + rate(error - (lambda * user_baseline));
// 	b->updateUserBaseline(user_index, updated_user_baseline); //Doesn't exist either
// }

// void updateMovieBaseline(int movie_index,
// 						 baseline * b,
// 						 float rate,
// 						 float lambda,
// 						 float error
// 				   		 ){

// 	float movie_baseline = b->getMovieBaseline(movie_index); // Function doesn't exist (at least not yet)
// 	float updated_movie_baseline = movie_baseline + rate(error - (lambda * movie_baseline));
// 	b->updateUserBaseline(movie_index, updated_movie_baseline); //Doesn't exist either
// }

void updateWeights(int user_index,
				   int k,
				   int movie_index,
				   int * movie_indexes,
				   float ** w,
				   const DataAccessor * d,
				   Baseline * b,
				   float rate,
				   float lambda,
				   float error){

	for (int j=0; j<k; j++){
		entry_t e = d->get_entry(user_index, movie_index);
		int rating = d->extract_rating(e);
		float w_new = w[movie_index][j] + rate *( (1/ sqrt(k)) * error * (rating - b->get_baseline(user_index, movie_indexes[j])) -lambda * w[movie_index][j] );
		w[movie_index][j] = w_new;
	}

}

float weightSum(int user_index,
				int movie_index,
				int k,
				float ** w,
				int ** r,
				const DataAccessor * d,
				Baseline * b
				){

	float sum_ws = 0;
	for (int j = 0; j < k; j ++)
	{
		if (d->has_entry(user_index, r[movie_index][j]))
		{
			entry_t e = d->get_entry(user_index, r[movie_index][j]);
			int rating_uj = d->extract_rating(e);
			sum_ws += w[movie_index][r[movie_index][j]] * (rating_uj - b->get_baseline(user_index, r[movie_index][j]));
		}
		else
		{
			sum_ws+=0;
		}
	}
	return sum_ws;
}