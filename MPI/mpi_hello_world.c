/* File:       
 *    mpi_hello.c
 *
 * Purpose:    
 *    A "hello,world" program that uses MPI
 *
 * Compile:    
 *    mpicc -g -Wall -std=c99 -o mpi_hello mpi_hello.c
 * Usage:        
 *    mpiexec -n<number of processes> ./mpi_hello
 *
 * Input:      
 *    None
 * Output:     
 *    A greeting from each process
 *
 * Algorithm:  
 *    Each process sends a message to process 0, which prints 
 *    the messages it has received, as well as its own message.
 *
 */
#include <stdio.h>
#include <string.h>  /* For strlen             */
#include <mpi.h>     /* For MPI functions, etc */ 
#ifndef _TIMER_H_
#define _TIMER_H_

#include <sys/time.h>

/* The argument now should be a double (not a pointer to a double) */
#define GET_TIME(now) { \
   struct timeval t; \
   gettimeofday(&t, NULL); \
   now = t.tv_sec + t.tv_usec/1000000.0; \
}

#endif

const int MAX_STRING = 1048577;

int main(int argc, char* argv[]) {
   char       greeting[MAX_STRING];  /* String storing message */
char       received_greet[MAX_STRING];  /* String storing message */
   int        comm_sz;               /* Number of processes    */
   int        my_rank;               /* My process rank        */
   int size;
	double start, finish, elapsed;
   /* Start up MPI */
   MPI_Init(&argc, &argv);

   /* Get the number of processes */
   MPI_Comm_size(MPI_COMM_WORLD, &comm_sz); 

   /* Get my rank among all the processes */
   MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); 

   if (my_rank != 0) { 
      /* Create message */
      //sprintf(greeting, "Greetings from process %d of %d!",my_rank, comm_sz);
      /* Send message to process 0 */
	
	for(size=0;  size<MAX_STRING ; size++){
		greeting[size] = 'x';
		greeting[size+1] = '\0';
		GET_TIME(start);
		MPI_Send(greeting, size, MPI_CHAR, 0, 0,MPI_COMM_WORLD);
		MPI_Recv(received_greet, size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		GET_TIME(finish);
		elapsed = finish - start;
		printf("The message (%d sized) to be timed took %e seconds\n",size, elapsed);
	} 
   } else {  
      /* Print my message */
      printf("Greetings from process %d of %d!\n", my_rank, comm_sz);
      
         /* Receive message from process q */
	for(size=0;  size<MAX_STRING ; size++){
		MPI_Recv(received_greet, size, MPI_CHAR, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Send(greeting, size, MPI_CHAR, 1, 0,MPI_COMM_WORLD);		
	}
      
   }

   /* Shut down MPI */
   MPI_Finalize(); 

   return 0;
}  /* main */
