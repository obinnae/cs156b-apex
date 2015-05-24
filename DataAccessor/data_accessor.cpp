#include "data_accessor.h"

// There are 4 levels of compression dealt with here. In order of compression level, they are:
//  1. data file compression:
//       Three integers (movie_id, rating, date) are compressed to a single integer with the formula
//           (date*NUM_RATINGS + rating)*num_movies + movie_id
//  2. entry_compressed_t
//       Information on a complete entry (user_id, movie_id, rating, date) are compressed into 6 bytes with the format
//           |---14 bits--||---15 bits---||-----19 bits-----|
//                 DR          movie_id         user_id
//       where DR = date*NUM_RATINGS + rating
//  3. entry_compressed_t in a long int
//       Exactly the same format as entry_compressed_t but in a numeric form for easier manipulation.
//       Uses more than 6 bytes because long ints are bigger.
//  4. entry_t
//       Information on a complete entry in a five-int array. Takes 20 bytes.
//       Stores the user_id, movie_id, rating, date, and entry index for fast access.
//       Quickest to access entry information but least space-efficient.


DataAccessor::DataAccessor(int k) {
  num_entries = 0;
  num_users = 0;
  num_movies = 0;
  
  entries_per_user = new int[1];
  user_start_indices = new int[1];
  entries = new entry_compressed_t[1];
  
  entries_per_movie = new int[1];
  movie_start_indices = new int[1];
  movie_entry_indices = new int[1];

  val_ids = new unsigned char[1];
  num_val_sets = k;
}

DataAccessor::~DataAccessor() {
  delete[] entries_per_movie;
  delete[] movie_start_indices;
  delete[] movie_entry_indices;

  delete[] entries_per_user;
  delete[] user_start_indices;
  delete[] entries;

  delete[] val_ids;
}

// Loads data from a file compressed in the format described in data_accessor.h.
// Ask me if you have questions about how to make/access this file.
void DataAccessor::load_data(char *datafile) {
  std::ifstream input(datafile);
  
  // The first 12 bytes of the file specify the number of entries,
  // number of users, and number of movies in the data file.
  // Load number of entries, users, and movies in data file
  input.read(reinterpret_cast<char*>(&num_entries), sizeof(int));
  input.read(reinterpret_cast<char*>(&num_users), sizeof(int));
  input.read(reinterpret_cast<char*>(&num_movies), sizeof(int));
  
  // Load number of entries per user from data file
  delete[] entries_per_user;
  entries_per_user = new int[num_users];
  input.read(reinterpret_cast<char*>(entries_per_user), num_users*sizeof(int));
  
  // Calculate index of the first entry for each user in the entries array
  // This is for efficiency purposes later
  delete[] user_start_indices;
  user_start_indices = new int[num_users];
  user_start_indices[0] = 0;
  for (int i = 1; i < num_users; i++) {
    user_start_indices[i] = user_start_indices[i-1] + entries_per_user[i-1];
  }

  // Load the entry data from the data file and convert it entry_compressed_t values
  int *entries_temp = new int[num_entries];
  input.read(reinterpret_cast<char*>(entries_temp), num_entries*sizeof(int));

  delete[] entries;
  entries = new entry_compressed_t[num_entries];
  for (int user_id = 0; user_id < num_users; user_id++) {
    for (int i = 0; i < entries_per_user[user_id]; i++) {
      int index = user_start_indices[user_id] + i;
      entries[index] = decompress_datafile_entry(user_id, entries_temp[index]);
    }
  }
  delete[] entries_temp;


  // Calculate movie information
  delete[] movie_entry_indices;
  delete[] movie_start_indices;
  delete[] entries_per_movie;
  
  movie_entry_indices = new int[num_entries];
  movie_start_indices = new int[num_movies];
  entries_per_movie = new int[num_movies];
  calc_movie_info();

  // Calculate validation IDs for each entry, between 0 and 255
  // When a validation ID is requested, they will be returned (mod k), where k is # validation sets
  delete[] val_ids;
  val_ids = new unsigned char[num_entries];
  reset_validation_ids();
}


//// Access loaded data

// Returns the number of entries loaded from the file
int DataAccessor::get_num_entries() const {
  return num_entries;
}
// Returns the number of users in the loaded data file
int DataAccessor::get_num_users() const {
  return num_users;
}
// Returns the number of movies in the loaded data file
int DataAccessor::get_num_movies() const {
  return num_movies;
}
// Returns the number of entries associated with the given user
int DataAccessor::get_num_user_entries(int user_id) const {
  return entries_per_user[user_id];
}
// Returns the number of entries associated with the given movie
int DataAccessor::get_num_movie_entries(int movie_id) const {
  return entries_per_movie[movie_id];
}


// Returns true if the entry has been loaded and false otherwise.
bool DataAccessor::has_entry(int user_id, int movie_id) const {
  int v = find_entry_index(user_id, movie_id);
  return v==-1;
}

// Returns the entry information for the given user_id and movie_id as an entry_t
// type return value. Use extract_XXX() to get information from this value.
// If the entry has not been loaded then all extract_XXX() calls will return -1 with this value.
entry_t DataAccessor::get_entry(int user_id, int movie_id) const {
  return get_entry(find_entry_index(user_id, movie_id));
}
entry_t DataAccessor::get_entry(int index) const {
  if (index < 0 || index >= num_entries) {
    return make_entry(-1, -1, -1, -1, -1);
  }
  
  return decompress_entry_val(index, entries[index]);
}

int DataAccessor::get_entry_batch(int start_index, int max_entries, entry_t *batch) const {
  int entries_in_batch = (max_entries < num_entries - start_index) ? max_entries : (num_entries - start_index);
  int entry_idx = start_index;
  for (int i = 0; i < entries_in_batch; i++, entry_idx++) {
    batch[i] = decompress_entry_val(entry_idx, entries[entry_idx]);
  }
  return entries_in_batch;
}

// Retrieves all entries associated with this user, placing them in the user_entries array.
// Return value is the number of user entries.
// Assumes user_entries array is large enough to hold all entries found. Note that
// the most number of entries associated with any one user is 3496.
int DataAccessor::get_user_entries(int user_id, entry_t *user_entries) const {
  int user_start = user_start_indices[user_id];
  for (int i = 0; i < entries_per_user[user_id]; i++) {
    user_entries[i] = decompress_entry_val(user_start + i, entries[user_start + i]);
  }
  
  return entries_per_user[user_id];
}

// Retrieves all entries associated with this movie, placing them in the movie_entries array.
// Return value is the number of movie entries.
// Assumes movie_entries array is large enough to hold all entries found. Note that
// the most number of entries associated with any one movie is 242126.
int DataAccessor::get_movie_entries(int movie_id, entry_t *movie_entries) const {
  int movie_start = movie_start_indices[movie_id];
  int num_entries = entries_per_movie[movie_id];
  int *entry_indices = new int[num_entries]; // it can be around 10% faster to copy over the entry indices before looping, not sure why...
  memcpy(entry_indices, movie_entry_indices, num_entries*sizeof(int));
  for (int i = 0; i < num_entries; i++) {
    movie_entries[i] = decompress_entry_val(entry_indices[i], entries[entry_indices[i]]);
  }
  delete[] entry_indices;
  
  return num_entries;
}


// Unwrap the entry_t object to get the entry index, user id, movie id, rating, or date.
// If you will extract all four pieces of information, it's slightly faster to
// extract them all at once with extract_all
int DataAccessor::extract_entry_index(entry_t entry) const {
  return entry.x[0];
}
int DataAccessor::extract_user_id(entry_t entry) const {
  return entry.x[1];
}
int DataAccessor::extract_movie_id(entry_t entry) const {
  return entry.x[2];
}
int DataAccessor::extract_rating(entry_t entry) const {
  return entry.x[3];
}
int DataAccessor::extract_date(entry_t entry) const {
  return entry.x[4];
}
int DataAccessor::extract_validation_id(entry_t entry) const {
  return get_validation_id(entry);
}
void DataAccessor::extract_all(entry_t entry, int &user_id, int &movie_id, int &rating, int &date) const {
  user_id = entry.x[1];
  movie_id = entry.x[2];
  rating = entry.x[3];
  date = entry.x[4];
}


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
void DataAccessor::set_num_validation_sets(int k) {
  num_val_sets = k;
}

int DataAccessor::get_num_validation_sets() const {
  return num_val_sets;
}

// Get validation ID associated with the given entry
// The returned validation ID will be between 0 and k-1, inclusive for a valid entry
// If no entry associated with the given user exists, then returns -1.
int DataAccessor::get_validation_id(int index) const {
  return val_ids[index] % num_val_sets;
}
int DataAccessor::get_validation_id(int user_id, int movie_id) const {
  int index = find_entry_index(user_id, movie_id);
  if (index != -1) 
    return get_validation_id(index);
  else
    return -1;
}
int DataAccessor::get_validation_id(entry_t entry) const {
  return get_validation_id(extract_entry_index(entry));
}

// Randomize validation IDs
// Gives new random values of V for each entry between 0 and 255
void DataAccessor::reset_validation_ids() {
  srand(time(NULL));
  for (int i = 0; i < num_entries; i++)
    val_ids[i] = static_cast<unsigned char>(rand() % 255);
}


//// Private member functions

// Retrieves the value of the desired entry (as an entry_compressed_t value)
// or a bad entry_compressed_t with x[5]=255 if the entry does not exist.
entry_compressed_t DataAccessor::find_entry_val(int user_id, int movie_id) const {
  int index = find_entry_index(user_id, movie_id);

  if (index != -1) {
    return entries[index];
  } else {
    entry_compressed_t e;
    e.x[3] = 255;
    return e;
  }
}

// Retrieves the index of the desired entry
// Does a modified binary search between the possible indices of the particular movie rating.
// If the entry is found, returns the entry index. Otherwise, returns -1.  
int DataAccessor::find_entry_index(int user_id, int movie_id) const {
  int min_index, max_index;
  int guess;
  int entry_movie_id;
  
  min_index = user_start_indices[user_id];
  max_index = (user_id<num_users-1) ? (user_start_indices[user_id + 1]-1) : (num_entries-1);
  guess = movie_id * (max_index - min_index) / num_movies + min_index;
  while (min_index <= max_index) {
    
    entry_movie_id = movie_id_from_entry_val(entries[guess]);
     
    if (entry_movie_id < movie_id) { // too early, adjust min_index
      min_index = guess + 1;
    } else if (entry_movie_id > movie_id) { // too far, adjust max_index
      max_index = guess - 1;
    } else { // movie rating found!
      return guess;
    }
    
    guess = movie_id * (max_index - min_index) / num_movies + min_index;
  }
  return -1; // movie rating not found
}


// Extract movie, rating, and date information from a compressed 6-byte entry value.
// The entry value is calculated with the formula
//    E = ((d*NUM_RATINGS + r)*num_movies + m)*num_users + u
int DataAccessor::user_id_from_entry_val(entry_compressed_t entry_val) const {
  // NOTE: takes about 13ns each time
  return ((entry_val.x[2]<<16) + (entry_val.x[1]<<8) + entry_val.x[0]) & 0x7FFFF;
}
int DataAccessor::movie_id_from_entry_val(entry_compressed_t entry_val) const {
  // NOTE: takes about 14ns each time
  return (((entry_val.x[4]<<16) + (entry_val.x[3]<<8) + entry_val.x[2]) >> 3) & 0x7FFF;
}
int DataAccessor::rating_from_entry_val(entry_compressed_t entry_val) const {
  // NOTE: takes between 20ns each time
  return (((entry_val.x[5]<<8) + entry_val.x[4]) >> 2) % NUM_RATINGS;
}
int DataAccessor::date_from_entry_val(entry_compressed_t entry_val) const {
  // NOTE: takes about 17ns to perform this once
  return (((entry_val.x[5]<<8) + entry_val.x[4]) >> 2) / NUM_RATINGS + 1;
} 
void DataAccessor::parse_entry_val(entry_compressed_t entry_val, int &user_id, int &movie_id, int &rating, int &date) const {
  // NOTE: takes about 27ns each time

  // Apologies for the gross code... but it makes a significant speed difference!
  user_id = ((entry_val.x[2]<<16) + (entry_val.x[1]<<8) + entry_val.x[0]) & 0x7FFFF; // extract first 19 bits for user_id

  movie_id = ((entry_val.x[4]<<13) + (entry_val.x[3]<<5) + (entry_val.x[2] >> 3)) & 0x7FFF; // extract 15 bits for movie_id

  rating = ((entry_val.x[5]<<6) + (entry_val.x[4] >> 2)) % NUM_RATINGS;
  date = ((entry_val.x[5]<<6) + (entry_val.x[4] >> 2)) / NUM_RATINGS + 1;
}

// Convert between uncompressed and compressed entry forms
entry_t DataAccessor::make_entry(int index, int user_id, int movie_id, int rating, int date) const {
  entry_t e;
  e.x[0] = index;
  e.x[1] = user_id;
  e.x[2] = movie_id;
  e.x[3] = rating;
  e.x[4] = date;

  return e;
}
entry_t DataAccessor::decompress_entry_val(int index, entry_compressed_t entry_val) const {
  int user_id, movie_id, rating, date;
  entry_t entry;

  parse_entry_val(entry_val, user_id, movie_id, rating, date);

  return make_entry(index, user_id, movie_id, rating, date);
}
entry_compressed_t DataAccessor::compress_entry(entry_t entry) const {
  int user_id, movie_id, rating, date;
  long compressed_val;

  extract_all(entry, user_id, movie_id, rating, date);
  compressed_val = (static_cast<long>((date-1)*NUM_RATINGS + rating) << 34)
      + (static_cast<long>(movie_id) << 19) + user_id;

  return long_to_entry_val(compressed_val);
}

// Convert between compressed entry value and a long (which is slightly less compressed but easier to manipulate)
long DataAccessor::entry_val_to_long(entry_compressed_t entry_val) const {
  // NOTE: this takes about 26ns each time
  long val = (static_cast<long>(entry_val.x[5]) << 40)
      + (static_cast<long>(entry_val.x[4]) << 32)
      + (static_cast<unsigned int>(entry_val.x[3]) << 24)
      + (static_cast<unsigned int>(entry_val.x[2]) << 16)
      + (static_cast<unsigned int>(entry_val.x[1]) << 8)
      + entry_val.x[0];
  return val;
}
entry_compressed_t DataAccessor::long_to_entry_val(long l) const {
  entry_compressed_t compressed;

  for (int i = 0; i < 6; i++) {
    compressed.x[i] = l % 256;
    l /= 256;
  }

  return compressed;
}


// Decompress an entry value from a compressed data file
entry_compressed_t DataAccessor::decompress_datafile_entry(int user_id, int datafile_entry) {
  long movie_id = datafile_entry % num_movies;
  long date_rating = datafile_entry / num_movies;
  return long_to_entry_val((date_rating << 34) + (movie_id << 19) + user_id);
}


// Included for legacy reasons. Equivalent to user_id_from_entry_val(entries[index]).
int DataAccessor::user_id_from_entry_index(int index) const {
  return user_id_from_entry_val(entries[index]);
}

// Calculate movie information for faster access with get_movie_ratings()
// Only called by load_data() after populating user information arrays.
void DataAccessor::calc_movie_info() {
  int *entry_count_per_movie = new int[num_movies];
  
  memset(entries_per_movie, 0, sizeof(int)*num_movies);
  for (int i = 0; i < num_entries; i++) {
    entries_per_movie[movie_id_from_entry_val(entries[i])]++;
  }
  
  movie_start_indices[0] = 0;
  for (int m = 1; m < num_movies; m++) {
    movie_start_indices[m] = movie_start_indices[m-1] + entries_per_movie[m-1];
  }
  
  memset(entry_count_per_movie, 0, sizeof(int)*num_movies);
  for (int u = 0; u < num_users; u++) {
    for (int i = 0; i < entries_per_user[u]; i++) {
      int entry_index = user_start_indices[u] + i;
      int m = movie_id_from_entry_val(entries[entry_index]);
      int index = movie_start_indices[m] + entry_count_per_movie[m];
      movie_entry_indices[index] = entry_index;
      
      entry_count_per_movie[m]++;
    }
  }
  
  delete[] entry_count_per_movie;
}