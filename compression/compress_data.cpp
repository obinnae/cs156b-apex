// all.dta @ /Users/jberleant/Documents/School/Undergraduate/Junior/Spring/CS156B/um/all.dta
// Assumes user-movie-date-rating format
// Requires user-first ordering!!!

// Compresses to ~0.383GB by putting data into following format:
//  1. An array of the number of movies per user.
//     The maximum number of movies per user is 3496, which can be represented by a 4-byte int.
//     There are at most 458293 users.
//     Max array size: 4*458293 bytes ~ 1.75MB
//  2. A list of entries in a compressed format. Each entry is a 4-byte integer E, with
//       E = (d*6 + r)*num_movies + m
//     where m is the movie ID (0-indexed), r is the rating (0 to 5), and d is the 
//     date indicator (0 to 2242).
//     Each entry is 4 bytes and there are 102416306 entries.
//     List size: 4*102416306 bytes ~ 0.382GB
// Total max size: ~0.383GB

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include "utils.h"

int get_compressed_data(char *infile, int *movies_per_user, int *entries, int num_users, int num_movies) {
  std::ifstream input(infile);
  
  int num_entries = 0;
  
  memset(movies_per_user, 0, num_users*sizeof(int));
  
  std::string line;
  int user_id, mov_id, date, rating;
  while (getline(input, line)) {
    parse_train_line(line, user_id, mov_id, date, rating);
    
    movies_per_user[user_id]++;
    entries[num_entries] = ((date-1)*NUM_RATINGS + rating)*num_movies + mov_id;
    num_entries++;
    
    if (num_entries % 1000000 == 0)
      std::cout << line << std::endl;
  }
  input.close();
  
  return num_entries;
}

void output_ints_as_bytes(std::ofstream& output, int *n, int num_ints) {
  int num_bytes = sizeof(int)*num_ints;
  char *bytes = reinterpret_cast<char*>(n);
  output.write(bytes, num_bytes); 
}

void output_compressed_data(char *outfile, int num_entries, int num_users, int num_movies, int *movies_per_user, int *entries) {
  std::ofstream output(outfile);
  
  output_ints_as_bytes(output, &num_entries, 1);
  output_ints_as_bytes(output, &num_users, 1);
  output_ints_as_bytes(output, &num_movies, 1);
  output_ints_as_bytes(output, movies_per_user, num_users);
  output_ints_as_bytes(output, entries, num_entries);
  
  output.close();
}

int main(int argc, char *argv[]) {
  char *infile;
  char *outfile;
  
  int *movies_per_user;
  int *entries;
  int num_entries;
  int num_users, num_movies;
  
  if (argc == 3) {
    infile = argv[1];
    outfile = argv[2];
    num_users = MAX_USERS;
    num_movies = MAX_MOVIES;
  } else if (argc == 5) {
    infile = argv[1];
    outfile = argv[2];
    num_users = atoi(argv[3]);
    num_movies = atoi(argv[4]);
  } else {
    std::cout << "Usage: compress_data <data-file> <output-file> [<num-users> <num-movies>]\n";
    exit(1);
  }
  
  movies_per_user = new int[num_users];
  entries = new int[MAX_TRAINING_ENTRIES];
  num_entries = get_compressed_data(infile, movies_per_user, entries, num_users, num_movies);
  
  output_compressed_data(outfile, num_entries, num_users, num_movies, movies_per_user, entries);

  delete[] entries;
  delete[] movies_per_user;
  
}