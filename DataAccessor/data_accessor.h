// Class usage:
//
// To use this class, follow these steps:
//  1. Instantiate.
//     Ex:
//       DataAccessor d;
//  2. Load data. File must be in the compressed format described below.
//     Ex:
//       d.load_data("path/to/compressed/data/file");
//  3. To access a particular user-movie rating, use
//       entry_t e = d.get_entry(user_id, movie_id)
//     user_id and movie_id are 0-indexed. Interpretation of the entry_t datatype is
//     described below.
//  4. To iterate through all ratings, use
//       for (int i = 0; i < d.get_num_entries(); i++) {
//         entry_t e = d.get_entry(index);
//         //PROCESS e
//       }
//  5. To get all ratings for a particular user or movie, use
//     d.get_user_entries() or d.get_movie_entries().
//  6. To interpret an entry_t object, e, use any of
//       extract_movie_id(), extract_user_id(), extract_rating(), extract_date(), extract_all()
//     Note that if you are extracting all components of the entry_t object, it is slightly faster
//     to use extract_all() to extract it all at once, as opposed to extracting each component
//     separately.
//     Ex:
//       entry_t e = d.get_entry(user_id, movie_id);
//       int r = d.get_rating(e);
//       int date = d.get_date(e);
//
// This class requires that loaded data files are compressed in the following format:
//  1. A list of integers (in binary format, not strings) giving the number of movies per user.
//     Assumes 458293 users.
//  2. A list of entries in a compressed format. Each entry is a 4-byte integer E, with
//       E = (d*NUM_RATINGS + r)*NUM_MOVIES + m
//     where m is the movie ID (0-indexed), r is the rating (0 to 5), and d is the 
//     date indicator (0 to 2242).
// This compression format allows all.dta to be compressed to ~0.383GB, making it feasible
// for being kept in memory.
//

// Note that all movie_id and user_id values are 0-indexed.

#ifndef DATA_ACCESSOR_H
#define DATA_ACCESSOR_H

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <ctime>


#define MAX_USERS 458293
#define MAX_MOVIES 17770
#define NUM_RATINGS 6
#define NUM_DATES 2243
#define MAX_ENTRIES_PER_USER 3496
#define MAX_ENTRIES_PER_MOVIE 242126

#define USE_VALIDATION

typedef struct entry_compressed_t { unsigned char x[6]; } entry_compressed_t;
typedef struct entry_t { unsigned int x[5]; } entry_t;

class DataAccessor {
// private member variables
  int num_users;
  int num_movies;

  int num_entries;
  entry_compressed_t *entries;

  int *entries_per_user;
  int *user_start_indices;
  
  int *entries_per_movie;
  int *movie_start_indices;
  entry_compressed_t *entries_by_movie;

#ifdef USE_VALIDATION
  unsigned char *val_ids;
  int num_val_sets; 
#endif
  
public:
  DataAccessor(int k = 8); // default constructor (default # validation sets = 8)
  ~DataAccessor(); // destructor
  
  void load_data(char *datafile); // load data from file into memory for quick access
  
  // Access loaded data
  int get_num_entries() const;
  int get_num_users() const;
  int get_num_movies() const;
  int get_num_user_entries(int user_id) const;
  int get_num_movie_entries(int movie_id) const;
  
  bool has_entry(int user_id, int movie_id) const;
  
  // Get the rating entry associated with the (user_id, movie_id) pair
  entry_t get_entry(int user_id, int movie_id) const;
  
  // Get the index-th entry, in user-sorted-first order
  entry_t get_entry(int index) const;

  // Gets a batch of entries beginning with start_index.
  // The entries are returned in the batch array.
  // Return value is number of entries in batch. This value is
  // equal to max_entries unless the last entry was reached.
  int get_entry_batch(int start_index, int max_entries, entry_t *batch) const;
  
  // Get all entries associated with the given user_id.
  // Entries are returned in the user_entries array.
  // Return value is number of entries associated with the user.
  int get_user_entries(int user_id, entry_t *user_entries) const;
  
  // Get all entries associated with the given movie_id.
  // Entries are returned in the movie_entries array.
  // Return value is number of entries associated with the movie.
  int get_movie_entries(int movie_id, entry_t *movie_entries) const;
  
  // Get rating information from an entry_t object
  int extract_entry_index(entry_t entry) const;
  int extract_user_id(entry_t entry) const;
  int extract_movie_id(entry_t entry) const;
  int extract_rating(entry_t entry) const;
  int extract_date(entry_t entry) const;

#ifdef USE_VALIDATION
  int extract_validation_id(entry_t entry) const; // for consistency, but the validation id isn't actually stored within the entry
#endif

  void extract_all(entry_t entry, int &user_id, int &movie_id, int &rating, int &date) const;

#ifdef USE_VALIDATION
  // Validation functionality
  // After loading from a data file, each entry is associated with
  // a random number V from 0 to 255, inclusive. The particular
  // validation ID for a given entry is V (mod k).
  // The value of V is kept constant (even if k is modified) unless
  // reset_validation_ids() is called.
  //
  // An entry should be checked for its validation ID prior to being
  // used for SGD or in other code. This checking will NOT be done
  // by the DataAccessor class.

  // Set/get number of validation sets to use
  // k should be between 0 and 255, inclusive
  void set_num_validation_sets(int k);
  int get_num_validation_sets() const;

  // Get validation ID associated with the given entry
  // The returned validation ID will be between 0 and k-1, inclusive
  int get_validation_id(int index) const; // this is fastest; use if possible
  int get_validation_id(int user_id, int movie_id) const;
  int get_validation_id(entry_t entry) const;

  // Randomize validation IDs
  // Gives new random values of V for each entry
  void reset_validation_ids();
#endif
  
private:
  // Retrieves the entry value of the desired entry
  entry_compressed_t find_entry_val(int user_id, int movie_id) const;
  int find_entry_index(int user_id, int movie_id) const;
  
  // Extract data from a compressed 4-byte entry value holding movie id, rating, and date.
  int user_id_from_entry_val(entry_compressed_t entry_val) const;
  int movie_id_from_entry_val(entry_compressed_t entry_val) const;
  int rating_from_entry_val(entry_compressed_t entry_val) const;
  int date_from_entry_val(entry_compressed_t entry_val) const;
  void parse_entry_val(entry_compressed_t entry_val, int &user_id, int &movie_id, int &rating, int &date) const;
  
  // Convert between compressed and uncompressed entry values
  entry_t make_entry(int index, int user_id, int movie_id, int rating, int date) const;
  entry_t decompress_entry_val(int index, entry_compressed_t entry_val) const;
  entry_compressed_t compress_entry(entry_t entry) const;

  // Convert between compressed entry value and a long (which is slightly less compressed but easier to manipulate)
  long entry_val_to_long(entry_compressed_t entry_val) const;
  entry_compressed_t long_to_entry_val(long l) const;

  // Decompress an entry value from a compressed data file
  entry_compressed_t decompress_datafile_entry(int user_id, int e);

  
  // Get user id for given entry index
  int user_id_from_entry_index(int index) const;

  // Calculate movie-sorted information (for faster access)
  void calc_movie_info();

};

#endif //DATA_ACCESSOR_H