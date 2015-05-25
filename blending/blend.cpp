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
    std::cout << estimates[i] << std::endl;
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

void calc_blend_weights(int num_files, char **blend_files, DataAccessor *probe_data, int num_bins, DataAccessor *train_data, float *weights) {
  int probe_size;

  probe_size = probe_data->get_num_entries();

  float *old_estimates = new float[probe_size];
  float *new_estimates = new float[probe_size];

  for (int i = 0; i < probe_size; i++) old_estimates[i] = 0;

  for (int i = 0; i < num_files; i++) {
    load_estimates(new_estimates, blend_files[i], probe_size);

    float old_weight = blend_new_estimates(old_estimates, new_estimates, probe_data);

    std::cout << "Weight calculated for " << blend_files[i] << ": " << (1-old_weight) << std::endl;

    for (int j = 0; j < i; j++)
      weights[j] *= old_weight;
    weights[i] = 1 - old_weight;
  }

  delete[] old_estimates;
  delete[] new_estimates;
}

void perform_blend(int num_files, char **blend_files, float *weights, DataAccessor *qual_data, int num_bins, char *output_file) {
  std::ifstream *ins = new std::ifstream[num_files];
  std::ofstream out;

  for (int i = 0; i < num_files; i++)
    ins[i].open(blend_files[i]);
  out.open(output_file);

  for (int idx = 0; idx < qual_data->get_num_entries(); idx++) {
    float r = 0;

    float val;
    for (int i = 0; i < num_files; i++) {
      ins[i] >> val;
      r += val * weights[i];
    }

    out << r << std::endl;
  }

  for (int i = 0; i < num_files; i++)
    ins[i].close();
  out.close();
}


int main(int argc, char *argv[]) {
  int num_bins;
  char *output_file;
  char *train_entry_file, *probe_entry_file, *qual_entry_file;
  char **probe_files;
  char **qual_files;
  int num_blends;
  
  if (argc < 8) {
    std::cout << "Usage: blend <#-bins> <output-file> <train-entries> <probe-entries> <qual-entries> <probe_1> [<probe_2> ...] <qual_1> [<qual_2> ...]\n";
    exit(1);
  }
  num_blends = (argc - 6) / 2;
  num_bins = atoi(argv[1]);
  output_file = argv[2];
  train_entry_file = argv[3];
  probe_entry_file = argv[4];
  qual_entry_file = argv[5];
  probe_files = argv + 6;
  qual_files = probe_files + num_blends;

  std::cout << "Blending " << num_blends << " files with the following parameters:\n"
      << "\tOutput file: " << output_file << std::endl
      << "\tTraining data file (for binning): " << train_entry_file << std::endl
      << "\tProbe data file (for calculating blend): " << probe_entry_file << std::endl
      << "\tQual data file (for performing blend): " << qual_entry_file << std::endl
      << "\tNumber of bins: " << num_bins << std::endl
      << "\tNumber of blends: " << num_blends << std::endl;

  DataAccessor train_data, probe_data, qual_data;

  train_data.load_data(train_entry_file);
  probe_data.load_data(probe_entry_file);
  qual_data.load_data(qual_entry_file);

  float *weights = new float[num_blends];
  calc_blend_weights(num_blends, probe_files, &probe_data, num_bins, &train_data, weights);

  std::cout << "Calculated weights: \n";
  for (int i = 0; i < num_blends; i++)
    std::cout << weights[i] << "\t" << probe_files[i] << std::endl;

  perform_blend(num_blends, qual_files, weights, &qual_data, num_bins, output_file);

  delete[] weights;

}
