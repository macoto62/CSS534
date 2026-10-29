#include <iostream>
#include "Timer.h"
#include <stdlib.h>   
#include "mpi.h"
#include <omp.h>

/* 1. Initialization 
      - include mpi.h
      - int my_rank, mpi_size;
      - MPI_Init( &argc, &argv );
      - MPI_Comm_rank( MPI_COMM_WORLD, &my_rank );
      - MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );
      - MPI_Finalize( );
    2. Implementation
      rank 0: master, rank 1~n: slaves
      MPI_Send(void* msg, int count: BUFFER_SIZE, MPI_Datatype type, int dest, int tag, MPI_COMM comm = MPI_COMM_WORLD);
      MPI_Recv(void* msg, int count, MPI_Datatype type, int source, int tag, MPI_COMM comm = MPI_COMM_WORLD, MPI_Status* status = MPI_STATUS_IGNORE);
      MPI_Bcast(void* msg, int count, MPI_Datatype type, int root, MPI_COMM comm = MPI_COMM_WORLD);
    3. Procedure
      1. Initialize each rank's local matrix z[][][];
      2. exchange the edge rows of z[][][] between ranks;
      3. calculate the new z[][][] for each rank;
      4. gather the results to rank 0 and print the results;
*/


int default_size = 100;  // the default system size
int defaultCellWidth = 8;
double c = 1.0;      // wave speed
double dt = 0.1;     // time quantum
double dd = 2.0;     // change in system

using namespace std;

void synchronize(double* grid, int stripe_size,
                 int size, int my_rank, int mpi_size) {
  if (mpi_size <= 1)
    return;

  const int send_up = 0;
  const int send_down = 1;

  if (my_rank == 0) {
    MPI_Sendrecv(
      grid + stripe_size * size, size, MPI_DOUBLE,
      my_rank + 1, send_down,
      grid + (stripe_size + 1) * size, size, MPI_DOUBLE,
      my_rank + 1, send_up,
      MPI_COMM_WORLD, MPI_STATUS_IGNORE
    );
  }
  else if (my_rank == mpi_size - 1) {
    MPI_Sendrecv(
      grid + size, size, MPI_DOUBLE,
      my_rank - 1, send_up,
      grid, size, MPI_DOUBLE,
      my_rank - 1, send_down,
      MPI_COMM_WORLD, MPI_STATUS_IGNORE
    );
  }
  else {
    MPI_Sendrecv(
      grid + size, size, MPI_DOUBLE,
      my_rank - 1, send_up,
      grid, size, MPI_DOUBLE,
      my_rank - 1, send_down,
      MPI_COMM_WORLD, MPI_STATUS_IGNORE
    );

    MPI_Sendrecv(
      grid + stripe_size * size, size, MPI_DOUBLE,
      my_rank + 1, send_down,
      grid + (stripe_size + 1) * size, size, MPI_DOUBLE,
      my_rank + 1, send_up,
      MPI_COMM_WORLD, MPI_STATUS_IGNORE
    );
  }
}

void gatherAndPrint(double* grid, int stripe_size, int size,
                    int my_rank, int simulation_time, double* output,
                    int* recv_counts, int* displacements) {
  MPI_Gatherv(
    grid + size, stripe_size * size, MPI_DOUBLE,
    output, recv_counts, displacements, MPI_DOUBLE,
    0, MPI_COMM_WORLD
  );

  if (my_rank == 0) {
    cout << simulation_time << "\n";
    for (int j = 0; j < size; j++) {
      for (int i = 0; i < size; i++)
        cout << output[i * size + j] << " ";
      cout << "\n";
    }
    cout << "\n";
  }
}

int main( int argc, char *argv[] ) {
  // initialize MPI
  int my_rank = 0;            // used by MPI
  int mpi_size = 1;           // used by MPI
  MPI_Init( &argc, &argv );
  MPI_Comm_rank( MPI_COMM_WORLD, &my_rank );
  MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );

  // verify arguments
  if ( argc != 5 ) {
    if (my_rank == 0)
      cerr << "usage: Wave2D_mpi size max_time interval threads\n";
    MPI_Finalize();
    return -1;
  }
  int size = atoi( argv[1] );
  int max_time = atoi( argv[2] );
  int interval  = atoi( argv[3] );
  int nthreads = atoi( argv[4] );

  if ( size < 100 || max_time < 3 || interval < 0 || nthreads < 1 || mpi_size > size ) {
    if (my_rank == 0) {
      cerr << "usage: Wave2D_mpi size max_time interval threads\n";
      cerr << "       where size >= 100, time >= 3, interval >= 0, "
              "threads >= 1, and ranks <= size\n";
    }
    MPI_Finalize();
    return -1;
  }

  omp_set_num_threads(nthreads);

  // Initialize
  int num_stripe = size / mpi_size; // the # of rows for each rank
  int remainder = size % mpi_size; // the number of remaining rows
  int stripe_size = num_stripe + (my_rank < remainder ? 1 : 0); // the number of rows for each rank
  int stripe_begin = my_rank * num_stripe + min(my_rank, remainder); // the starting row for each rank

  // Report each rank's global i-index range on stderr, leaving wave data on stdout.
  if (my_rank == 0) {
    for (int rank = 0; rank < mpi_size; rank++) {
      int rank_rows = num_stripe + (rank < remainder ? 1 : 0);
      int rank_begin = rank * num_stripe + min(rank, remainder);
      cerr << "rank[" << rank << "]'s range = " << rank_begin << " ~ " << rank_begin + rank_rows - 1 << "\n";
    }
  }

  // Each time plane is one contiguous array, including two ghost rows.
  double* z[3];
  for (int p = 0; p < 3; p++)
    z[p] = new double[(stripe_size + 2) * size]();

  double* z_output = nullptr;
  int* recv_counts = nullptr;
  int* displacements = nullptr;
  if (my_rank == 0 && interval != 0) {
    z_output = new double[size * size];
    recv_counts = new int[mpi_size];
    displacements = new int[mpi_size];
    for (int rank = 0; rank < mpi_size; rank++) {
      int rank_rows = num_stripe + (rank < remainder ? 1 : 0);
      int rank_begin = rank * num_stripe + min(rank, remainder);
      recv_counts[rank] = rank_rows * size;
      displacements[rank] = rank_begin * size;
    }
  }

  // start a timer
  Timer time;
  time.start( );

  // time = 0;
  // initialize the simulation space: calculate z[0][][]
  int weight = size / default_size;
  #pragma omp parallel for schedule(static)
  for (int local_i = 1; local_i <= stripe_size; local_i++) {
    int global_i = stripe_begin + (local_i - 1);

    for (int j = 0; j < size; j++) {
      if (global_i > 40 * weight && global_i < 60 * weight && j > 40 * weight && j < 60 * weight) {
          z[0][local_i * size + j] = 20.0;
      } 
      else {
          z[0][local_i * size + j] = 0.0;
      }
    }
  }

  synchronize(z[0], stripe_size, size, my_rank, mpi_size);


  // time = 1
  // calculate z[1][][] 
  // cells not on edge
  const double waveFactor = c * c * (dt / dd) * (dt / dd);
  #pragma omp parallel for schedule(static)
  for (int local_i = 1; local_i <= stripe_size; local_i++) {
    int global_i = stripe_begin + local_i - 1;

    // The top and bottom rows are fixed boundaries.
    if (global_i == 0 || global_i == size - 1)
      continue;

    for (int j = 1; j < size - 1; j++) {
      int index = local_i * size + j;
      z[1][index] = z[0][index] + waveFactor / 2.0 * (z[0][index + size] + z[0][index - size] + z[0][index + 1] + z[0][index - 1] - 4.0 * z[0][index]);
    }
  }

  if (interval == 1) {
    gatherAndPrint(z[1], stripe_size, size, my_rank, 1,
                   z_output, recv_counts, displacements);
  }

  // simulate wave diffusion from time = 2
  for ( int t = 2; t < max_time; t++ ) {
    // Rotate the three time grids by index instead of copying grids.
    int current  = t % 3;
    int previous = ( t - 1 ) % 3;
    int old      = ( t - 2 ) % 3;

    synchronize(z[previous], stripe_size, size, my_rank, mpi_size);

    #pragma omp parallel for schedule(static)
    for (int local_i = 1; local_i <= stripe_size; local_i++) {
      int global_i = stripe_begin + local_i - 1;

      // The top and bottom rows are fixed boundaries.
      if (global_i == 0 || global_i == size - 1)
        continue;

      for (int j = 1; j < size - 1; j++) {
        int index = local_i * size + j;
        z[current][index] = 2.0 * z[previous][index] - z[old][index] + waveFactor * ( z[previous][index + size] + z[previous][index - size] + z[previous][index + 1] + z[previous][index - 1] - 4.0 * z[previous][index] );
      }
    }

    if ( interval != 0 && ( t % interval == 0 || t == max_time - 1 ) ) {
      gatherAndPrint(z[current], stripe_size, size, my_rank, t, z_output, recv_counts, displacements);
    }
  } // end of simulation

    
  // finish the timer
  if ( my_rank == 0 ){
    cerr << "Elapsed time = " << time.lap( ) << endl;
  }
  for (int p = 0; p < 3; p++){
    delete[] z[p];
  }
  delete[] z_output;
  delete[] recv_counts;
  delete[] displacements;

  MPI_Finalize( ); // shut down MPI
  return 0;
}
