#include <cstdio>
#include "baseline.h"

int main(int argc, char *argv[]) {
  DataAccessor d;
  Baseline b(&d);
  
  char *datafile;
  
  int user_id, movie_id;
  float baseline;
  
  if (argc != 4) {
    std::cout << "Usage: baseline_test <datafile> <user_id> <movie_id>\n";
    exit(1);
  }
  datafile = argv[1];
  user_id = atoi(argv[2]);
  movie_id = atoi(argv[3]);
  
  std::cout << "Loading data file...\n";
  d.load_data(datafile);
  
  std::cout << "Calculating baseline...\n";
  baseline = b.get_baseline(user_id, movie_id);
  
  std::cout << "Baseline calculated for user " << (user_id+1) << " and movie " << (movie_id+1) << ": " << baseline << std::endl;
}