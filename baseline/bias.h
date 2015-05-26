#include <iostream>
#include <fstream>
#include "../DataAccessor/data_accessor.h"

class Bias {

	float * theta_1;
	float * theta_2;

public:
	Bias(char * path1, char * path2);
	~Bias();
	float get_baseline(int movie_id, int user_id);
};