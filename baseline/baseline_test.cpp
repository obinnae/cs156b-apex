#include <cstdio>
#include "baseline.h"

int main(int argc, char *argv[]) {
  DataAccessor d;
  Baseline b;
  
  char *datafile;
  
  int user_id, movie_id, baseline_type;
  float baseline;
  
  if (argc != 5) {
    std::cout << "Usage: baseline_test <datafile> <user_id> <movie_id> <baseline_type>\n";
    exit(1);
  }
  datafile = argv[1];
  user_id = atoi(argv[2]);
  movie_id = atoi(argv[3]);
  baseline_type = atoi(argv[4]);
  
  std::cout << "Loading data file...\n";
  d.load_data(datafile);
  
  std::cout << "Setting up Baseline...\n";
  b.set_data(&d);
  b.set_type(baseline_type);

  std::cout << "Calculating baseline...\n";
  baseline = b.get_baseline(user_id, movie_id);
  
  std::cout << "Baseline calculated for user " << (user_id+1) << " and movie " << (movie_id+1) << ": " << baseline << std::endl;
}