#include "mpi.h"
#include <stdlib.h> // atoi
#include <iostream> // cerr
#include "Timer.h"

using namespace std;

void init( double *matrix, int size, char op ) {
  for ( int i = 0; i < size; i++ )
    for ( int j = 0; j < size; j++ )
      matrix[i * size + j]= ( op == '+' ) ? i + j : ( ( op == '-' ) ? i - j : 0 );
}

void print( double *matrix, int size, char id ) {
  for ( int i = 0; i < size; i++ )
    for ( int j = 0; j < size; j++ )
      cout << id << "[" << i << "][" << j << "] = " << matrix[i * size + j] << endl;
}

void multiplication( double *a, double *b, double *c, int stripe, int size ) {
  for ( int k = 0; k < size; k++ )
    for ( int i = 0; i < stripe; i++ )
      for ( int j = 0; j < size; j++ )
        // c[i][k] += a[i][j] * b[j][k];
        c[i * size + k] += a[i * size + j] * b[j * size + k];
}

int main( int argc, char* argv[] ) {
  int my_rank = 0;            // used by MPI
  int mpi_size = 1;           // used by MPI
  int size = 400;             // array size
  bool print_option = false;  // print out c[] if it is true
  Timer timer;

  // variables verification
  if ( argc == 3 ) {
    if ( argv[2][0] == 'y' )
      print_option = true;
  }
  
  if ( argc == 2 || argc == 3 ) {
    size = atoi( argv[1] );
  }
  else {
    cerr << "usage:   matrix size [y|n]" << endl;
    cerr << "example: matrix 400   y" << endl;
    return -1;
  }

  MPI_Init( &argc, &argv ); // start MPI
  MPI_Comm_rank( MPI_COMM_WORLD, &my_rank );
  MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );

  // matrix initialization
  double *a = new double[size * size];
  double *b = new double[size * size];
  double *c = new double[size * size];

  if ( my_rank == 0 ) { // master initializes all matrices
    init( a, size, '+' );
    init( b, size, '-' );
    init( c, size, '0' );

    // print initial values
    if ( false ) {
      print( a, size, 'a' );
      print( b, size, 'b' );
    }

    // start a timer
    timer.start( );
  }
  else {                // slavs zero-initializes all matrices
    init( a, size, '0' );
    init( b, size, '0' );
    init( c, size, '0' );
  }

  // broadcast the matrix size to all.
  MPI_Bcast( &size, 1, MPI_INT, 0, MPI_COMM_WORLD );

  int stripe = size / mpi_size;     // partitioned stripe
  int local_rows = stripe;

  if ( my_rank == mpi_size - 1 ){
    local_rows += size % mpi_size; // last rank may have more rows
  }

  // master sends each partition of a[] to a different slave
  // master also sends b[] to all slaves
  // master also sends c[] to all slaves
  if ( my_rank == 0 ) {
    for ( int i = 1; i < mpi_size; i++ ) {
      if( i == mpi_size - 1){
        MPI_Send(&a[i * stripe * size], (size - (i * stripe)) * size, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
      }
      else{
        MPI_Send( &a[i * stripe * size], stripe * size, MPI_DOUBLE, i, 0, MPI_COMM_WORLD );
      }
      MPI_Send( b, size * size, MPI_DOUBLE, i, 0, MPI_COMM_WORLD );
      MPI_Send( c, size * size, MPI_DOUBLE, i, 0, MPI_COMM_WORLD );
    }
  }
  else {
    if( my_rank == mpi_size - 1 ) {
      MPI_Recv( a, (size - (my_rank * stripe)) * size, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
    }
    else{
      MPI_Recv( a, stripe * size, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
    }
    MPI_Recv( b, size * size, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
    MPI_Recv( c, size * size, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
  }

  multiplication( a, b, c, local_rows, size ); // all ranks should compute multiplication

  // master receives each partition of c[] from a different slave
  if ( my_rank == 0 ) {
    for ( int i = 1; i < mpi_size; i++ ) {
      if( i == mpi_size - 1 ){
        MPI_Recv( &c[i * stripe * size], (size - (i * stripe)) * size, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
      }
      else{
        MPI_Recv( &c[i * stripe * size], stripe * size, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
      }
    }
  }
  else {
    if( my_rank == mpi_size - 1 ){
      MPI_Send( c, (size - (my_rank * stripe)) * size, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD );
    }
    else{
      MPI_Send( c, stripe * size, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD );
    }
  }

  if ( my_rank == 0 )
    // stop the timer
    cout << "elapsed time = " << timer.lap( ) << endl;

  // results
  if ( print_option && my_rank == 0 )
    print( c, size, 'c' );

  MPI_Finalize( ); // shut down MPI
}


  

  
