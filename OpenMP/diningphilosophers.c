/* 
 * Simulación del problema de los filosofos chinos. 
 *
 * Vida del filosofo:
 * - Coger paralillo de la izquierda (Cada palillo es un recurso que se debe utilizar de forma exclusiva: tiene asociado un mutex) 
 * - Cojer palillo de la derecha 
 * - Comer (tiempo variable)
 * - Soltar palillo de la izquierda
 * - Soltar palillo de la derecha
 * - Pensar (tiempo variable)
 *
 * Problema clasico para introduccir el concepto de interbloqueo
 * 
 */

/* http://docs.oracle.com/cd/E19205-01/820-0619/geosb/index.html */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <unistd.h>
#include <time.h>

#define CHINOS 5

/* Definicion de tiempos maximos y minimos*/
static const int min_comer[CHINOS]= {300, 300, 300, 300, 300};
static const int max_comer[CHINOS]= {600, 600, 600, 600, 600};
static const int min_pensar[CHINOS] = {300, 300, 300, 300, 300};
static const int max_pensar[CHINOS] = {600, 600, 600, 600, 600};

/* Funciiones */
static void vida_filosofo(int chinoid, omp_lock_t *palilloizda, omp_lock_t *palillodcha, unsigned *semilla);
static void millisleep(int ms);
static void sleep_rand(int min_ms, int max_ms, unsigned *semilla);


int main(int argc, char **argv)
{
    int chinoid;
    time_t tiempo;
    /* Los palillos son recursos, representados por mutexes  */
    omp_lock_t palillos[CHINOS];
    omp_lock_t *palilloizda;
    omp_lock_t *palillodcha;
    /* Semilla aleatoria*/
    unsigned semilla;

    tiempo = time(NULL);
    /* Cada filosofo chino es un thread */
    #pragma omp parallel num_threads(CHINOS) private(chinoid,palilloizda,palillodcha,semilla)
    {
	int primero = chinoid;
	int segundo = (chinoid+1) % CHINOS;
	int aux = 0;

	if(primero > segundo){
		aux = primero;
		primero = segundo;
		segundo = aux;
	}

	chinoid = omp_get_thread_num();
	palilloizda = &palillos[primero];
	omp_init_lock(&palillos[chinoid]);
	palillodcha = &palillos[segundo];
	

	semilla = tiempo + chinoid;
	while (1) {
	    vida_filosofo(chinoid, palilloizda, palillodcha, &semilla);
	}
    }
    return 0;
}

static void vida_filosofo(int chinoid, omp_lock_t *palilloizda, omp_lock_t *palillodcha, unsigned *semilla)
{
	printf("El filosofo %d preparado para comer\n", chinoid);
	omp_set_lock(palilloizda); 
    printf("El filosofo %d ha cogido el palillo de la izquierda\n", chinoid);
    omp_set_lock(palillodcha); 
    printf("El filosofo %d ha cogido ek palillo de la derecha\n", chinoid);

	sleep_rand(min_comer[chinoid], max_comer[chinoid], semilla);

	omp_unset_lock(palilloizda); 
    omp_unset_lock(palillodcha);
	printf("El filosofo %d ha termindado de comer y ha soltado los palillos\n", chinoid);

	sleep_rand(min_pensar[chinoid], max_pensar[chinoid], semilla);

}

/* Suspender ejecución (milisegundos) */
static void millisleep(int ms)
{
	usleep(ms * 1000);
}

/* Suspends the execution of the calling thread for a random time between
 * min_ms milliseconds and max_ms milliseconds.  */
static void sleep_rand(int min_ms, int max_ms, unsigned *semilla)
{
	int rango, ms;
    rango = max_ms - min_ms + 1;
	ms = rand_r(semilla) % rango + min_ms;
	millisleep(ms);
}
