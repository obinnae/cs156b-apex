#include <iostream>
#include <cmath>
#include <fstream>

#include "../DataAccessor/data_accessor.h"

#define MAX_MOVIES 17770
#define MAX_USERS 458293


void calcPearsonCoeff (float ** pearson_coeff,
					   bool ** coeff_calculated,
					   DataAccessor * d,
					   char * coeffsfilepath){


	entry_t * entry_major = new entry_t[MAX_ENTRIES_PER_MOVIE];
	entry_t * entry_minor = new entry_t[MAX_ENTRIES_PER_MOVIE];
	std::ofstream coeffsFile;
    coeffsFile.open(coeffsfilepath);

    /*std::cout << "Testing data file...\n";
    for (int i = 0; i < MAX_MOVIES; i++) {
    	d->get_movie_entries(i, entry_major);
    }
    std::cout << "Verified!\n";*/

	for (int i = 0; i < MAX_MOVIES; i++)
	{
		printf("calculating coeffs for movie %d\n", i);

		int size_major = d->get_movie_entries(i, entry_major);
		int users_index_major [size_major];

		for (int l=0; l < size_major; l++)
		{
			users_index_major[l]=d->extract_user_id(entry_major[l]);
		}

		for (int j = 0; j < MAX_MOVIES; j++)
		{
			printf("calculating coeff for movies %d and %d\n", i, j);
			

			if (coeff_calculated[j][i])
			{
				pearson_coeff[i][j] = pearson_coeff[j][i];
				coeff_calculated[i][j] = 1;
				continue;
			}
			int size_minor = d->get_movie_entries(j, entry_minor);
			int users_index_minor [size_minor];


			for (int l=0; l < size_minor; l++)
			{
				users_index_minor[l]=d->extract_user_id(entry_minor[l]);
			}

			int * major_intersection_index = new int [MAX_ENTRIES_PER_MOVIE];
			int * minor_intersection_index = new int [MAX_ENTRIES_PER_MOVIE];
			int intersection_size = 0;

			//LOOP BELOW GETS INTERSECTION OF MAJOR AND MINOR ARRAY

			for (int m = 0; m <size_major; m++)
			{
				for (int n = 0; n < size_minor; n++)
				{
					if (users_index_major[m] == users_index_minor[n])
					{
						major_intersection_index[intersection_size]=m;
						minor_intersection_index[intersection_size]=n;
						intersection_size++;
						printf("user_id from major: %d \t user_d from minor: %d\n", users_index_major[m],users_index_minor[n]);
						n = size_minor;
					}
				}
			} // END OF INTERSECTION SEARCH

			printf("intersection size = %d\n", intersection_size);
			printf("size_major: %d\n", size_major);
			printf("size_minor: %d\n", size_minor);

			//LOOP BELOW CALCULATES SUM OF RATINGS FROM INTERSECTION;

			float major_ratings_sum;
			float minor_ratings_sum;

			for (int m = 0; m < intersection_size; m++)
			{
				major_ratings_sum += (float) d->extract_rating(entry_major[major_intersection_index[m]]);
				minor_ratings_sum += (float) d->extract_rating(entry_minor[minor_intersection_index[m]]); //HMMMMN
			}

			float major_ratings_avg = major_ratings_sum/intersection_size;
			float minor_ratings_avg = minor_ratings_sum/intersection_size;

			//LOOP BELOW CALCULATES THE SUM OF THE VARIANCES OF EACH RATING FROM AVG

			float major_var = 0;
			float minor_var = 0;

			for (int m = 0; m < intersection_size; m++)
			{
				major_var += (d->extract_rating(entry_major[major_intersection_index[m]]) - major_ratings_avg) * (d->extract_rating(entry_major[major_intersection_index[m]]) - major_ratings_avg);
				minor_var += (d->extract_rating(entry_minor[minor_intersection_index[m]]) - minor_ratings_avg) * (d->extract_rating(entry_minor[minor_intersection_index[m]]) - minor_ratings_avg);
			}

			float s_major = major_var/(intersection_size - 1);
			float s_minor = minor_var/(intersection_size - 1);

			//LOOP BELOW CALCULATES RHO VALUES USING FINAL PEARSON COEFF VALUE

			float nrho = 0;

			for (int m = 0; m < intersection_size; m++)
			{
				nrho += ( (d->extract_rating(entry_major[major_intersection_index[m]]) - major_ratings_avg)/(sqrt (s_major)) ) * ( (d->extract_rating(entry_minor[minor_intersection_index[m]]) - minor_ratings_avg)/( sqrt (s_minor)) );
			}

			float rho = nrho/(intersection_size - 1);
			pearson_coeff[i][j] = rho;
			coeff_calculated[i][j] = 1;

			//printf("movie %d to %d correlation = %f \n", i, j, rho);

			coeffsFile << rho << " ";

			delete [] major_intersection_index;
			delete [] minor_intersection_index;
		}

		coeffsFile << "\n";
	}

	coeffsFile.close();

	delete [] entry_major;
	delete [] entry_minor;
}


int main(int argc, char *argv[])
{
	DataAccessor d;
	char * data_path;
	char * coeffsfilepath;
	data_path = argv[1];
	coeffsfilepath = argv [2];
	// char * cpath = path.c_str();
  	d.load_data(data_path);

	float ** pearson_coeff = new float * [MAX_MOVIES];
  	for(int i = 0; i < MAX_MOVIES; i++)
  	{
    	pearson_coeff[i] = new float[MAX_MOVIES];
  	}

  	bool ** coeff_calculated = new bool * [MAX_MOVIES];
  	for(int i = 0; i < MAX_MOVIES; i++)
  	{
  		coeff_calculated[i] = new bool [MAX_MOVIES];
  		memset(coeff_calculated[i], 0, sizeof(bool) * MAX_MOVIES);
  	}
  	calcPearsonCoeff(pearson_coeff, coeff_calculated, &d, coeffsfilepath);



	// int Nij = 6;
	// int a1 [] = {4,6,2,2,8,10};
	// //int a2 [] = {3,5,1,1,7,9};
	// int a2 [] = {2,3,1,1,4,5};
	// float a1_sum = 0;
	// float a2_sum = 0;
	// float nvar1 = 0;
	// float nvar2 = 0;
	// float ncor = 0;

	// for (int i = 0; i < Nij; i++)
	// {
	// 	a1_sum += (float) a1[i];
	// 	a2_sum += (float) a2[i];
	// }
	// float a1_avg = a1_sum/Nij;
	// float a2_avg = a2_sum/Nij;

	// printf("%f \t%f\n", a1_avg, a2_avg);

	// for (int i = 0; i < Nij; i++)
	// {
	// 	nvar1 += ((a1[i] - a1_avg) * (a1[i] - a1_avg));
	// 	nvar2 += ((a2[i] - a2_avg) * (a2[i] - a2_avg));
	// }

	// float s1 = nvar1/(Nij - 1);
	// float s2 = nvar2/(Nij - 1);

	// printf("%f \t%f\n", s1, s2);

	// for (int i = 0; i < Nij; i++)
	// {
	// 	ncor += ( (a1[i] - a1_avg)/(sqrt (s1)) ) * ( (a2[i] - a2_avg)/( sqrt (s2)) );
	// }

	// float r = ncor/(Nij - 1);

	// printf("%f\n", r);
}


//BELOW FUNCTIONS ARE HELPER FUNCTIONS WE WONT NEED YET

/*
 * Function to generate index of where array reordering begins for top_k[i]
 * 
 * new_coeff = new pearson coeff just calculated in calcPearsonCoeff
 * top_k_coeffsi = array of coefficients of top k most correlated movies to movie i
 * k = length of array
 */
int BS_index (float new_coeff,
			  float * top_k_coeffsi,
			  int k){

	int lower = 0;
	int upper = k-1;
	int mid;

	while (lower != upper)
	{
		mid = upper+lower/2; //Intentionally taking advantage of integer division

		if (top_k_coeffsi[mid] < new_coeff)
		{
			lower = mid + 1;
		}
		else
		{
			upper = mid;
		}
	}

	if (top_k_coeffsi[lower] < new_coeff)
	{
		return lower;
	}
	else
	{
		return lower + 1;
	}
}

/*
 * Function to reorder coeffs array to accomodate new coefficient and keep sorted
 * index = index gotten from using BS_index
 */

 void reorderCoeffs (int index,
 					 float * top_k_coeffsi,
 					 int k,
 					 float new_coeff){

 	for (int j=index; j < k; j++)
 	{
 		float temp = top_k_coeffsi[j];
 		top_k_coeffsi[j] = new_coeff;
 		new_coeff = temp;
 	}
 }

 void reorderIndexes (int index,
 					  int * top_k_indexesi,
 					  int k,
 					  int new_index){
 	for (int j=index; j < k; j++)
 	{
 		int temp = top_k_indexesi[j];
 		top_k_indexesi[j] = new_index;
 		new_index = temp;
 	}
 }