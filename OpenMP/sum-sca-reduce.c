#include <stdio.h> 
#include <mpi.h> 
#include <time.h>
#include <stdlib.h>

#define size 1000000

int my_id, nproc, i, first, chunk; 
int a[size], b[size]; 
double local_sum, sum; 
MPI_Status status; 
double wtime_diff;
double wtime_end;
double wtime_start;

int main (int argc, char** argv) 
{             
  MPI_Init(&argc, &argv); 
  MPI_Comm_rank(MPI_COMM_WORLD, &my_id); 
  MPI_Comm_size(MPI_COMM_WORLD, &nproc); 
  srand(time(NULL));

  /* No calcule si size no es divisible por nproc */ 
  if (chunk*nproc != size) { 
    if (my_id == 0) 
      printf("'size(%d)' debe ser divisible por 'nproc(%d)'\n",size,nproc);  
    MPI_Finalize(); 
    exit(1); 
  } 
  
  if ( my_id == 0 ){
    wtime_start = MPI_Wtime ( );
  }

	chunksize = size/nproc; 
  /* Master rellena el vector */ 
  if (my_id == 0) {
    printf("Numero de procesos: %d\n",nproc);
    for (i=0;i<size;i++)
      a[i] = i+1; 
  }

  /* Distribuye el vector desde el master en sub-vectores de longitud chunksize. */ 
  MPI_Scatter(a,chunksize,MPI_INT,b,chunksize,MPI_INT,0,MPI_COMM_WORLD); 

  local_sum = 0; 
  for (i=0;i<chunksize;i++)
    local_sum = local_sum + b[i];

  /* Hacer la reducción de las sumas parciales para obtener la suma total */ 
  MPI_Reduce(&local_sum,&sum,1,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);

  if (my_id == 0){
    wtime_end = MPI_Wtime ( );
    wtime_diff = wtime_end - wtime_start;
    printf("Suma: %10.0f\n",sum);
    printf("Tiempo: %f\n",wtime_diff);
  }
  MPI_Finalize(); 
}
