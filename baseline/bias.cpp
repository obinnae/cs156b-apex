#include "bias.h"
#define GLOBAL_AVG 3.6081

//Constructs a bias object for a given movie and user id
Bias::Bias(char * path1, char * path2){
	//load thetas
	//read in files as arrays
	std::ifstream datafile;
  std::string line;

  theta_1 = new float[MAX_MOVIES];
  theta_2 = new float[MAX_USERS];

  	// read the movie effect thetas
  	datafile.open(path1);
    for(int i = 0; i < MAX_MOVIES; i++) {
    	datafile >> theta_1[i];
    }
    datafile.close();

    //add the user effect thetas
    datafile.open(path2);
    for(int i = 0; i < MAX_USERS; i++) {
    	datafile >> theta_2[i];
    }
    datafile.close();
}

//Destructor
Bias::~Bias() {
	delete[] theta_1;
	delete[] theta_2;
}

float Bias::get_baseline(int movie_id, int user_id){
	float baseline = GLOBAL_AVG;
	baseline = baseline + theta_1[movie_id];
	baseline = baseline + theta_2[user_id];
	return baseline;
}