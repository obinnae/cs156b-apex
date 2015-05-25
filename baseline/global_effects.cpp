/*
 * Nancy Wen
 * May 25, 2015
 *
 * This program calculates the baseline for the training data.
 *
 */

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <fstream>
#include <string>
#include <cstdlib>
#include "../DataAccessor/data_accessor.h"

#define QUAL_SIZE 2749898
#define MAX_USERS 458293
#define MAX_MOVIES 17770
#define GLOBAL_AVG 3.6081
using namespace std;

//sums up the residual values
float find_residual_sum(float * r, int size){
	float sum = 0;
 	for(int i = 0; i < size; i++) {
 		sum = sum + r[i] * r[i];
 	}
 	return sum/size;
}

//find the means for all the movies
float find_movie_mean(int movie_id, DataAccessor * d){
 	entry_t *entries = new entry_t[MAX_ENTRIES_PER_MOVIE];
 	int num_entries;
	int curr_rating;
	int sum = 0;
	int num_ratings = 0;
	float average;
	num_entries = d->get_movie_entries(movie_id, entries);
	for(int i = 0; i < num_entries; i++){
		curr_rating = d->extract_rating(entries[i]);
		if(curr_rating != 0)
 		{
 			sum = sum + curr_rating;
 			num_ratings++;
 		}
	}
	average = (float)sum/(float)num_ratings;
	delete[] entries;
	return average;
}

//find the mean for all the movies (using residuals)
float find_movie_residual_mean(int movie_id, DataAccessor * d, float * r){
 	entry_t *entries = new entry_t[MAX_ENTRIES_PER_MOVIE];
 	entry_t curr_entry;
 	int num_entries;
	int curr_index;
	float sum = 0;
	int num_ratings = 0;
	float average;
	num_entries = d->get_movie_entries(movie_id, entries);
	for(int i = 0; i < num_entries; i++){
		curr_index = d->extract_user_id(entries[i]);
		curr_entry = d->get_entry(curr_index, movie_id);
		curr_index = d->extract_entry_index(curr_entry);
		//curr_index = d->extract_entry_index(entries[i]);
 		sum = sum + r[curr_index];
 		num_ratings = num_ratings + 1;
	}
	average = sum/(float)num_ratings;
	delete[] entries;
	return average;
}

 //find the means for all the users
 float find_user_mean(int user_id, DataAccessor * d)
 {
 	entry_t *entries = new entry_t[MAX_ENTRIES_PER_USER];
	int num_entries;
	int curr_rating;
	int sum = 0;
	int num_ratings = 0;
	float average;
	num_entries = d->get_user_entries(user_id, entries);

	for(int i  = 0; i < num_entries; i++){
		curr_rating = d->extract_rating(entries[i]);
		if(curr_rating != 0)
 		{
 			sum = sum + curr_rating;
 			num_ratings++;
 		}
	}
	average = (float)sum/(float)num_ratings;
	delete[] entries;
	return average;
 }

//find the mean for all the users (using residuals)
float find_user_residual_mean(int user_id, DataAccessor * d, float * r){
 	entry_t *entries = new entry_t[MAX_ENTRIES_PER_MOVIE];
 	int num_entries;
	int curr_index;
	float sum = 0;
	int num_ratings = 0;
	float average;
	num_entries = d->get_user_entries(user_id, entries);
	for(int i = 0; i < num_entries; i++){
		curr_index = d->extract_entry_index(entries[i]);
 		sum = sum + r[curr_index];
 		num_ratings = num_ratings + 1;
	}
	average = sum/(float)num_ratings;
	delete[] entries;
	return average;
}

//given an entry index, return the movie index
int entry_to_movie_index(int entry_index, DataAccessor * d){
	entry_t curr_entry = d->get_entry(entry_index);
	return d->extract_movie_id(curr_entry);
}

//given an entry index, return the user index
int entry_to_user_index(int entry_index, DataAccessor * d){
	entry_t curr_entry = d->get_entry(entry_index);
	return d->extract_user_id(curr_entry);
}

//given a probe index, return the movie index
int probe_to_movie_index(int probe_index, DataAccessor * d2)
{
	entry_t curr_entry = d2->get_entry(probe_index);
	return d2->extract_movie_id(curr_entry);
}

//given a probe index, return the user index
int probe_to_user_index(int probe_index, DataAccessor * d2)
{
	entry_t curr_entry = d2->get_entry(probe_index);
	return d2->extract_user_id(curr_entry);
}


//update the residuals matrix
void update_residuals(float * r, float * theta, bool movie_effect, int num_entries, DataAccessor * d){
	entry_t curr_entry;
	if(movie_effect){
		//loop through all of r and update based on the movies
		for(int i = 0; i < num_entries; i++){
			//pull out the entry for index r
			int curr_index = entry_to_movie_index(i, d);
			r[i] = r[i] - theta[curr_index];
		}
	}
	else{
		//loop through all of r and update based on the users
		for(int i = 0; i < num_entries; i++){
			int curr_index = entry_to_user_index(i, d);
			r[i] = r[i] - theta[curr_index];
		}
	}
}

//update the residuals matrix with x_ui
void update_residuals_x_ui(float * r, float * theta, float * x_ui, bool movie_effect, int num_entries, DataAccessor * d){
	entry_t curr_entry;
	if(movie_effect){
		//loop through all of r and update based on the movies
		for(int i = 0; i < num_entries; i++){
			//pull out the entry for index r
			int curr_index = entry_to_movie_index(i, d);
			r[i] = r[i] - theta[curr_index] * x_ui[curr_index];
		}
	}
	else{
		//loop through all of r and update based on the users
		for(int i = 0; i < num_entries; i++){
			int curr_index = entry_to_user_index(i, d);
			r[i] = r[i] - theta[curr_index] * x_ui[curr_index];
		}
	}
}

//this method prints out the values in the array to a file
void print_array_to_file(float * arr, int size){
	std::ofstream outputfile;
	outputfile.open("theta_1.dta"); //changed the pathname
	for(int i = 0; i < size; i++)
	{
		outputfile << arr[i] << endl;
	}
}

//this method compares probe ratings to test ratings
float evaluate_ratings(float * probe_ratings, float * test_ratings, int num_entries){
	float diff;
	float sum = 0;
	for(int i = 0; i < num_entries; i++){
		float diff = probe_ratings[i] - test_ratings[i];
		sum  = sum  + (diff * diff);
	}
	float rmse = sum / num_entries;
	rmse = std::sqrt(rmse);
	return rmse;
}

//this method returns the earliest rating for a user index
int first_user_date(int user_id, DataAccessor * d){
 	int min_date = 2243; //this is the last day
 	int curr_date;
 	entry_t *entries = new entry_t[MAX_ENTRIES_PER_USER];
 	int num_entries = d->get_user_entries(user_id, entries);
 	for(int i = 0; i < num_entries; i++)
 	{
 		curr_date = d->extract_date(entries[i]);
 		if(curr_date < min_date)
 			min_date = curr_date;
 	}
 	delete[] entries;
 	return min_date;
}

//this method returns the earliest rating for a movie index
int first_movie_date(int movie_id, DataAccessor * d){
	int min_date = 2243; //this is the last day
 	int curr_date;
 	entry_t *entries = new entry_t[MAX_ENTRIES_PER_MOVIE];
 	int num_entries = d->get_movie_entries(movie_id, entries);
 	for(int i = 0; i < num_entries; i++)
 	{
 		curr_date = d->extract_date(entries[i]);
 		if(curr_date < min_date)
 			min_date = curr_date;
 	}
 	delete[] entries;
 	return min_date;
}

//this method gets the time elapsed since user's first rating
int time_elapsed_user(int user_id, int movie_id, DataAccessor * d) {
	//pull the date for this particular rating
	entry_t curr_entry = d->get_entry(user_id, movie_id);
	int rating_date = d->extract_date(curr_entry);
	int first_date = first_user_date(user_id, d);
	return std::sqrt(rating_date - first_date);
}

//this method gets the time elapsed since the movie's first rating
int time_elapsed_movie(int user_id, int movie_id, DataAccessor * d) {
	entry_t curr_entry = d->get_entry(user_id, movie_id);
	int rating_date = d->extract_date(curr_entry);
	int first_date = first_movie_date(user_id, d);
	return std::sqrt(rating_date - first_date);
}

//this method fills the time array
void fill_time_array(float * x_ui, DataAccessor * d, bool user, int num_entries) {
	int user_id;
	int movie_id;
	//time since user's first rating
	if(user){
		for(int i = 0; i < num_entries; i++) {
			std::cout << "Getting time elapsed for entry: " << i << endl;
 			user_id = entry_to_user_index(i, d);
 			movie_id = entry_to_movie_index(i, d);
 			x_ui[i] = time_elapsed_user(user_id, movie_id, d);
 		}
 	}
 	//time since movie's first rating
 	else
 	{
		for(int i = 0; i < num_entries; i++) {
 			user_id = entry_to_user_index(i, d);
 			movie_id = entry_to_movie_index(i, d);
 			x_ui[i] = time_elapsed_movie(user_id, movie_id, d);
 		}
 	}
}

//this method returns the x value given a user and a movie index: time elapsed since first rating
float get_value_x(user_index, movie_index, bool movie, DataAccessor * d){

}

int find_theta_3(int user_id, float * r, float * x_ui, DataAccessor * d) {
 	entry_t * user_entries = new entry_t[MAX_ENTRIES_PER_USER];
 	int num_entries = d->get_user_entries(user_id, user_entries);
 	int curr_id;
 	float top_sum = 0;
 	float bottom_sum = 0;
 	//int * entry_ids = new int[num_entries];
 	for(int i = 0; i< num_entries; i++) {
 		curr_id = d->extract_entry_index(user_entries[i]);
 		top_sum = top_sum + r[curr_id] * x_ui[curr_id];
 		bottom_sum = bottom_sum + x_ui[curr_id] * x_ui[curr_id];
 	}
 	delete[] user_entries;
 	return top_sum/bottom_sum;

 }


int main() {
	//load all the training data entries
 	DataAccessor d;
	d.load_data("../Data/train.cdta");
	int num_entries = d.get_num_entries();

	//load all the probe entries
	DataAccessor d2;
	d2.load_data("../Data/probe.cdta");
	int num_probes = d2.get_num_entries();


	//save the probe ratings to an array
	entry_t curr_entry;
	float * probe_ratings = new float[num_entries];
	for(int i  = 0; i < num_entries; i++) {
		curr_entry = d2.get_entry(i);
		probe_ratings[i] = d2.extract_rating(curr_entry);
	}

	/*******************************************
 	 * Effect 0: Global average
 	 *******************************************/
 	float * r = new float[num_entries];
 	for (int i = 0; i < num_entries; i++){
 		curr_entry = d.get_entry(i);
 		r[i] = d.extract_rating(curr_entry) - GLOBAL_AVG;
 	}

	//float error = find_residual_sum(r, num_entries);
 	//std::cout << error << endl;

 	//create array of test_ratings
 	float * test_ratings = new float[num_probes];
 	for(int i = 0; i < num_probes; i++){
 		test_ratings[i] = GLOBAL_AVG;
 	}


 	//compare the probe ratings to ratings in the file
	float rmse = evaluate_ratings(probe_ratings, test_ratings, num_probes);

	//print out the rmse
	std::cout << rmse << endl;



 	/*******************************************
 	 * Effect 1: Movie effect
 	 *******************************************/
 	//float * movie_avg = new float[MAX_MOVIES];

 	float * movie_freq = new float[MAX_MOVIES];
 	float * theta_1 = new float[MAX_MOVIES];
 	float alpha_1 = 25;
 	for(int i = 0; i < MAX_MOVIES; i++){
 		movie_freq[i] = d.get_num_movie_entries(i);
 		theta_1[i] = find_movie_residual_mean(i, &d, r);
 		theta_1[i] = (theta_1[i] * movie_freq[i])/(alpha_1 + movie_freq[i]);
 	}
 	//true since it is a movie effect
 	update_residuals(r, theta_1, true, num_entries, &d);
 	//error = find_residual_sum(r, num_entries);
 	//std::cout << error << endl;

 	//update test_ratings to include the movie effect
 	for(int i = 0; i < num_probes; i++){
 		int curr_index = probe_to_movie_index(i, &d2);
 		test_ratings[i] = test_ratings[i] + theta_1[curr_index];
 	}

 	//compare the probe ratings to ratings in the file
	rmse = evaluate_ratings(probe_ratings, test_ratings, num_probes);
	std::cout << rmse << endl;

	/*
	//print theta_1 to file
	std::ofstream outputfile;
	outputfile.open("theta_1.dta");
	for(int i = 0; i < MAX_MOVIES; i++)
	{
		outputfile << theta_1[i] << endl;
	}
	outputfile.close();
	*/

 	/*******************************************
 	 * Effect 2: User effect
 	 *******************************************/

 	float * user_avg = new float[MAX_USERS];
 	float * user_freq = new float[MAX_USERS];
 	float * theta_2 = new float[MAX_USERS];
 	float alpha_2 = 7;
 	for (int i = 0; i < MAX_USERS; i++) {
 		user_freq[i] = d.get_num_user_entries(i);
 		theta_2[i] = find_user_residual_mean(i, &d, r);
 		theta_2[i] = (theta_2[i] * user_freq[i])/(alpha_2 + user_freq[i]);
 	}
 	//false since it is a user effect
 	update_residuals(r, theta_2, false, num_entries, &d);

 	//Probe: update test_ratings to include the user effect
 	for(int i = 0; i < num_probes; i++){
 		int curr_index = probe_to_user_index(i, &d2);
 		test_ratings[i] = test_ratings[i] + theta_2[curr_index];
 	}

 	//compare the probe ratings to ratings in the file
	rmse = evaluate_ratings(probe_ratings, test_ratings, num_probes);
	std::cout << rmse << endl;

    /*
	//print theta_2 to file
	outputfile.open("theta_2.dta");
	for(int i = 0; i < MAX_USERS; i++)
	{
		outputfile << theta_2[i] << endl;
	}
	outputfile.close();



	//print the probe ratings to a file
	std::ofstream outputfile;
	outputfile.open("probe_2.dta");
	for(int i = 0; i < num_probes; i++)
	{
		outputfile << test_ratings[i] << endl;
	}
	outputfile.close();
	*/


	//print the qual ratings to a file

 	/*******************************************
 	* Effect 3: User x Time(user)^(1/2)
 	*******************************************/
 	float alpha_3 = 550;


 	//find theta_3 by looping through all the users
 	float * theta_3 = new float[MAX_USERS];
 	for(int i = 0; i < MAX_USERS; i++){
 		theta_3[i] = find_theta_3(i, r, x_ui, &d);
 		std::cout << "Updating theta for user: " << i << endl;
 		theta_3[i] = (theta_3[i] * user_freq[i])/(alpha_3 + user_freq[i]);
 	}

 	//update residuals
 	update_residuals_x_ui(r, theta_3, x_ui, false, num_entries, &d);

 	std::cout << "Update residuals" << endl;



 	//update test_ratings to include the user x time(user) effect
 	for(int i = 0; i < num_probes; i++){
 		int user_index = probe_to_user_index(i, &d2);
 		int movie_index = probe_to_movie_index(i, &d2);
 		float * x = new float[num_probes];
 		fill_time_array(x, &d2, true, num_probes);
 		test_ratings[i] = test_ratings[i] + theta_3[user_index] * x[user_index];

 		std::cout<< "Generating test rating for probe: " << i << endl;
 		delete[] x;
 	}

 	//compare the probe ratings to ratings in the file
	rmse = evaluate_ratings(probe_ratings, test_ratings, num_probes);
	std::cout << rmse << endl;
	*/

 	/*******************************************
 	* Effect 4: User x Time(movie)^(1/2)
 	*******************************************/
 	float alpha_4 = 150;

 	//fill the x_ui array for the time elapsed since movie first rating
 	//fill_time_array(x_ui, &d, false, num_entries);

 	//find theta_4 by looping through all the movies

 	//update residuals

 	//update


 	/*******************************************
 	* Effect 5: Movie x Time(movie)^(1/2)
 	*******************************************/
 	float alpha_5 = 4000;

 	/*******************************************
 	* Effect 6: Movie x Time(user)^(1/2)
 	*******************************************/
 	float alpha_6 = 500;

 	/*******************************************
 	* Effect 7: User x Movie average
 	*******************************************/
 	float alpha_7 = 90;

 	/*******************************************
 	* Effect 8: User x Movie support
 	*******************************************/
 	float alpha_8 = 90;

 	/*******************************************
 	* Effect 9: Movie x User average
 	*******************************************/
 	float alpha_9 = 50;

 	/*******************************************
 	* Effect 10: Movie x User support
 	*******************************************/
 	float alpha_10 = 50;

 	delete[] movie_freq;
 	delete[] user_freq;
 	delete[] probe_ratings;
 	delete[] r;
 	delete[] test_ratings;
 	//delete[] theta_1;
 	//delete[] theta_2;
 	//delete[] theta_3;
	return 0;
}