#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 10000

int array[SIZE];

int main(int argc, char **argv){
	int maximo = 0;
	int i;

	#pragma omp parallel for
	for (i = 0; i < SIZE; i++) {
	    array[i] = rand();
	}	
	#pragma omp parallel for reduction (max: maximo)
	for(i=0; i < SIZE; i++){
		if (array[i] > maximo) maximo = array[i];
	}
	printf("el maximo es %d \n",maximo);

	return 0;
}
