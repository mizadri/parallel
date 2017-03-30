# include <cstdlib>
# include <iostream>
# include <iomanip>
# include <cmath>
# include <ctime>
# include <mpi.h>

using namespace std;

int main ( int argc, char *argv[] );
double f ( double x );
void timestamp ( );

int main ( int argc, char *argv[] ){
  double h;
  int i;
  int id;
  int ierr;
  int n;
  int p;
  double sum;
  double q_diff;
  double q_exact = 3.141592653589793238462643;
  double sum_local;
  double wtime_diff;
  double wtime_end;
  double wtime_start;
  double x;
  double pi;

  MPI_Init ( &argc, &argv );
  MPI_Comm_size ( MPI_COMM_WORLD, &p );
  MPI_Comm_rank ( MPI_COMM_WORLD, &id );

  if ( id == 0 ) {
	wtime_start = MPI_Wtime ( );
    n = 10000000;
    cout << "  Numero de intervalos " << n << "\n";
  }

  sum_local = 0;
	
  // Comunicar el numero de intervalos a los otros procesos
  MPI_Bcast ( &n, 1, MPI_INT, 0, MPI_COMM_WORLD );

  h = 2.0 / ( double ) n;

  for ( i = id; i < n; i = i + p ) 
  {
	// Hayar punto medio del intervalo
	x = -1 + ( i + 0.5 ) * h;
	sum_local += sqrt( 1 - x * x ) * h;
  } 

   //  Cada proceso envia su parte de la integral al proceso padre
  MPI_Reduce ( &sum_local, &sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD );
  pi = sum * 2.0;

  if ( id == 0 ){

	cout << "  Estimacion de PI: " << pi << "\n";
    cout << "  Valor real      : " << q_exact << "\n";
    q_diff = fabs ( pi - q_exact );
    cout << "  Error           : " << q_diff << "\n";

    wtime_end = MPI_Wtime ( );
    wtime_diff = wtime_end - wtime_start;

    cout << "\n";
    cout << "  Tiempo de CPU = " << wtime_diff << "\n";
  }
  MPI_Finalize();

  return 0;
}
