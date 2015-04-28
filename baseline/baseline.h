// Describes the Baseline class, which allows for easy calculation of baseline
// rating estimates.
//
// To construct an instance of this class, you must give as input a pointer to a DataAccessor, 
// which will be used to calculate the baseline rating for a given movie-user pair.
//
// To speed usage, movie and user averages are not computed until necessary. However, once
// computed, they are stored to save time later. This means that the first request for a baseline
// involving a particular movie-user pair may take longer than later requests.
//
// Class usage:
//
// To use this class, follow these steps:
//  1. Instantiate. Supply a pointer to a DataAccessor object, either as the constructor argument
//     or with the set_data function.
//     Ex:
//       Baseline b(data_accessor_ptr);
//         OR
//       Baseline b;
//       b.set_data(data_accessor_ptr);
//  2. Request a particular baseline by supplying a user_id and movie_id.
//     user_id and movie_id are 0-indexed (i.e. range from 0 to NUM_USERS-1 or NUM_MOVIES-1, respectively)
//     Ex:
//       d.get_baseline(user_id, movie_id);
//
// Note that data should be loaded into the DataAccessor object prior to requesting any baselines.
// However, it is not necessary to load the data before instantiation.

#ifndef BASELINE_H
#define BASELINE_H

#include <iostream>
#include "../DataAccessor/data_accessor.h"

class Baseline {
// private member variables
  const DataAccessor *data;
  
  bool calculated_user_avg[NUM_USERS];
  bool calculated_movie_avg[NUM_MOVIES];

  float *user_avgs;
  float *movie_avgs;
  
public:
  //// Constructors and destructors
  
  // Constructs a Baseline object for the given DataAccessor object.
  // If d is not supplied, then you should call set_data() before
  // attempted to calculate baselines.
  Baseline(const DataAccessor *d = NULL);
  
  // destructor
  ~Baseline();
  
  //// General usage
  
  // Sets the associated DataAccessor object. This can be done at any time.
  // This removes all stored user/movie averages.
  void set_data(const DataAccessor *d);
  
  // Returns the baseline rating for the given user_id and movie_id.
  // The baseline is the average of the means for the user and movie.
  // user_id ranges from 0 to NUM_USERS-1.
  // movie_id ranges from 0 to NUM_MOVIES-1.
  float get_baseline(int user_id, int movie_id);
  
private:
  // Calculate user or movie average from the DataAccessor object.
  // The result is stored in the corresponding user_avgs or movie_avgs array
  // and the associated flag is set to indicate the average has been calculated
  // and stored.
  void compute_user_average(int user_id);
  void compute_movie_average(int movie_id);
  float calc_average_rating(entry_t *entries, int num_entries);
  
  // Clears all averages.
  // Performed after setting the DataAccessor object, so that any future
  // requests for baselines will use the user/movie averages corresponding
  // to the new data set.
  void clear_averages();
  
};

#endif //BASELINE_H