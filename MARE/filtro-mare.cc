#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <mare/mare.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>
#include <limits.h>
#include <sys/time.h>

using namespace std;

#define GET_TIME(now) { \
   struct timeval t; \
   gettimeofday(&t, NULL); \
   now = t.tv_sec + t.tv_usec/1000000.0; \
}

#define NPIXELX 1000
#define NPIXELY 1000

#define FILTER(x,y)	\
	im[(x * py) + y] += 0.25 * sqrt(im[((x - 1) * py) + y] + im[(x * py) + y - 1])


int main() {

	int px = NPIXELX;
	int py = NPIXELY;
	int i, j;	
	double sum, promedio;
	mare::task_ptr tasks[px-2][py-2]; 
	double start, end;
	// Imagen como array de px * py para mayor localidad
	double *im = (double*) malloc(px * py * sizeof(double));

	//mare::runtime::init();
	// "Lectura/Inicialización" de la imagen
	for (i = 0; i < px; i++)
		for (j = 0; j < py; j++)
			im[i * py + j] = (double) (i * px) + j;

	// Promedio inicial  (test de entrada)
	sum = 0.0;
	for (i = 0; i < px; i++)
		for (j = 0; j < py; j++)
			sum += im[(i * py) + j];

	promedio = sum / (px * py);
	printf("El promedio inicial es %g\n", promedio);
	
	tasks[0][0] = mare::create_task([im,py]{FILTER(1, 1);});

	//Crear columna 1
	for(i=2,j=1; i < px -1; i++){
		tasks[i-1][0] = mare::create_task([im,py,i,j]{FILTER(i, j);});
		mare::after(tasks[i-2][0],tasks[i-1][0]);
	}

	//Crear fila 1
	for(i=1,j=2; j < py -1; j++){
		tasks[0][j-1] = mare::create_task([im,py,i,j]{FILTER(i, j);});
		mare::after(tasks[0][j-2],tasks[0][j-1]);
	}
	
	for(i=2; i < px-1; i++){
		for(j=2; j < py-1; j++){
			tasks[i-1][j-1] = mare::create_task([im,py,i,j]{FILTER(i, j);});
			mare::after(tasks[i-1][j-2],tasks[i-1][j-1]);
			mare::after(tasks[i-2][j-1],tasks[i-1][j-1]);		
		}
	}
	
	GET_TIME(start);
	for(i=0; i < px-2; i++){
		for(j=0; j < py-2; j++){
			mare::launch(tasks[i][j]);		
		}
	}


	mare::wait_for(tasks[px-3][px-3]);
	GET_TIME(end);
	// Promedio tras el filtro (test de salida)
	sum = 0.0;
    for(i=0; i < px; i++)
    {
        //printf("\n");
        for(j=0; j < py; j++)
        {
            //printf("%g ",im[(i * py) + j]);
            sum += im[(i * py) + j];
        }
    }
    printf("\n");
	promedio = sum /(px*py);
    printf("El promedio tras el filtro es %g\n", promedio);
	double elapsed = end-start;
    printf("tiempo-ejecucion: %.6lf \n", elapsed);
	//mare::runtime::shutdown();

	return EXIT_SUCCESS;
}
