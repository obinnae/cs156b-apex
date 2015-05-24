#include <iostream>
#include <fstream>
#include <string>
#include "../DataAccessor/data_accessor.h"

int main(int argc, char *argv[]) {
  DataAccessor d;
  entry_t e;
  int user_id, movie_id, date, rating;
  char *compressed_file, *decompressed_file;

  if (argc != 3) {
  	std::cout << "Usage: decompress <compressed-file> <output-file>";
  	exit(1);
  }
  compressed_file = argv[1];
  decompressed_file = argv[2];

  d.load_data(compressed_file);

  std::ofstream out(decompressed_file);
  for (int i = 0; i < d.get_num_entries(); i++) {
  	e = d.get_entry(i);
  	d.extract_all(e, user_id, movie_id, rating, date);
  	out << (user_id + 1) << " " << (movie_id + 1) << " " << date << " " << rating << std::endl;

  	if (i % 500000 == 0)
  	  std::cout << (100*i/d.get_num_entries()) << "% done.\n";
  }
  out.close();

}