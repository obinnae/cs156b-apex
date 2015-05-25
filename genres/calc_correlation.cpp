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

float pearson_coef(float v1_sum, float v2_sum, float v1_norm, float v1_dot_v2, float v2_norm, int n) {
  return (v1_dot_v2 - v1_sum * v2_sum / n)
      / sqrt((v1_norm - v1_sum * v1_sum / n) * (v2_norm - v2_sum * v2_sum / n));
}

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

void calc_correlation_matrix(char *data_path, char *out_path) {

  DataAccessor d;
  d.load_data(data_path);
  
  Baseline b(&d, BASELINE_ZERO);
  //Baseline user_avgs(&d, BASELINE_USER_AVG);

  entry_t *user_entries = new entry_t[MAX_ENTRIES_PER_USER];
  entry_t e;
  
  float *v1sum = new float[MAX_MOVIES * MAX_MOVIES];
  float *v2sum = new float[MAX_MOVIES * MAX_MOVIES];
  float *v1sqsum = new float[MAX_MOVIES * MAX_MOVIES];
  float *dot_prods = new float[MAX_MOVIES * MAX_MOVIES];
  float *v2sqsum = new float[MAX_MOVIES * MAX_MOVIES];
  int *count = new int[MAX_MOVIES * MAX_MOVIES];
  
  int num_users = d.get_num_users();
  int num_movies = d.get_num_movies();

  // Declare and allocate memory for the correlation matrix
  float *correlation = new float[MAX_MOVIES * MAX_MOVIES];

  srand(time(NULL));

  time_t t1= time(NULL);

  for (int u = 0; u < num_users; u++) {
    int num_user_entries = d.get_user_entries(u, user_entries);

    int idx;
    entry_t e1, e2;
    int m1, m2;
    float r1, r2;
    for (int idx1 = 0; idx1 < num_user_entries; idx1++) {
      e1 = user_entries[idx1];
      m1 = d.extract_movie_id(e1);
      r1 = d.extract_rating(e1) - b.get_baseline(u, m1);
      for (int idx2 = idx1 + 1; idx2 < num_user_entries; idx2++) {
        e2 = user_entries[idx2];
        m2 = d.extract_movie_id(e2);
        r2 = d.extract_rating(e2) - b.get_baseline(u, m2);

        idx = idx1 * MAX_MOVIES + idx2;
        dot_prods[idx] += r1 * r2;
        v1sum[idx] += r1;
        v1sqsum[idx] += r1 * r1;
        v2sum[idx] += r2;
        v2sqsum[idx] += r2 * r2;
        count[idx]++;
      }
    }

    if (u % 10000 == 0)
      std::cout << (u+1) << " sets of user entries processed\n";
  }

  for (int m1 = 0; m1 < num_movies; m1++) {
    for (int m2 = m1 + 1; m2 < num_movies; m2++) {
      int idx = m1 * MAX_MOVIES + m2;
      correlation[idx] = pearson_coef(v1sum[idx], v2sum[idx], v1sqsum[idx], dot_prods[idx], v2sqsum[idx], count[idx]);
    }
  }

  time_t t2 = time(NULL);
  std::cout << "Calculated all correlations in " << difftime(t2, t1) << " seconds: " << (difftime(t2, t1) / (MAX_MOVIES * (MAX_MOVIES + 1)/2)) << "s/corr\n";

  std::cout << "Correlation between movies 0 and 1 is " << correlation[1] << std::endl;

  delete[] v1sum;
  delete[] v2sum;
  delete[] v1sqsum;
  delete[] dot_prods;
  delete[] v2sqsum;

}



int main(int argc, char *argv[]) {
  char *data_path, *out_path;
  
  if (argc != 3) {
    std::cout << "Usage: calc_correlation <train-data-file> <output-file-path>\n";
    /*
     * Modified usage message to clarify the extra command line arg
     * Also renamed old data-file arg to differentiate between train-data and probe-data
     */
    exit(1);
  }
  data_path = argv[1];
  out_path = argv[2];

  std::cout << "Calculating movie correlation matrix:\n"
      << "\tData file: " << data_path << std::endl
      << "\tOutput file: " << out_path << std::endl;

  calc_correlation_matrix(data_path, out_path);

  std::cout << "Movie correlation matrix output to " << out_path << "\n";

}