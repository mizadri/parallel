#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>

int head = 0;
int spot = 0;
int elems = 0;
omp_lock_t ind_lock;
int tam = 1000;

/*-----------------------------------------------------------------
 * Function:  Usage
 * Purpose:   Summary of how to run program
 */
void usage(char* prog_name, int status) {
	if(status){
		fprintf(stdout,"Usage: %s -p <producers> -c <consumers> [-t <BufferSize>][-h]\n",prog_name);
		fprintf(stdout,"\t-h: Displays this help message\n");
		fprintf(stdout,"\t-p <nProducers>\n");
		fprintf(stdout,"\t-c <nConsumers>\n");
		fprintf(stdout,"\t-t <BufferSize>\n");
	}else{
		fprintf(stderr,"Usage: %s -p <producers> -c <consumers> [-t <BufferSize>][-h]\n",prog_name);
		fprintf(stderr,"\t-p <nProducers>\n");
		fprintf(stderr,"\t-c <nConsumers>\n");
		fprintf(stderr,"\t-t <BufferSize>\n");
	}
	exit(status);
}  /* Usage */
/*-----------------------------------------------------------------
 * Function:  Get_args
 * Purpose:   Get and check command line arguments
 * In args:   argc, argv
 * Out args:  n_p, n_c
 */
void get_args(int argc, char* argv[], int* np, int* nc) {
	char optc;
	int nops=0;

	while ((optc = getopt(argc, argv, "+hp:c:t:")) != -1) {
		switch (optc) {
			case 'h': 
				usage(argv[0], EXIT_SUCCESS);
				exit(0);		
			case 'p':
				nops++;
				*np = atoi(optarg);  
				break;
				
			case 'c':
				nops++;
				*nc = atoi(optarg);
				break;
			case 't':
				tam = atoi(optarg);  
				break;
				
			default:
				fprintf(stderr, "Opcion incorrecta: %c\n", optc);
				usage(argv[0], EXIT_FAILURE);
		}
	}
	if(nops != 2){
		usage(argv[0], EXIT_FAILURE);
	}
}  /* Get_args */

int nopuedoconsumir(){
	int ret = 0;
	omp_set_lock(&ind_lock);
	if( spot == head && elems == 0){
		ret = 1;
		elems++;
		#pragma omp flush
	}
	omp_unset_lock(&ind_lock);
	return ret; 
}

int nopuedoproducir(){
	int ret = 0;
	omp_set_lock(&ind_lock);
	if( spot == head && elems == tam){
		ret = 1;
		elems--;
		#pragma omp flush
	}
	omp_unset_lock(&ind_lock);
	return ret; 
}


int main(int argc, char* argv[]){

	int np = 0, nc = 0;
	int *buffer;
	
	omp_init_lock(&ind_lock);

	get_args(argc, argv, &np, &nc);
	printf("Number of Consumers: %i\nNumber of Producers: %i\nTam: %i \n",nc, np, tam);
	buffer = (int*) malloc(tam*sizeof(int));
	omp_set_nested(1);
	
	#pragma omp parallel  num_threads(2)
	{ 
			#pragma omp sections
			{

			#pragma omp section /////////// Productor
			{
				if(np > 0){
					//printf("Number of Sect-Thread: %i\n", omp_get_thread_num());
					#pragma omp parallel num_threads(np)
					{
						int notaccess = 1;
						//printf("Number of Thread: %i\n", omp_get_thread_num());
						 
						while(1){
							
							int random = rand()%100;
							
							
							while(notaccess){
								notaccess = nopuedoproducir();													
							}
							//while(nohayhuecos){esperar}
							
							omp_set_lock(&ind_lock);
							if( elems != tam){
								buffer[spot] = random;
								spot = (spot+1)%tam;
								elems++; 
								
							}else{
								elems--;
							}
							#pragma omp flush
							omp_unset_lock(&ind_lock);
						} 
					}
				}
			} // fin productor

			#pragma omp section  /////////// Consumidor
			{
				
				if(nc > 0){
					int sumatotal = 0;
					//printf("Number of Sect-Thread: %i\n", omp_get_thread_num());
					#pragma omp parallel num_threads(nc)
					{
						int notaccess = 1;
						//printf("Number of Thread: %i\n", omp_get_thread_num());
						while(1){
							
							//while(nohaydatos){esperar}
							   

							while(notaccess){
								notaccess = nopuedoconsumir();
							}
							
							omp_set_lock(&ind_lock);
							if( spot != head && elems > 0){ 
								sumatotal += buffer[head];
								head = (head +1)%tam;
								elems--;
								printf("SumaTotal: %i\n",sumatotal);
								
							}else{
								elems++;
							}
							#pragma omp flush
							omp_unset_lock(&ind_lock);
							
						}
					}
				}			
			}// fin consumidor
		}// fin sections
	}
	omp_destroy_lock(&ind_lock);
	free(buffer);
	return 0;
}
