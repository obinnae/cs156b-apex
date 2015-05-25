#include <iostream>
#include <cstdlib>
#include <ctime>
#include "data_accessor.h"

float time_access_by_index(DataAccessor *d, int num_indices) {
  time_t t1 = time(NULL);
  for (int i = 0; i < num_indices; i++) {
    d->get_entry(i % d->get_num_entries());
  }
  time_t t2=  time(NULL);
  int time_diff=  difftime(t2, t1);
  float avg_time = ((float)difftime(t2, t1)/num_indices);
  std::cout << "Accessed " << num_indices << " entries by index in " << time_diff << " seconds: " << avg_time << "s/access\n";

  return avg_time;
}

float time_access_by_user(DataAccessor *d, int num_users) {
  int num_entries = 0;
  entry_t *entries = new entry_t[MAX_ENTRIES_PER_USER];
  time_t t1 = time(NULL);
  for (int u = 0; u < num_users; u++) {
    num_entries += d->get_user_entries(u % d->get_num_users(), entries);
  }
  time_t t2 = time(NULL);
  int time_diff=  difftime(t2, t1);
  float avg_time = ((float)difftime(t2, t1)/num_entries);
  std::cout << "Accessed " << num_entries << " entries by user in " << time_diff << " seconds: " << avg_time << "s/access\n";

  return avg_time;
}

float time_access_by_movie(DataAccessor *d, int num_movies) {
  int num_entries = 0;
  entry_t *entries = new entry_t[MAX_ENTRIES_PER_MOVIE];
  time_t t1 = time(NULL);
  for (int m = 0; m < num_movies; m++) {
    num_entries += d->get_movie_entries(m % d->get_num_movies(), entries);
  }
  time_t t2 = time(NULL);
  int time_diff=  difftime(t2, t1);
  float avg_time = ((float)difftime(t2, t1)/num_entries);
  std::cout << "Accessed " << num_entries << " entries by movie in " << time_diff << " seconds: " << avg_time << "s/access\n";

  return avg_time;
}

float time_random_access(DataAccessor *d, int num_users, int num_movies) {
  time_t t1 = time(NULL);
  for (int u = 0; u < num_users; u++) {
    for (int m = 0; m < num_movies; m++) {
      d->get_entry(u % d->get_num_users(), m % d->get_num_movies());
    }
  }
  time_t t2=  time(NULL);
  int time_diff=  difftime(t2, t1);
  float avg_time = ((float)difftime(t2, t1)/(num_users*num_movies));
  std::cout << "Accessed " << (num_users*num_movies) << " entries by (u_id,m_id) in " << time_diff << " seconds: " << avg_time << "s/access\n";

  return avg_time;

}

int main(int argc, char *argv[]) {
  DataAccessor d;
  char *datafile;
  int user_id, movie_id;
  entry_t entry;
  entry_t *user_entries, *movie_entries;
  int num_user_entries, num_movie_entries;
  int rating, date, u, m;
  long num_accessed = 0;
  int num_indices;
  
  if (argc != 5) {
    std::cout << "Usage: data_access_test <data-file> <user-id> <movie-id> <num_indices>\n";
    exit(1);
  }
  datafile = argv[1];
  user_id = atoi(argv[2]);
  movie_id = atoi(argv[3]);
  num_indices = atoi(argv[4]);
  
  std::cout << "Loading data file " << datafile << "...\n";
  d.load_data(datafile);
  std::cout << "Loaded!\n\n";
  

  entry = d.get_entry(user_id, movie_id);
  rating = d.extract_rating(entry);
  date = d.extract_date(entry);
  u = d.extract_user_id(entry);
  m = d.extract_movie_id(entry);
  
  std::cout << "Loaded " << d.get_num_entries() << " entries over " << d.get_num_users() << " users and " << d.get_num_movies() << " movies.\n";
  std::cout << "Accessed rating for user " << (u+1) << " and movie " << (m+1) << ": "
            << rating << " " << date << std::endl;
  //std::cout << "Validation ID for this entry: " << d.get_validation_id(entry) << " = " << d.get_validation_id(u, m) << std::endl;

  time_access_by_index(&d, num_indices);
  time_access_by_user(&d, user_id+1);
  time_access_by_movie(&d, movie_id+1);
  time_random_access(&d, user_id+1, movie_id+1);
}