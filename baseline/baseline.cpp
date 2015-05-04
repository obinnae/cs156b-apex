#include "baseline.h"


//// Public member functions

// Constructs a Baseline object for the given DataAccessor object.
// If d is not supplied, then you should call set_data() before
// attempted to calculate baselines.
// Default value for d is NULL.
Baseline::Baseline(const DataAccessor *d, int t) {
  // Allocate dummy space for the arrays
  // This is just so set_data can delete them and replace them with the proper sizes

  // Allocate space for precomputed user and movie averages    
  user_avgs = new float[MAX_USERS];
  movie_avgs = new float[MAX_MOVIES];

  // Set flags to indicate what information must be computed
  set_type(t);

  set_data(d);
}

// destructor
Baseline::~Baseline() {
  delete[] user_avgs;
  delete[] movie_avgs;
}


// Sets the type of baseline to be computed. This can be performed at any time.
// <type> must be one of BASELINE_STANDARD, BASELINE_ZERO, BASELINE_USER_AVG, or BASELINE_MOVIE_AVG
void set_type(int t) {
  type = t;

  switch(t) {
    case BASELINE_STANDARD:
      use_user_avg = true;
      use_movie_avg = true;
      break;
    case BASELINE_ZERO:
      use_user_avg = false;
      use_movie_avg = false;
      break;
    case BASELINE_MOVIE_AVG:
      use_user_avg = false;
      use_movie_avg = true;
      break;
    case BASELINE_USER_AVG:
      use_user_avg = true;
      use_movie_avg = false;
    case default:
      std::cout << "Invalid base type given; assuming BASELINE_STANDARD.\n";
      set_type(BASELINE_STANDARD);
  }
}


// Sets the associated DataAccessor object. This can be done at any time.
// This removes all stored user/movie averages.
void Baseline::set_data(const DataAccessor *d) {
  data = d;
  
  clear_averages();
}


// Returns the baseline rating for the given user_id and movie_id.
// The baseline is the average of the means for the user and movie.
// user_id ranges from 0 to NUM_USERS-1.
// movie_id ranges from 0 to NUM_MOVIES-1.
float Baseline::get_baseline(int user_id, int movie_id) {
  if (!data) {
    std::cout << "You must use set_data() to supply a DataAccessor object before requesting baselines!\n";
    return 0;
  }
  if (user_id < 0 || user_id >= data->get_num_users()
      || movie_id < 0 || movie_id >= data->get_num_movies()) {
    std::cout << "Number of users: " << data->get_num_users() << "\nNumber of movies: " << data->get_num_movies() << std::endl;
    std::cout << "Invalid (user_id, movie_id) pair: (" << user_id << ", " << movie_id << ")\n";
    return 0;
  }
//  std::cout << "Accessing baseline for (" << user_id << ", " << movie_id << ")\n";
  
  // Calculate user/movie averages, if necessary
  if (use_user_avg && !calculated_user_avg[user_id])
    compute_user_average(user_id);
  if (use_movie_avg && !calculated_movie_avg[movie_id])
    compute_movie_average(movie_id);
    
  // Calculate and return baseline value
  float baseline;
  switch (type) {
    case BASELINE_STANDARD:
      baseline = (user_avgs[user_id] + movie_avgs[movie_id])/2;
    case BASELINE_ZERO:
      baseline = 0;
    case BASELINE_USER_AVG:
      baseline = user_avgs[user_id];
    case BASELINE_MOVIE_AVG:
      baseline = movie_avgs[movie_id];

  return baseline;
}



//// Private member functions

// Calculate user or movie average from the DataAccessor object.
// The result is stored in the corresponding user_avgs or movie_avgs array
// and the associated flag is set to indicate the average has been calculated
// and stored.
void Baseline::compute_user_average(int user_id) {
  entry_t *entries = new entry_t[MAX_ENTRIES_PER_USER];
  int num_entries;
  
  num_entries = data->get_user_entries(user_id, entries);
  
  user_avgs[user_id] = calc_average_rating(entries, num_entries);
  calculated_user_avg[user_id] = true;
  
  delete[] entries;
  
//  std::cout << "Average for user " << user_id << " is " << user_avgs[user_id] << std::endl;
}
void Baseline::compute_movie_average(int movie_id) {
  entry_t *entries = new entry_t[MAX_ENTRIES_PER_MOVIE];
  int num_entries;
  
  num_entries = data->get_movie_entries(movie_id, entries);

  movie_avgs[movie_id] = calc_average_rating(entries, num_entries);
  calculated_movie_avg[movie_id] = true;
  
  delete[] entries;
  
//  std::cout << "Average for movie " << movie_id << " is " << movie_avgs[movie_id] << std::endl;
}
float Baseline::calc_average_rating(entry_t *entries, int num_entries) {
  int rating;
  
  int total = 0;
  int num_ratings = 0;
  for (int i = 0; i < num_entries; i++) {
    rating = data->extract_rating(entries[i]);
    if (rating != 0) {
      total += rating;
      num_ratings++;
    }
  }
  
  if (num_ratings == 0) {
    std::cout << "Couldn't calculate the average because there were no ratings!\n";
    return 0;
  }
  
  return static_cast<float>(total) / num_ratings;
}

// Clears all averages.
// Performed after setting the DataAccessor object, so that any future
// requests for baselines will use the user/movie averages corresponding
// to the new data set.
void Baseline::clear_averages() {
  // Reset all flags to indicate no averages are currently precomputed
  for (int i = 0; i < MAX_USERS; i++) calculated_user_avg[i] = false;
  for (int i = 0; i < MAX_MOVIES; i++) calculated_movie_avg[i] = false;
  
  // It is not necessary to clear the values in the user_avgs/movie_avgs arrays
  // because they will not be used since the flags are reset.
}