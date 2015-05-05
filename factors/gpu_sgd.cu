void gradient(const float * const * u, 
                 const float * const * v,
                 const int index,
                 const float * y
                 Baseline *b,
                 const int factor_length,
                 float lambda,
                 bool isU
                 int k,
                 int sizeofV){
    /* k should be num factors, sizeofV is number of movies or users */
    if(isU){
        cudaMemcpy(dev_U, u[index], k * sizeof(float), cudaMemcpyHostToDevice);
        cudaMemcpy(dev_V, v, sizeofV * k * sizeof(float), cudaMemcpyHostToDevice);
        
    }else{
        cudaMemcpy(dev_U, v[index], k * sizeof(float), cudaMemcpyHostToDevice);
        cudaMemcpy(dev_V, u, sizeofV * k * sizeof(float), cudaMemcpyHostToDevice);
       
    }
    cudaMemcpy(dev_Y, y, sizeofV * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(dev_lambda, lambda, sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(dev_sizeofV, sizeofV, sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(dev_K, k, sizeof(int), cudaMemcpyHostToDevice);
    cudaGradient<<<1024, 32>>> (dev_U, dev_V, dev_Y, dev_lambda, dev_sizeofV, dev_K);
    if(isU){
        cudaMemcpy(u[index], dev_U, k * sizeof(float), cudaMemcpyDeviceToHost);
        cudaMemcpy(v, dev_V, sizeofV * k * sizeof(float), cudaMemcpyDeviceToHost);
    }else{
        cudaMemcpy(v[index], dev_U, k * sizeof(float), cudaMemcpyDeviceToHost);
        cudaMemcpy(u, dev_V, sizeofV * k * sizeof(float), cudaMemcpyDeviceToHost);
    }
    float *sum = malloc(sizeofV * sizeof(float));
    cudaMemcpy(sum, out, sizeofV * sizeof(float), cudaMemcpyDeviceToHost);
    int i;
    float total = 0;
    for (i = 0; i < sizeofV; i++){
        total += sum[i];
    }

    float *grad = malloc(k * sizeof(float));
    for (i = 0; i < k; i++){
        grad[i] = lambda * dev_U[i] - dev_V[i];
    }
                         

                     
}

void cudaGradient(const float *dev_U, const float * const * dev_V, const float * dev_Y, float dev_lambda, int dev_sizeofV, int dev_K, const float *out){

    unsigned int index = blockIdx.x * blockDim.x + threadIdx.x;
    while (index < dev_sizeofV){
        unsigned int i = 0;
        float sum = 0;
        for (i = 0; i < dev_K; i++){
            sum += dev_U[i] * dev_V[index][i];
        }
        sum = dev_Y[index] - sum;
    }
    out[index] = sum;


}
