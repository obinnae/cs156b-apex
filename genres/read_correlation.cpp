#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <cmath>

#include "../baseline/baseline.h"
#include "../DataAccessor/data_accessor.h"
/*
float dot_product(float *v1, float *v2, int n) {
  float product = 0;
  for (int i = 0; i < n; i++)
    product += v1[i] * v2[i];

  return product;
}

float array_sum(float *v, int n) {
  float sum = 0;
  for (int i = 0; i < n; i++)
    sum += v[i];

  return sum;

}

float pearson_coef(float *v1, float *v2, int n) {
  float v1_sum, v2_sum;
  float v1_norm, v2_norm;
  float v1_dot_v2;

  v1_sum = array_sum(v1, n);
  v2_sum = array_sum(v2, n);
  v1_norm = dot_product(v1, v1, n);
  v2_norm = dot_product(v2, v2, n);
  v1_dot_v2 = dot_product(v1, v2, n);

  return (v1_dot_v2 - v1_sum * v2_sum / n)
      / sqrt((v1_norm - v1_sum * v1_sum / n) * (v2_norm - v2_sum * v2_sum / n));

}*/

/*float pearson_coef(float v1_sum, float v2_sum, float v1_norm, float v1_dot_v2, float v2_norm, int n) {
  return (v1_dot_v2 - v1_sum * v2_sum / n)
      / sqrt((v1_norm - v1_sum * v1_sum / n) * (v2_norm - v2_sum * v2_sum / n));
}*/

/*float calc_movie_correlation(int m1, int m2, entry_t *m1_entries, entry_t *m2_entries, DataAccessor *d, Baseline *b) {
  float m1_vals[MAX_ENTRIES_PER_MOVIE];
  float m2_vals[MAX_ENTRIES_PER_MOVIE];
  int num_m1_entries, num_m2_entries;
  int num_shared_entries = 0;

  num_m1_entries = d->get_num_movie_entries(m1);
  num_m2_entries = d->get_num_movie_entries(m2);

  int u1, u2;
  int m1_idx = 0, m2_idx = 0;
  while (m1_idx < num_m1_entries && m2_idx < num_m2_entries) {
    u1 = d->extract_user_id(m1_entries[m1_idx]);
    u2 = d->extract_user_id(m2_entries[m2_idx]);

    if (u1 < u2) {
      m1_idx++;
    } else if (u1 > u2) {
      m2_idx++;
    } else {
      m1_vals[num_shared_entries] = d->extract_rating(m1_entries[m1_idx]) - b->get_baseline(u1, m1);
      m2_vals[num_shared_entries] = d->extract_rating(m2_entries[m2_idx]) - b->get_baseline(u2, m2);
      num_shared_entries++;

      m1_idx++;
      m2_idx++;
    }
  }

  float correlation = pearson_coef(m1_vals, m2_vals, num_shared_entries);

  return correlation;

}*/

void read_correlation_file(char *file, int m1, int m2) {
  std::ifstream in(file);

  float *correlation_matrix = new float[MAX_MOVIES * MAX_MOVIES];
  for (int i = 0; i < MAX_MOVIES; i++) {
    for (int j = i; j < MAX_MOVIES; j++) {
      int idx = i * MAX_MOVIES + j;
      in.read(reinterpret_cast<char*>(correlation_matrix + idx), 4);
    }
  }

  int mov_idx = m1 * MAX_MOVIES + m2;
  return correlation_matrix[mov_idx];
}

int main(int argc, char *argv[]) {
  char *correlation_file;
  int m1, m2;
  
  if (argc != 4) {
    std::cout << "Usage: read_correlation <correlation-file> <movie1> <movie2>\n";
    /*
     * Modified usage message to clarify the extra command line arg
     * Also renamed old data-file arg to differentiate between train-data and probe-data
     */
    exit(1);
  }
  correlation_file = argv[1];
  m1 = atoi(argv[2]);
  m2 = atoi(argv[3]);

  std::cout << "Reading movie correlation matrix:\n"
      << "\tData file: " << correlation_file << std::endl
      << "\tMovie ID 1 (0-indexed): " << m1 << std::endl
      << "\tMovie ID 2 (0-indexed): " << m2 << std::endl;

  float c = read_correlation_file(correlation_file, m1, m2);

  std::cout << "Movie correlation between movies " << m1 << " and " << m2 << " is " << c << "\n";

}