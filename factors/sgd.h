#ifndef SGD_H_INCLUDED
#define SGD_H_INCLUDED	

float * gradient(const float y, 
                 const float ** u,
                 const float ** v, 
                 const int index, 
                 const int factor_length,
                 const int non_factor_width,
                 float lambda,
                 bool isU)

#endif