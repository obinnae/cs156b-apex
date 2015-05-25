#include <iostream>
#include <cstdlib>
#include "../DataAccessor/data_accessor.h"
#include <cmath>
using namespace std;

#define GLOBAL_AVG 3.6081

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

int main(){

	//load all the probe entries
	DataAccessor d;
	d.load_data("../Data/probe.cdta");
	int num_entries = d.get_num_entries();

	//save the probe ratings to an array
	entry_t curr_entry;
	float * probe_ratings = new float[num_entries];
	for(int i  = 0; i < num_entries; i++) {
		curr_entry = d.get_entry(i);
		probe_ratings[i] = d.extract_rating(curr_entry);
	}

	//make a array of the overall mean
	float * avg_ratings = new float[num_entries];
	for(int i  = 0; i < num_entries; i++) {
		avg_ratings[i] = GLOBAL_AVG;
	}

	//compare the probe ratings to ratings in the file
	float rmse = evaluate_ratings(probe_ratings, avg_ratings, num_entries);

	//print out the rmse
	std::cout<< rmse << endl;
	return 0;
}