#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>
#include <limits.h>

#define NPIXELX 105
#define NPIXELY 105

#define FILTER(x,y)	\
	im[(x * py) + y] += 0.25 * sqrt(im[((x - 1) * py) + y] + im[(x * py) + y - 1])

int calcularj(int i_inicial, int j_inicial, int *indi, int *indj){
int i,j,size = 0;
	
	for (i=i_inicial,j=j_inicial; j <= i_inicial ; i--,j++) {
		indi[size] = i;
		indj[size] = j;
		size++;
	}

return size;
}

int main(int argc, char **argv) {

	int px = NPIXELX;
	int py = NPIXELY;
	int i, j, d, i_inicial, j_inicial, size;	
	double sum, promedio;
	double tIni, tFin;
	int indi[px];
	int indj[py];

	// Imagen como array de px * py para mayor localidad
	double *im = (double*) malloc(px * py * sizeof(double));

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

	tIni = omp_get_wtime();

	// Filtro a Paralelizar
	for (d = 2; d < 2*px - 3; d++){
		if(d < px){
			i_inicial = d-1;
			j_inicial = 1;
		}else{
			i_inicial = px - 2;
			j_inicial = d - i_inicial;
		}

		size = calcularj(i_inicial,j_inicial, indi, indj);			
		#pragma omp parallel for private(i) 
		for (i=0; i < size ; i++) {		
			FILTER(indi[i], indj[i]);
		}
	}
		

	tFin = omp_get_wtime();

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
	printf("Tiempo: %f\n", tFin - tIni);

	return EXIT_SUCCESS;
}
