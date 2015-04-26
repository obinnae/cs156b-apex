#ifndef SGD_H_INCLUDED
#define SGD_H_INCLUDED	

void gradientDescent(float ** u,
                     float ** v, 
                     const int factor_length,
                     const int num_users,
                     const int num_movies,
                     float lambda);

float * gradient(const float y, 
                 const float ** u,
                 const float ** v, 
                 const int index, 
                 const int factor_length,
                 const int non_factor_width,
                 float lambda,
                 bool isU);

#endif