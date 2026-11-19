#include <iostream>
#include <string>
#include <sstream>
#include <string.h> // strcmp

using namespace std;
#define MAX 5

int main( ) {

  // reading five keywords into keywords[MAX]
  std::string keywords[MAX];  // key0, key1, key2, ..., key4
  int nKeys = 0;
  for ( nKeys = 0; nKeys < MAX; nKeys++ ) {
    std::string keyword = "key" + std::to_string( nKeys );
    char *value = std::getenv( keyword.c_str( ) );    
    if ( value != NULL ) {
      keywords[nKeys] = std::string( std::getenv( keyword.c_str( ) ) );
      if ( !keywords[nKeys].empty( ) ) continue;
    }
    break;
  }

  int countArray[nKeys] = {0};
  const char* filename = std::getenv("mapreduce_map_input_file"); // the name of the current file being read
  std::string line;
  // Read line-by-line from stdin
  while ( std::getline( std::cin, line ) ) {
    // YOUR WORK
    // Tokenize line by whitespace
    // count plus one if keyword appears
    std::stringstream ss(line);
    std::string word;
    while(ss >> word){
      for(int i=0;i<nKeys;i++){
        if(keywords[i] == word){
          countArray[i]++;
        }
      }
    }
  }

  // YOUR WORK
  // Write "keywords[i] filename_counts
  for(int i=0;i<nKeys;i++){
    if(countArray[i]>0){
      cout << keywords[i] << "\t" << filename << " " << countArray[i] << "\n";
    }
  }
    
  return 0;
}
