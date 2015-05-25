#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;

int main ()
{

  ifstream neighborsFile;
  ofstream trialFile;
  trialFile.open("../neighborhood/knn20.txt");

    neighborsFile.open("../neighborhood/knn50.txt");

    int i = 0;
    int j = 0;
    string line;

    while(getline(neighborsFile, line))
    {
      j = 0;
      istringstream iss(line);
      string word;
      while(iss >> word && j < 20)
      {
        trialFile << word << " ";
        j+=1;
      }
      trialFile << endl;
      i+=1;
  }
  printf("%d\t%d\n",i,j);
}