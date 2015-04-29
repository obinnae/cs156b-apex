#include <cmath>
#include "utils.h"

void load_avgs(char *filename, double *avg_array, int num_entries) {
  std::ifstream input(filename);
  std::string line;
  double avg;
  
  for (int i = 0; i < num_entries; i++) {
    getline(input, line);
    avg = atof(line.substr(line.find_first_of(',') + 1).c_str());
    avg_array[i] = avg;
  }
  input.close();
}

void parse_train_line(std::string line, int& user_id, int& mov_id, int& date, int& rating) {
  int sep1, sep2, sep3;
  sep1 = line.find_first_of(' ');
  sep2 = line.find_first_of(' ', sep1+1);
  sep3 = line.find_first_of(' ', sep2+1);
    
  user_id = atoi(line.substr(0, sep1).c_str()) - 1;
  mov_id = atoi(line.substr(sep1+1, sep2).c_str()) - 1;
  date = atoi(line.substr(sep2+1, sep3).c_str());
  rating = atoi(line.substr(sep3).c_str());
}

void parse_qual_line(std::string line, int& user_id, int& mov_id, int& date) {
  int sep1, sep2;
  sep1 = line.find_first_of(' ');
  sep2 = line.find_first_of(' ', sep1+1);
    
  user_id = atoi(line.substr(0, sep1).c_str()) - 1;
  mov_id = atoi(line.substr(sep1+1, sep2).c_str()) - 1;
  date = atoi(line.substr(sep2+1).c_str());
}

double calc_entry_weight(int date, double decay_per_day) {
//  double decay_per_day = 0.9986653; // oldest ratings are weighted 5% of newest ratings
  int days_from_present = 2243 - date;
  return pow(decay_per_day, days_from_present);
}