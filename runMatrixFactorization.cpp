using namespace std;

void runMatrixFactorization(double ** u, double **v, int k, string inputFile, string outputFile){
    ofstream outFile;
    instream inFile;

    outFile.open(outputFile);
    inFile.open(inputFile);


    //loop through data
    while(getline(inFile, line)){
	outputfile << getResult(parseLine(line), u, v) << endl;
    }
}

*int parseLine(string line){
    int *data = new int*[2];
    int sep1, sep2;
    sep1 = line.find_firstof(' ');
    sep2 = line.find_first_of(' ', sep1 + 1);
    data[0] = atoi(line.substr(0, sep1).c_str());
    data[1] = atoi(line.substr(sep1 + 1, sep2).cstr());
    return data;
}

double getResult(int *data, double **u, double **v){
    double sum = 0;
    for(int i = 0; i < k; i++){
	sum += u[data[0]][i] * v[i][data[1]];
    }
    return sum;
}
