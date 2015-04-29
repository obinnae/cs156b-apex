#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>


#define MAX_MOVIES 17770
#define MAX_USERS 458293
#define MAX_TRAINING_ENTRIES 102416306
#define NUM_RATINGS 6
#define NUM_DATES 2243


void load_avgs(char *filename, double *avg_array, int num_entries);

void parse_train_line(std::string line, int& user_id, int& mov_id, int& date, int& rating);
void parse_qual_line(std::string line, int& user_id, int& mov_id, int& date);

double calc_entry_weight(int date, double decay_per_day);

#endif //UTILS_H