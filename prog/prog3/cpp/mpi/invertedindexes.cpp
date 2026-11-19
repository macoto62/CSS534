#include <iostream>  // cout
#include "mpi.h"
#include "Timer.h"
#include <filesystem>
#include <vector>
#include <fstream>

#define BUFSIZE 100

using namespace std;
namespace fs = std::filesystem;

int my_rank;              // rank of process
int nprocs;               // #processes
int master = 0;           // master rank
int tag = 0;              // tag for messages

vector<string> getFileNames( string path ) {
  vector<string> filenames;

  // retrieve all file names from a given directory in path.
  if ( fs::exists( path ) && fs::is_directory( path ) ) {
    for ( const auto& entry : fs::directory_iterator( path ) ) {
      if ( fs::is_regular_file( entry.status( ) ) ){
        filenames.push_back( entry.path( ).filename( ).string( ) ); 
      }
    }
  }
  else{
    if(my_rank == 0 ){
      std::cout << "Path not exist or is not a directory\n";
    }
  }
  
  return filenames;
 }

void invertedindexing( string inputPath, vector<string> keywords, vector<string> filenames ) {
  string* emits = new string[ keywords.size( ) ]; // outputs are emitted here.

  // map
  int* keywordCount = new int[ keywords.size( ) ];            // the occurence of keywords 
  for ( int i = my_rank; i < filenames.size( ); i += nprocs ) { // each rank takes care of filename[rank + i];
    for ( int j = 0; j < keywords.size( ); j++ ) {
      keywordCount[j] = 0;
    }
      
    // read word by word from a filename[rank + i]
    std::string filepath = inputPath + "/" + filenames[i];
    ifstream infile( filepath );

    // YOUR WORK
    // create a list of tuples <filename occurent> for each keyword[j]
    try{
      if(!infile.is_open()){
        string err_msg = "Cannot open " + filepath;
        throw std::runtime_error(err_msg);
      }
      std::string word;
      while(infile >> word){
        for(int j=0;j < keywords.size();j++){
          if(word == keywords[j]){
            keywordCount[j]++;
          }
        }
      }
    } catch(const std::exception &e) {
      std::cerr << "Error: " << e.what() << "\n";
    }
      
    // like: emits[j] += ( " " + filenames[i] + " " + to_string( keywordCount[j] ) );
    for(int j=0;j<keywords.size();j++){
      if(keywordCount[j] > 0 ){
        emits[j] += " " + filenames[i] + " " + to_string(keywordCount[j]);
      }
    }
  }
  
  // reduce
  for ( int j = 0; j < keywords.size( ); j++ ) { // for each keyword, collect a list of <filename occurence>
    
    if ( my_rank == 0 ) {
      for ( int worker = 1; worker < nprocs; worker++ ) {
        int size = 0;
        MPI_Recv( &size, 1, MPI_INT, worker, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
        string received( size, '\0' );
        if ( size > 0 ){
          MPI_Recv( received.data( ), size, MPI_CHAR, worker, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
        }
        emits[j] += received;
      }
      cout << keywords[j] << "\t" + emits[j] << endl; // print this keyword's list of <filename occurence>
    }
    else { // worker: rank 1 through to rank n-1
      int size = static_cast<int>( emits[j].size( ) );
      MPI_Send( &size, 1, MPI_INT, master, tag, MPI_COMM_WORLD );
      if ( size > 0 )
        MPI_Send( emits[j].data( ), size, MPI_CHAR, master, tag, MPI_COMM_WORLD );
    }
  }
}

int main( int argc, char* argv[] ) {


  char message[BUFSIZE];    // return status for 
  MPI_Status status;        // receive
  char hostname[BUFSIZE];   // host name

  MPI_Init( &argc, &argv ); // Start up MPI

  MPI_Comm_rank( MPI_COMM_WORLD, &my_rank ); // Find out process rank
  MPI_Comm_size( MPI_COMM_WORLD, &nprocs );  // Find out # processes
  
  // initialize inputPath and keywordPaths.                                                                                                                          
  string inputPath = argv[1];
  
  vector<string> keywords;
  for ( int i = 2; i < argc; i++ )
    keywords.push_back( argv[i] );

  // debugging                                                                                                                                                       
  if ( my_rank == 0 ) {
    cout << "rank[" << my_rank << "]: input_path = " << inputPath << endl;
    
    for ( string key : keywords )
      cout << "rank[" << my_rank << "]: keyword = " + key << endl;
  }

  // Start Computation
  Timer timer;
  timer.start( );

  // Read all file names
  vector<string> filenames = getFileNames( inputPath );
  
  // An actual implementation of inverted indexing
  invertedindexing( inputPath, keywords, filenames );
  
  // Terminate the MPI library.
  if ( my_rank == 0 ) {
    // stop the timer
    cout << "elapsed time = " << timer.lap( ) << endl;
  }
  
  MPI_Finalize( );          // Shut down MPI
}
