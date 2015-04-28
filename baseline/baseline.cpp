#include "baseline.h"


//// Public member functions

// Constructs a Baseline object for the given DataAccessor object.
// If d is not supplied, then you should call set_data() before
// attempted to calculate baselines.
// Default value for d is NULL.
Baseline::Baseline(const DataAccessor *d) {
  data = d;

  // Allocate space for precomputed user and movie averages
  user_avgs = new float[NUM_USERS];
  movie_avgs = new float[NUM_MOVIES];
}

// destructor
Baseline::~Baseline() {
  delete[] user_avgs;
  delete[] movie_avgs;
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
  
  if (!calculated_user_avg[user_id])
    compute_user_average(user_id);
  if (!calculated_movie_avg[movie_id])
    compute_movie_average(movie_id);
    
  return (user_avgs[user_id] + movie_avgs[movie_id])/2;
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
  
  std::cout << "Average for user " << (user_id+1) << " is " << user_avgs[user_id] << std::endl;
}
void Baseline::compute_movie_average(int movie_id) {
  entry_t *entries = new entry_t[MAX_ENTRIES_PER_MOVIE];
  int num_entries;
  
  num_entries = data->get_movie_entries(movie_id, entries);

  movie_avgs[movie_id] = calc_average_rating(entries, num_entries);
  calculated_movie_avg[movie_id] = true;
  
  delete[] entries;
  
  std::cout << "Average for movie " << (movie_id+1) << " is " << movie_avgs[movie_id] << std::endl;
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
  for (int i = 0; i < NUM_USERS; i++) calculated_user_avg[i] = false;
  for (int i = 0; i < NUM_MOVIES; i++) calculated_movie_avg[i] = false;
  
  // It is not necessary to clear the values in the user_avgs/movie_avgs arrays
  // because they will not be used since the flags are reset.
}