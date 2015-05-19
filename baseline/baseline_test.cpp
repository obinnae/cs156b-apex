#include <cmath>
#include <cstdio>
#include "baseline.h"

float max(float n1, float n2) {
  if (n1 > n2) return n1;
  else return n2;
}

float max_baseline_deviation(DataAccessor *d, Baseline *b) {
  float max_deviation = 0;
  entry_t e;
  int r, u, m;
  for (int i = 0; i < d->get_num_entries(); i++) {
    e = d->get_entry(i);
    r = d->extract_rating(e);
    u = d->extract_user_id(e);
    m = d->extract_movie_id(e);
    max_deviation = max(max_deviation, (float)fabs(r - b->get_baseline(u, m)));
  }
  return max_deviation;
}

float std_dev_baseline_deviation(DataAccessor *d, Baseline *b) {
  float total_deviation = 0;
  entry_t e;
  int r, u, m;
  for (int i = 0; i < d->get_num_entries(); i++) {
    e = d->get_entry(i);
    r = d->extract_rating(e);
    u = d->extract_user_id(e);
    m = d->extract_movie_id(e);
    float error = r - b->get_baseline(u, m);
    total_deviation += error*error;
  }
  return sqrt(total_deviation / d->get_num_entries());
}

float average_baseline_deviation(DataAccessor *d, Baseline *b) {
  float total_deviation = 0;
  entry_t e;
  int r, u, m;
  for (int i = 0; i < d->get_num_entries(); i++) {
    e = d->get_entry(i);
    r = d->extract_rating(e);
    u = d->extract_user_id(e);
    m = d->extract_movie_id(e);
    total_deviation += (r - b->get_baseline(u, m));
  }
  return total_deviation / d->get_num_entries();
}

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

  std::cout << "Average baseline deviation: " << average_baseline_deviation(&d, &b) << std::endl;
  std::cout << "Standard deviation of baseline deviation: " << std_dev_baseline_deviation(&d, &b) << std::endl;
  std::cout << "Maximum baseline deviation: " << max_baseline_deviation(&d, &b) << std::endl;

}