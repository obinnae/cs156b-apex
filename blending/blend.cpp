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
#include <cstring>
#include <string>

#include "../DataAccessor/data_accessor.h"

#define MAX_FILENAME_LENGTH 100

void load_estimates(float *estimates, char *file, int num_entries) {
  std::ifstream in(file);
  
  for (int i = 0; i < num_entries; i++) {
    if (!in) {
      std::cout << "Error: Could not read enough ratings from file " << file << "!\n";
      exit(1);
    }

    in >> estimates[i];
  }
  
  in.close();
}

int get_bin_index(int entry_index, DataAccessor *d, int num_user_bins, int num_movie_bins) {
  entry_t e = d->get_entry(entry_index);
  int user_id = d->extract_user_id(e);
  int movie_id = d->extract_movie_id(e);
  int user_bin = (d->get_num_user_entries(user_id) - 1) * num_user_bins / MAX_ENTRIES_PER_USER;
  int movie_bin = (d->get_num_movie_entries(movie_id) - 1) * num_movie_bins / MAX_ENTRIES_PER_MOVIE;

  return user_bin * num_movie_bins + movie_bin;
}

void blend_new_estimates(float *old_estimates, float *new_estimates, DataAccessor *data, int num_user_bins, int num_movie_bins, float *bin_weights) {
  // Returns the optimal weight to be put on the old estimates
  float *numerator, *denominator;

  numerator = new float[num_user_bins * num_movie_bins];
  denominator = new float[num_user_bins * num_movie_bins];
  memset(numerator, 0, num_user_bins * num_movie_bins * sizeof(float));
  memset(denominator, 0, num_user_bins * num_movie_bins * sizeof(float));
  for (int i = 0; i < data->get_num_entries(); i++) {
    int bin_index = get_bin_index(i, data, num_user_bins, num_movie_bins);

    int r = data->extract_rating(data->get_entry(i));
    float r_est1 = old_estimates[i];
    float r_est2 = new_estimates[i];

    numerator[bin_index] += (r - r_est2) * (r_est1 - r_est2);
    denominator[bin_index] += (r_est1 - r_est2) * (r_est1 - r_est2);
  }

  for (int i = 0; i < num_user_bins * num_movie_bins; i++)
    bin_weights[i] = numerator[i] / denominator[i];
}

void calc_blend_weights(int num_files, char **blend_files, DataAccessor *probe_data, int num_user_bins, int num_movie_bins, DataAccessor *train_data, float **bin_weights) {
  int probe_size;

  probe_size = probe_data->get_num_entries();

  float *old_estimates = new float[probe_size];
  float *new_estimates = new float[probe_size];

  float *old_weights = new float[num_user_bins * num_movie_bins];

  for (int i = 0; i < probe_size; i++) old_estimates[i] = 0;

  for (int i = 0; i < num_files; i++) {
    load_estimates(new_estimates, blend_files[i], probe_size);

    blend_new_estimates(old_estimates, new_estimates, probe_data, num_user_bins, num_movie_bins, old_weights);

    std::cout << "Weights calculated for " << blend_files[i] << ": " << std::endl;
    for (int bin_idx = 0; bin_idx < num_user_bins * num_movie_bins; bin_idx++) {
      std::cout << "(" << (bin_idx / num_user_bins) << ", " << (bin_idx % num_movie_bins) << "): " << (1-old_weights[bin_idx]) << std::endl;
    }

    for (int idx = 0; idx < probe_size; idx++) {
      int bin_idx = get_bin_index(idx, probe_data, num_user_bins, num_movie_bins);
      old_estimates[idx] = old_estimates[idx] * old_weights[bin_idx] + new_estimates[idx] * (1 - old_weights[bin_idx]);
    }
    std::cout << "Updated rating estimates with weight\n";

    for (int bin_idx = 0; bin_idx < num_user_bins * num_movie_bins; bin_idx++) {
      float *weights = bin_weights[bin_idx];
      for (int j = 0; j < i; j++)
        weights[j] *= old_weights[bin_idx];
      weights[i] = 1 - old_weights[bin_idx];
    }
  }

  delete[] old_weights;
  delete[] old_estimates;
  delete[] new_estimates;
}

void perform_blend(int num_files, char **blend_files, float **bin_weights, DataAccessor *qual_data, int num_user_bins, int num_movie_bins, char *output_file) {
  std::ifstream *ins = new std::ifstream[num_files];
  std::ofstream out;

  for (int i = 0; i < num_files; i++)
    ins[i].open(blend_files[i]);
  out.open(output_file);

  for (int idx = 0; idx < qual_data->get_num_entries(); idx++) {
    int bin_index = get_bin_index(idx, qual_data, num_user_bins, num_movie_bins);
    float *weights = bin_weights[bin_index];

    float r = 0;

    float val;
    for (int i = 0; i < num_files; i++) {
      if (!ins[i]) {
        std::cout << "Error: Could not read enough ratings from file " << blend_files[i] << "!\n";
        exit(1);
      }

      ins[i] >> val;
      r += val * weights[i];
    }

    out << r << std::endl;
  }

  for (int i = 0; i < num_files; i++)
    ins[i].close();
  out.close();
}

void load_blend_list(char *blend_list_file, char **probe_files, char **qual_files, int num_blends) {
  std::ifstream in(blend_list_file);

  std::string probe_str, qual_str;
  for (int i = 0; i < num_blends; i++) {
    if (!in) {
      std::cout << "Could not load " << num_blends << " sets of files to blend from " << blend_list_file << std::endl;
      exit(1);
    }

    getline(in, probe_str, ' ');
    getline(in, qual_str);

    strcpy(probe_files[i], probe_str.c_str());
    strcpy(qual_files[i], qual_str.c_str());

    std::cout << probe_files[i] << " " << qual_files[i] << std::endl;
  }
  in.close();
}

int main(int argc, char *argv[]) {
  int num_user_bins, num_movie_bins;
  char *output_file;
  char *train_entry_file, *probe_entry_file, *qual_entry_file;
  char *blend_list_file;
  char **probe_files;
  char **qual_files;
  int num_blends;
  
  if (argc != 9) {
    std::cout << "Usage: blend <user-bins> <movie-bins> <output-file> <train-entries> <probe-entries> <qual-entries> <num_blends> <blend-list-file>\n";
    exit(1);
  }
  num_user_bins = atoi(argv[1]);
  num_movie_bins = atoi(argv[2]);
  output_file = argv[3];
  train_entry_file = argv[4];
  probe_entry_file = argv[5];
  qual_entry_file = argv[6];
  num_blends = atoi(argv[7]);
  blend_list_file = argv[8];

  std::cout << "Blending " << num_blends << " files with the following parameters:\n"
      << "\tOutput file: " << output_file << std::endl
      << "\tTraining data file (for binning): " << train_entry_file << std::endl
      << "\tProbe data file (for calculating blend): " << probe_entry_file << std::endl
      << "\tQual data file (for performing blend): " << qual_entry_file << std::endl
      << "\tNumber of user bins: " << num_user_bins << std::endl
      << "\tNumber of movie bins: " << num_movie_bins << std::endl
      << "\tNumber of blends: " << num_blends << std::endl;

  probe_files = new char*[num_blends];
  qual_files = new char*[num_blends];
  for (int i = 0; i < num_blends; i++) {
    probe_files[i] = new char[MAX_FILENAME_LENGTH];
    qual_files[i] = new char[MAX_FILENAME_LENGTH];
  }
  load_blend_list(blend_list_file, probe_files, qual_files, num_blends);

  DataAccessor train_data, probe_data, qual_data;

  train_data.load_data(train_entry_file);
  probe_data.load_data(probe_entry_file);
  qual_data.load_data(qual_entry_file);

  float **bin_weights = new float*[num_user_bins * num_movie_bins];
  for (int bin_idx = 0; bin_idx < num_user_bins * num_movie_bins; bin_idx++)
    bin_weights[bin_idx] = new float[num_blends];

  calc_blend_weights(num_blends, probe_files, &probe_data, num_user_bins, num_movie_bins, &train_data, bin_weights);

  std::cout << "Calculated weights: \n";
  for (int bin_idx = 0; bin_idx < num_user_bins * num_movie_bins; bin_idx++) {
    int user_bin = bin_idx / num_movie_bins;
    int movie_bin = bin_idx % num_movie_bins;
    std::cout << "Bin (" << user_bin << ", " << movie_bin << "):\n";
    for (int i = 0; i < num_blends; i++)
      std::cout << bin_weights[bin_idx][i] << "\t" << probe_files[i] << std::endl;
  }

  perform_blend(num_blends, qual_files, bin_weights, &qual_data, num_user_bins, num_movie_bins, output_file);

  for (int bin_idx = 0; bin_idx < num_user_bins * num_movie_bins; bin_idx++)
    delete[] bin_weights[bin_idx];
  delete[] bin_weights;

  for (int i = 0; i < num_blends; i++) {
    delete[] probe_files[i];
    delete[] qual_files[i];
  }
  delete[] probe_files;
  delete[] qual_files;

}
