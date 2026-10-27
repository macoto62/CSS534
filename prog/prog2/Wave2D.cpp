#include <iostream>
#include "Timer.h"
#include <stdlib.h>   // atoi

int default_size = 100;  // the default system size
int defaultCellWidth = 8;
double c = 1.0;      // wave speed
double dt = 0.1;     // time quantum
double dd = 2.0;     // change in system

using namespace std;

int main( int argc, char *argv[] ) {
  // verify arguments
  if ( argc != 4 ) {
    cerr << "usage: Wave2D size max_time interval" << endl;
    return -1;
  }
  int size = atoi( argv[1] );
  int max_time = atoi( argv[2] );
  int interval  = atoi( argv[3] );

  if ( size < 100 || max_time < 3 || interval < 0 ) {
    cerr << "usage: Wave2D size max_time interval" << endl;
    cerr << "       where size >= 100 && time >= 3 && interval >= 0" << endl;
    return -1;
  }

  // create a simulation space
  double z[3][size][size];
  for ( int p = 0; p < 3; p++ ) 
    for ( int i = 0; i < size; i++ )
      for ( int j = 0; j < size; j++ )
	      z[p][i][j] = 0.0; // no wave

  // start a timer
  Timer time;
  time.start( );

  // time = 0;
  // initialize the simulation space: calculate z[0][][]
  int weight = size / default_size;
  for( int i = 0; i < size; i++ ) {
    for( int j = 0; j < size; j++ ) {
      if( i > 40 * weight && i < 60 * weight  && j > 40 * weight && j < 60 * weight ) {
        z[0][i][j] = 20.0;
      } 
      else {
        z[0][i][j] = 0.0;
      }
    }
  }

  // time = 1
  // calculate z[1][][] 
  // cells not on edge
  const double waveFactor = c * c * (dt / dd) * (dt / dd);
  for ( int i = 1; i < size - 1; i++ ) {
    for ( int j = 1; j < size - 1; j++ ) {
      z[1][i][j] = z[0][i][j] + waveFactor / 2.0 * ( z[0][i + 1][j] + z[0][i - 1][j]+ z[0][i][j + 1] + z[0][i][j - 1] - 4.0 * z[0][i][j] );
    }
  }

  if ( interval > 0 && ( 1 % interval == 0 || 1 == max_time - 1 ) ) {
    cout << 1 << "\n";
    for ( int j = 0; j < size; j++ ) {
      for ( int i = 0; i < size; i++ )
        cout << z[1][i][j] << " ";
      cout << "\n";
    }
    cout << "\n";
  }

  // simulate wave diffusion from time = 2
  for ( int t = 2; t < max_time; t++ ) {
    // Rotate the three time planes by index instead of copying grids.
    int current  = t % 3;
    int previous = ( t - 1 ) % 3;
    int old      = ( t - 2 ) % 3;

    for ( int i = 1; i < size - 1; i++ ) {
      for ( int j = 1; j < size - 1; j++ ) {
        z[current][i][j] = 2.0 * z[previous][i][j] - z[old][i][j] + waveFactor * ( z[previous][i + 1][j] + z[previous][i - 1][j] + z[previous][i][j + 1] + z[previous][i][j - 1] - 4.0 * z[previous][i][j] );
      }
    }

    if ( interval > 0 && ( t % interval == 0 || t == max_time - 1 ) ) {
      cout << t << "\n";
      for ( int j = 0; j < size; j++ ) {
        for ( int i = 0; i < size; i++ )
          cout << z[current][i][j] << " ";
        cout << "\n";
      }
      cout << "\n";
    }
  } // end of simulation
    
  // finish the timer
  cerr << "Elapsed time = " << time.lap( ) << endl;
  return 0;
}
