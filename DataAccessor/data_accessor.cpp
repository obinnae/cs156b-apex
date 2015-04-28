#include "data_accessor.h"

DataAccessor::DataAccessor() {
  num_entries = 0;
  num_users = 0;
  num_movies = 0;
  
  entries_per_user = new int[1];
  user_start_indices = new int[1];
  entries = new int[1];
  
  entries_per_movie = new int[1];
  movie_start_indices = new int[1];
  movie_entry_indices = new int[1];
}

DataAccessor::~DataAccessor() {
  delete[] entries_per_movie;
  delete[] movie_start_indices;
  delete[] movie_entry_indices;

  delete[] entries_per_user;
  delete[] user_start_indices;
  delete[] entries;
}

// Loads data from a file compressed in the format described in data_accessor.h.
// Ask me if you have questions about how to make/access this file.
void DataAccessor::load_data(char *datafile) {
  std::ifstream input(datafile);
  
  input.read(reinterpret_cast<char*>(&num_entries), sizeof(int));
  input.read(reinterpret_cast<char*>(&num_users), sizeof(int));
  input.read(reinterpret_cast<char*>(&num_movies), sizeof(int));
  
  delete[] user_start_indices;
  delete[] entries_per_user;
  delete[] entries;
  user_start_indices = new int[num_users];
  entries_per_user = new int[num_users];
  entries = new int[num_entries];
  input.read(reinterpret_cast<char*>(entries_per_user), num_users*sizeof(int));
  input.read(reinterpret_cast<char*>(entries), num_entries*sizeof(int));
  
  user_start_indices[0] = 0;
  for (int i = 1; i < num_users; i++) {
    user_start_indices[i] = user_start_indices[i-1] + entries_per_user[i-1];
  }
  
  delete[] movie_entry_indices;
  delete[] movie_start_indices;
  delete[] entries_per_movie;
  
  movie_entry_indices = new int[num_entries];
  movie_start_indices = new int[num_movies];
  entries_per_movie = new int[num_movies];
  calc_movie_info();
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

// Returns true if the entry has been loaded and false otherwise.
bool DataAccessor::has_entry(int user_id, int movie_id) const {
  int v = find_entry_val(user_id, movie_id);
  return v==-1;
}

// Returns the entry information for the given user_id and movie_id as an entry_t
// type return value. Use extract_XXX() to get information from this value.
// If the entry has not been loaded then all extract_XXX() calls will return -1 with this value.
entry_t DataAccessor::get_entry(int user_id, int movie_id) const {
  int entry = find_entry_val(user_id, movie_id);
  
  return make_entry_t(user_id, entry);
}
entry_t DataAccessor::get_entry(int index) const {
  if (index < 0 || index >= num_entries) {
    return entry_t(-1, 0);
  }
  
  int entry = entries[index];
  int user_id = user_id_from_entry_index(index);
  
  return make_entry_t(user_id, entry);
}

// Retrieves all entries associated with this user, placing them in the user_entries array.
// Return value is the number of user entries.
// Assumes user_entries array is large enough to hold all entries found. Note that
// the most number of entries associated with any one user is 3496.
int DataAccessor::get_user_entries(int user_id, entry_t *user_entries) const {
  int user_start = user_start_indices[user_id];
  for (int i = 0; i < entries_per_user[user_id]; i++) {
    user_entries[i] = make_entry_t(user_id, entries[user_start + i]);
  }
  
  return entries_per_user[user_id];
}

// Retrieves all entries associated with this movie, placing them in the movie_entries array.
// Return value is the number of movie entries.
// Assumes movie_entries array is large enough to hold all entries found. Note that
// the most number of entries associated with any one movie is 242126.
int DataAccessor::get_movie_entries(int movie_id, entry_t *movie_entries) const {
  int movie_start = movie_start_indices[movie_id];
  int entry_index, user_id;
  for (int i = 0; i < entries_per_movie[movie_id]; i++) {
    entry_index = movie_entry_indices[movie_start + i];
    user_id = user_id_from_entry_index(entry_index);
    movie_entries[i] = make_entry_t(user_id, entries[entry_index]);
  }
  
  return entries_per_movie[movie_id];
}


// Unwrap the entry_t object to get the user id, movie id, rating, or date.
// If you will extract all four pieces of information, it's slightly faster to
// extract them all at once with extract_all
int DataAccessor::extract_user_id(entry_t entry) const {
  return entry.first;
}
int DataAccessor::extract_movie_id(entry_t entry) const {
  if (entry.first == -1) return -1;

  return movie_id_from_entry_val(entry.second);
}
int DataAccessor::extract_rating(entry_t entry) const {
  if (entry.first == -1) return -1;

  return rating_from_entry_val(entry.second);
}
int DataAccessor::extract_date(entry_t entry) const {
  if (entry.first == -1) return -1;

  return date_from_entry_val(entry.second);
}
void DataAccessor::extract_all(entry_t entry, int &user_id, int &movie_id, int &rating, int &date) const {
  if (entry.first == -1) {
    user_id = -1;
    movie_id = -1;
    rating = -1;
    date = -1;
  }
  
  user_id = entry.first;
  parse_entry_val(entry.second, movie_id, rating, date);
}



//// Private member functions

// Retrieves the entry value of the desired entry
// Does a modified binary search between the possible indices of the particular movie rating.
// If the entry is found, returns the entry value. Otherwise, returns -1.  
int DataAccessor::find_entry_val(int user_id, int movie_id) const {
  int min_index, max_index;
  int guess;
  int entry_val, entry_movie_id;
  
  min_index = user_start_indices[user_id];
  max_index = (user_id<num_users-1) ? (user_start_indices[user_id + 1]-1) : (num_entries-1);
  guess = movie_id * (max_index - min_index) / num_movies + min_index;
  while (min_index <= max_index) {
    
    entry_val = entries[guess];
    entry_movie_id = movie_id_from_entry_val(entry_val);
     
    if (entry_movie_id < movie_id) { // too early, adjust min_index
      min_index = guess + 1;
    } else if (entry_movie_id > movie_id) { // too far, adjust max_index
      max_index = guess - 1;
    } else { // movie rating found!
      return entry_val;
    }
    
    guess = movie_id * (max_index - min_index) / num_movies + min_index;
  }
  return -1; // movie rating not found
}


// Extract movie, rating, and date information from a compressed 4-byte entry value.
// The entry value is calculated with the formula
//    E = (d*NUM_RATINGS + r)*num_movies + m
int DataAccessor::movie_id_from_entry_val(int entry_val) const {
  return entry_val % num_movies;
}
int DataAccessor::rating_from_entry_val(int entry_val) const {
  int temp = entry_val / num_movies;
  return temp % NUM_RATINGS;
}
int DataAccessor::date_from_entry_val(int entry_val) const {
  int temp = entry_val / num_movies / NUM_RATINGS;
  return temp + 1;
} 
void DataAccessor::parse_entry_val(int entry_val, int &movie_id, int &rating, int &date) const {
  int temp;
  movie_id = entry_val % num_movies;
  temp = entry_val / num_movies;
  
  rating = temp % NUM_RATINGS;
  date = temp / NUM_RATINGS + 1;
}

entry_t DataAccessor::make_entry_t(int user_id, int entry_val) const {
  if (entry_val == -1)
    return entry_t(-1, -1);
  else
    return entry_t(user_id, entry_val);
}

int DataAccessor::user_id_from_entry_index(int index) const {
  // binary search to find user id corresponding to given entry index
  int min_id = 0;
  int max_id = num_users-1;
  int id = index * num_users / num_entries;
  while (min_id < max_id) {
    if (user_start_indices[id] <= index &&
        (id == num_users-1 || user_start_indices[id + 1] > index)) { // user_id is correct
      break;
    } else if (user_start_indices[id] > index) { // user_id is too high
      max_id = id - 1;
    } else { // user_id is too low
      min_id = id + 1;
    }
    
    id = (min_id + max_id) / 2;
  }
  
  return id;
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