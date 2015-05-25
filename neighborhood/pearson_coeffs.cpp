#include <iostream>
#include <cmath>
#include <fstream>
#include <ctime>

#include "../DataAccessor/data_accessor.h"

#define MAX_MOVIES 17770
#define MAX_USERS 458293

int getIntersection(const int * const major,
 					 int size_major,
 					 const int * const minor,
 					 int size_minor,
 					 int * intersection_a,
 					 int * intersection_b)
{
	int intersection_size = 0;

	int i = 0;
	int j = 0;

	do
	{
		if (major[i] == minor[j])
		{
			intersection_a[intersection_size] = i;
			intersection_b[intersection_size] = j;
			intersection_size += 1;
			i+=1;
			j+=1;
		}
		else if (major[i]>minor[j])
		{
			j+=1;
		}
		else
		{
			i+=1;
		}
	} while (i < size_major && j < size_minor);

	return intersection_size;
}

int BS_index (float new_coeff,
			  float * top_k_coeffsi,
			  int k){

	int lower = 0;
	int upper = k-1;
	int mid;

	while (lower != upper)
	{
		mid = (upper+lower)/2; //Intentionally taking advantage of integer division

		if (top_k_coeffsi[mid] > new_coeff)
		{
			lower = mid + 1;
			//printf("%f is greater than %f\n", new_coeff, top_k_coeffsi[mid]);
		}
		else
		{
			upper = mid;
			//printf("%f is less than %f\n", new_coeff, top_k_coeffsi[mid]);
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


void calcPearsonCoeff (float ** pearson_coeff,
					   int ** neighbor_index,
					   DataAccessor * d,
					   int num_neighbors){


	
    int num_movies = d->get_num_movies();
    time_t t1, t2;

	for (int i = 0; i < num_movies; i++)
	{
		t1 = time(NULL);
		printf("calculating coeffs for movie %d\n", i);

		entry_t * entry_major = new entry_t[MAX_ENTRIES_PER_MOVIE];
		int size_major = d->get_movie_entries(i, entry_major);
		int users_index_major [size_major];

		for (int l=0; l < size_major; l++)
		{
			users_index_major[l]=d->extract_user_id(entry_major[l]);
			//printf("%d\n",users_index_major[l]);
		}

		for (int j = 0; j < num_movies; j++)
		{
			float rho;
			//printf("calculating coeff for movies %d and %d\n", i, j);

			entry_t * entry_minor = new entry_t[MAX_ENTRIES_PER_MOVIE];

			int size_minor = d->get_movie_entries(j, entry_minor);
			int users_index_minor [size_minor];

			//printf("\n\n");


			for (int l=0; l < size_minor; l++)
			{
				users_index_minor[l]=d->extract_user_id(entry_minor[l]);
				//printf("%d\n", users_index_minor[l]);
			}

			int * major_intersection_index = new int [MAX_ENTRIES_PER_MOVIE];
			int * minor_intersection_index = new int [MAX_ENTRIES_PER_MOVIE];
			int intersection_size = getIntersection(users_index_major, size_major, users_index_minor, size_minor, major_intersection_index, minor_intersection_index);

			//printf("intersection size = %d\n", intersection_size);
			//printf("size_major: %d\n", size_major);
			//printf("size_minor: %d\n\n", size_minor);

			//LOOP BELOW CALCULATES SUM OF RATINGS FROM INTERSECTION;

			if (intersection_size < 2)
			{
				rho = 0;
			}

			else
			{
				float major_ratings_sum =0;
				float minor_ratings_sum =0;
	
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
	
				rho = nrho/(intersection_size - 1);
			}

			if (rho > pearson_coeff[i][num_neighbors-1])
			{
				int insert_index = BS_index (rho, pearson_coeff[i], num_neighbors);

				reorderCoeffs (insert_index,pearson_coeff[i], num_neighbors, rho);
				reorderIndexes (insert_index,neighbor_index[i], num_neighbors, j);
			}

			if (j > i )
			{
				int insert_index = BS_index (rho, pearson_coeff[j], num_neighbors);

				reorderCoeffs (insert_index,pearson_coeff[j], num_neighbors, rho);
				reorderIndexes (insert_index,neighbor_index[j], num_neighbors, i);
			}


			//printf("movie %d to %d correlation = %f \n\n\n\n\n", i, j, rho);

			pearson_coeff[i][j] = rho;

			delete [] major_intersection_index;
			delete [] minor_intersection_index;
			delete [] entry_minor;
		}
		delete [] entry_major;
		t2 = time(NULL);
		std::cout << "iteration time: " << difftime(t2, t1) << " sec\n";
	}

}


int main(int argc, char *argv[])
{

	DataAccessor d;
	char * data_path;
	char * neighboursfilepath;
	data_path = argv[2];
	neighboursfilepath = argv [3];
	int num_neighbors = atoi(argv[1]);
	// char * cpath = path.c_str();
  	d.load_data(data_path);

  	int num_movies = d.get_num_movies();

	float ** pearson_coeff = new float * [num_movies];
  	for(int i = 0; i < num_movies; i++)
  	{
    	pearson_coeff[i] = new float[num_neighbors];
  	}
  	int ** neighbor_index = new int * [num_movies];
  	for(int i = 0; i < num_movies; i++)
  	{
    	neighbor_index[i] = new int[num_neighbors];
  	}
  	calcPearsonCoeff(pearson_coeff, neighbor_index, &d, num_neighbors);

  	std::ofstream coeffsFile;
    coeffsFile.open(neighboursfilepath);

    for (int i = 0; i < num_movies; i++)
    {
    	for (int j; j < num_neighbors; j++)
    	{
    		int ind = neighbor_index[i][j];
    		coeffsFile << ind << " ";
    	}
    	coeffsFile << "\n";

    }
    coeffsFile.close();
}


//BELOW FUNCTIONS ARE HELPER FUNCTIONS WE WONT NEED YET

/*
 * Function to generate index of where array reordering begins for top_k[i]
 * 
 * new_coeff = new pearson coeff just calculated in calcPearsonCoeff
 * top_k_coeffsi = array of coefficients of top k most correlated movies to movie i
 * k = length of array
 */