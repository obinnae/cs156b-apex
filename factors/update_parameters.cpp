#include <iostream>

#include "../baseline/baseline.h"
#include "../DataAccessor/data_accessor.h"

void updateUserBaseline(int user_index,
						baseline * b,
						float rate,
						float lambda,
						float error
				   		){
	float user_baseline = b->getUserBaseline(user_index); // Function doesn't exist (at least not yet)
	float updated_user_baseline = user_baseline + rate(error - (lambda * user_baseline));
	b->updateUserBaseline(user_index, updated_user_baseline); //Doesn't exist either
}

void updateMovieBaseline(int movie_index,
						 baseline * b,
						 float rate,
						 float lambda,
						 float error
				   		 ){

	float movie_baseline = b->getMovieBaseline(movie_index); // Function doesn't exist (at least not yet)
	float updated_movie_baseline = movie_baseline + rate(error - (lambda * movie_baseline));
	b->updateUserBaseline(movie_index, updated_movie_baseline); //Doesn't exist either
}

void updateWeights(int user_index,
				   int num_rated_movies,
				   int * movie_indexes,
				   float ** w,
				   baseline * b,
				   int rating,
				   float rate,
				   float lambda,
				   float error){
	for (i=0, i<num_rated_movies, i++){
		for (j=0, j<num_rated_movies, j++){
			w_new = w[i][j] + rate *( (1/ sqrt(num_rated_movies)) * error * (rating - b->getBaseLine(user_index, movie_indexes[j])) -lambda * w[i][j] );
			w[i][j] = w_new;
		}
	}

}

updateCoefficients(int user_index,
				   int num_rated_movies,
				   int * movie_indexes,
				   float ** c,
				   float rate,
				   float lambda,
				   float error);


#endif