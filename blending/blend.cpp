/*
 * Nancy Wen
 * April 21, 2015
 *
 * This program uses the gradient calculations to update the latent factors.
 */
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <cmath>

#include "../DataAccessor/data_accessor.h"

void load_estimates(float *estimates, char *file, int num_entries) {
  std::ifstream in(file);
  
  for (int i = 0; i < num_entries; i++) {
    in >> estimates[i];
  }
  
  in.close();
}

float blend_new_estimates(float *old_estimates, float *new_estimates, DataAccessor *data) {
  // Returns the optimal weight to be put on the old estimates
  float numerator, denominator;

  for (int i = 0; i < data->get_num_entries(); i++) {
    int r = data->extract_rating(data->get_entry(i));
    float r_est1 = old_estimates[i];
    float r_est2 = new_estimates[i];

    numerator += (r - r_est2) * (r_est1 - r_est2);
    denominator += (r_est1 - r_est2) * (r_est1 - r_est2);
  }

  return numerator / denominator;
}

void calc_blend_weights(int num_files, char *blend_files[], char *probe_file, int num_bins, char *train_file, float *weights) {
  DataAccessor train_data, probe_data;
  int probe_size;

  train_data.load_data(train_file);
  probe_data.load_data(probe_file);

  probe_size = probe_data.get_num_entries();

  float *old_estimates = new float[probe_size];
  float *new_estimates = new float[probe_size];

  for (int i = 0; i < probe_size; i++) old_estimates[i] = 0;

  for (int i = 0; i < num_files; i++) {
    load_estimates(new_estimates, blend_files[i], probe_size);

    float old_weight = blend_new_estimates(old_estimates, new_estimates, probe_data);

    for (int j = 0; j < i; j++)
      weights[j] *= old_weight;
    weights[i] = 1 - old_weight;
  }

  delete[] old_estimates;
  delete[] new_estimates;
}


int main(int argc, char *argv[]) {
  char *train_file, *probe_file;
  char *blend_files[];
  int num_bins;
  int num_files;
  
  if (argc < 6) {
    std::cout << "Usage: blend <#-bins> <train-entries> <probe-entries> <file1> <file2> [<file3> ...]\n";
    exit(1);
  }
  num_bins = atoi(argv[1]);
  train_file = argv[2];
  probe_file = argv[3];
  blend_files = argv + 4;
  num_files = argc - 4;

  std::cout << "Blending " << num_files << " files with the following parameters:\n"
      << "\tTraining data file (for binning): " << data_path << std::endl
      << "\tProbe data file (for blending): " << num_factors << std::endl
      << "\tNumber of bins: " << num_bins << std::endl;

  float *weights = new float[num_files];
  calc_blend_weights(num_files, blend_files, probe_file, num_bins, train_file, weights);

  std::cout << "Calculated weights: \n";
  for (int i = 0; i < num_files; i++)
    std::cout << weights[i] << "\t" << blend_files[i] << std::endl;

  delete[] weights;

}
