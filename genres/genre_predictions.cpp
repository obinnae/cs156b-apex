#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <cmath>

#include "../baseline/baseline.h"
#include "../DataAccessor/data_accessor.h"

bool has_correlation(float *corr_matrix, int m1, int m2) {
  int idx = m1 * MAX_MOVIES + m2;
  float c = corr_matrix[idx];

  return (!std::isnan(c) && !std::isinf(c));
}


void initialize_matrix(float *factor_matrix, float *lambdas, int num_factors) {
  for (int m = 0; m < MAX_MOVIES; m++) {
    float norm = 0;
    float *row = factor_matrix + m * num_factors;
    for (int f = 0; f < num_factors; f++) {
      float val = static_cast<float>(rand()) / RAND_MAX * 2 - 1;
      row[f] = val;
      norm += val * val;
    }
    norm = sqrt(norm);
    // make sure each latent factor vector is a unit vector
    for (int f = 0; f < num_factors; f++) {
      row[f] /= norm;
    }

    lambdas[m] = (static_cast<float>(rand()) / RAND_MAX * 2 - 1) * 0.001;
  }
}

float calc_lambda_gradient(float *factor_matrix, int num_factors, int i) {
  float norm;
  float *factor_row;

  factor_row = factor_matrix + i * num_factors;

  norm = 0;
  for (int f = 0; f < num_factors; f++)
    norm += factor_row[f] * factor_row[f];

  return 0.5 * (norm - 1);
}

void calc_factor_gradient(float *corr_matrix, float *factor_matrix, float *lambdas, int num_factors, int i, int j, float *i_gradient, float *j_gradient) {
  float error;
  float *i_row, *j_row;

  i_row = factor_matrix + i * num_factors;
  j_row = factor_matrix + j * num_factors;

  // Calculate difference between correlation value and the prediction
  error = corr_matrix[i * MAX_MOVIES + j];
  for (int f = 0; f < num_factors; f++)
    error -= i_row[f] * j_row[f];

  for (int f = 0; f < num_factors; f++) {
    i_gradient[f] = lambdas[i] * i_row[f] - error * j_row[f];
    j_gradient[f] = lambdas[j] * j_row[f] - error * i_row[f];
  }
}

void run_epoch(float *corr_matrix, float *factor_matrix, float *lambdas, int num_factors, float lrate) {
  float *i_gradient = new float[num_factors];
  float *j_gradient = new float[num_factors];

  int count = 0;
  for (int m1 = 0; m1 < MAX_MOVIES; m1++) {
    for (int m2 = m1; m2 < MAX_MOVIES; m2++) {
      if (!has_correlation(corr_matrix, m1, m2)) continue;

      calc_factor_gradient(corr_matrix, factor_matrix, lambdas, num_factors, m1, m2, i_gradient, j_gradient);
      float lambda1_grad = calc_lambda_gradient(factor_matrix, num_factors, m1);
      float lambda2_grad = calc_lambda_gradient(factor_matrix, num_factors, m2);

      for (int f = 0; f < num_factors; f++) {
        factor_matrix[m1 * num_factors + f] -= lrate * i_gradient[f];
        factor_matrix[m2 * num_factors + f] -= lrate * j_gradient[f];
      }
      lambdas[m1] -= lrate * lambda1_grad;
      lambdas[m2] -= lrate * lambda2_grad;

      count++;

      if (count % 15000000 == 0)
        std::cout << "Processed " << count << " correlation values. Lambda = " << lambdas[m1] << " and " << lambdas[m2] << ". Lambda gradients: " << lambda1_grad << " and " << lambda2_grad << "\n";
    }

  }

  delete[] i_gradient;
  delete[] j_gradient;
}

float calc_error(float *corr_matrix, float *factor_matrix, int num_factors) {
  float total_error = 0;

  int count = 0;
  for (int m1 = 0; m1 < MAX_MOVIES; m1++) {
    for (int m2 = m1; m2 < MAX_MOVIES; m2++) {
      if (!has_correlation(corr_matrix, m1, m2)) continue;

      float error;
      float pred = 0;
      for (int f = 0; f < num_factors; f++)
        pred += factor_matrix[m1 * num_factors + f] * factor_matrix[m2 * num_factors + f];

      error = pred - corr_matrix[m1 * MAX_MOVIES + m2];
      total_error += error * error;
      count++;

      if (count % 15000000 == 0)
        std::cout << "Error over " << count << " correlation values: " << sqrt(total_error / count) << std::endl;
    }
  }

  total_error /= count;
  total_error = sqrt(total_error);
  return total_error;
}

void do_matrix_factorization(float *corr_matrix, float *factor_matrix, int num_factors, float lrate, int num_epochs) {
  float *lambdas = new float[MAX_MOVIES];

  std::cout << "Initializing factor matrix...\n";
  initialize_matrix(factor_matrix, lambdas, num_factors);

  std::cout << "Performing matrix factorization\n";
  for (int e = 0; e < num_epochs; e++) {
    time_t t1 = time(NULL);
    run_epoch(corr_matrix, factor_matrix, lambdas, num_factors, lrate);

    float error = calc_error(corr_matrix, factor_matrix, num_factors);
    time_t t2 = time(NULL);
    
    std::cout << "Epoch time: " << difftime(t2, t1) << " seconds\n";

    std::cout << "Epoch " << e << " complete: RMSE = " << error << std::endl;
  }

  delete[] lambdas;
}


void calc_user_prefs(float *factor_matrix, float *user_prefs, int num_factors, DataAccessor *train_data, Baseline *b) {
  float *user_totals = new float[MAX_USERS * num_factors];

  for (int i = 0; i < MAX_USERS * num_factors; i++) {
    user_prefs[i] = 0;
    user_totals[i] = 0;
  }
  
  for (int i = 0; i < train_data->get_num_entries(); i++) {
    entry_t e = train_data->get_entry(i);
    int user_id, movie_id;
    float r;

    user_id = train_data->extract_user_id(e);
    movie_id = train_data->extract_movie_id(e);
    r = train_data->extract_rating(e) - b->get_baseline(user_id, movie_id);

    for (int f = 0; f < num_factors; f++) {
      user_prefs[user_id * num_factors + f] += r * factor_matrix[movie_id * num_factors + f];
      user_totals[user_id * num_factors + f] += factor_matrix[movie_id * num_factors + f];
    }
  }

  for (int i = 0; i < MAX_USERS * num_factors; i++)
    user_prefs[i] /= user_totals[i];
  
  delete[] user_totals;
}

void calc_predictions(float *factor_matrix, float *user_prefs, int num_factors, DataAccessor *data, Baseline *b, char *outfile) {
  std::ofstream out(outfile);

  for (int i = 0; i < data->get_num_entries(); i++) {
    entry_t e = data->get_entry(i);
    int user_id, movie_id;

    user_id = data->extract_user_id(e);
    movie_id = data->extract_movie_id(e);

    float pred = 0;
    for (int f = 0; f < num_factors; f++)
      pred += factor_matrix[movie_id * num_factors + f] * user_prefs[user_id * num_factors + f];
    pred += b->get_baseline(user_id, movie_id);

    if (pred > 5) pred = 5;
    if (pred < 1) pred = 1;	 

    out << pred << std::endl;
  }
  out.close();
}


void read_correlation_file(char *file, float *correlation_matrix) {
  std::ifstream in(file);

  for (int i = 0; i < MAX_MOVIES; i++) {
    for (int j = i; j < MAX_MOVIES; j++) {
      int idx = i * MAX_MOVIES + j;
      in.read(reinterpret_cast<char*>(correlation_matrix + idx), 4);
      correlation_matrix[j * MAX_MOVIES + i] = correlation_matrix[idx];
    }
  }
  in.close();
}

void genre_predictions(float *corr_matrix, int num_factors, float lrate, int num_epochs, DataAccessor *train_data, DataAccessor *probe_data, DataAccessor *qual_data, Baseline *b, char *probe_out, char *qual_out) {
  float *factor_matrix = new float[MAX_MOVIES * num_factors];
  float *user_prefs = new float[MAX_USERS * num_factors];

  do_matrix_factorization(corr_matrix, factor_matrix, num_factors, lrate, num_epochs);

  std::cout << "Calculating user preferences\n";
  calc_user_prefs(factor_matrix, user_prefs, num_factors, train_data, b);

  std::cout << "Outputting probe predictions\n";
  calc_predictions(factor_matrix, user_prefs, num_factors, probe_data, b, probe_out);

  std::cout << "Outputting qual predictions\n";
  calc_predictions(factor_matrix, user_prefs, num_factors, qual_data, b, qual_out);

  delete[] factor_matrix;
  delete[] user_prefs;
}

int main(int argc, char *argv[]) {
  char *correlation_file;
  int num_factors;
  float lrate;
  int num_epochs;
  char *train_infile, *probe_infile, *qual_infile;
  char *probe_outfile, *qual_outfile;
  
  if (argc != 10) {
    std::cout << "Usage: genre_predictions <correlation-file> <num-factors> <lrate> <num-epochs> <train-data> <probe-data> <qual-data> <probe-output> <qual_output>\n";
    /*
     * Modified usage message to clarify the extra command line arg
     * Also renamed old data-file arg to differentiate between train-data and probe-data
     */
    exit(1);
  }
  correlation_file = argv[1];
  num_factors = atoi(argv[2]);
  lrate = atof(argv[3]);
  num_epochs = atoi(argv[4]);
  train_infile = argv[5];
  probe_infile = argv[6];
  qual_infile = argv[7];
  probe_outfile = argv[8];
  qual_outfile = argv[9];

  std::cout << "Factoring movie correlation matrix:\n"
      << "\tData file: " << correlation_file << std::endl
      << "\tNumber of factors: " << num_factors << std::endl
      << "\tLearning rate: " << lrate << std::endl
      << "\tNumber of epochs: " << num_epochs << std::endl
      << "\tTrain data file: " << train_infile << std::endl
      << "\tProbe data file: " << probe_infile << std::endl
      << "\tQual data file: " << qual_infile << std::endl
      << "\tProbe output file: " << probe_outfile << std::endl
      << "\tQual output file: " << qual_outfile << std::endl;


  srand(time(NULL));


  float *correlation_matrix = new float[MAX_MOVIES * MAX_MOVIES];

  DataAccessor train_data, probe_data, qual_data;

  std::cout << "Loading data files...\n";
  train_data.load_data(train_infile);
  probe_data.load_data(probe_infile);
  qual_data.load_data(qual_infile);
  std::cout << "Loaded!\n\n";

  Baseline b(&train_data, BASELINE_USER_AVG);

  std::cout << "Loading correlation file...\n";
  read_correlation_file(correlation_file, correlation_matrix);
  std::cout << "Loaded!\n\n";

  std::cout << "Beginning genre prediction calculations\n";
  genre_predictions(correlation_matrix, num_factors, lrate, num_epochs, &train_data, &probe_data, &qual_data, &b, probe_outfile, qual_outfile);

  delete[] correlation_matrix;

}