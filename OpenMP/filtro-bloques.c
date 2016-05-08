#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>
#include <limits.h>

#define FILTER(x,y)	\
	im[(x * py) + y] += 0.25 * sqrt(im[((x - 1) * py) + y] + im[(x * py) + y - 1])

int calcularj(int i_inicial, int j_inicial, int *indi, int *indj, int yblocks){
int i,j,size = 0;
	
	for (i=i_inicial,j=j_inicial; i>=0 && j<yblocks ; i--,j++) {
		indi[size] = i;
		indj[size] = j;
		size++;
	}

return size;
}

void filtro_secuencial(int istart, int iend,int jstart,int jend, double *im, int py){
		int i,j;
		for(i = istart; i < iend; i++){
				for(j = jstart; j < jend; j++){
					FILTER(i,j);
			}
		}
}

int main(int argc, char **argv) {

	int px, istride,  jstride;
	int py, tamblock, yblocks, xblocks, xfiltered, yfiltered, yleft, xleft;
	int i, j, d, i_inicial, j_inicial, size;	
	double sum, promedio;
	double tIni, tFin;
	
	
	if(argc != 4){
		fprintf(stderr,"Usage: %s n-filas n-columnas block-size\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	px = atoi(argv[1]);
	py = atoi(argv[2]);
	tamblock = atoi(argv[3]);
	
	if((py-2)/tamblock < 2 || (px-2)/tamblock < 2){
		fprintf(stderr,"El tamaño de bloque es demasiado grande: Hay menos de dos bloques.\n");
		exit(EXIT_FAILURE);
	}
	
	xblocks = (px-2) / tamblock;
	yblocks = (py-2) / tamblock;
	xleft = (px-2) % tamblock;
	yleft = (py-2) % tamblock;
	xfiltered = xblocks*tamblock;
	yfiltered = yblocks*tamblock;
	
	printf("xblocks: %i yblocks: %i\n",xblocks,yblocks);
	printf("xleft: %i yleft: %i\n",xleft,yleft);
	
	int indi[xblocks];
	int indj[yblocks];
	
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
	//omp_set_num_threads(8);
	//Nº de diagonales: Vertical + Horizontal - comun
	for(d = 0; d < xblocks + yblocks -1;d++){
			if(d < xblocks){
				i_inicial = d;
				j_inicial = 0;					
			}else{
				i_inicial =	xblocks - 1;
				j_inicial = d - i_inicial;
			}
			size = calcularj(i_inicial,j_inicial, indi, indj, yblocks);			
			#pragma omp parallel for private(i,istride,jstride) 
			for (i=0; i < size ; i++) {
				istride = 1+indi[i]*tamblock;
				jstride = 1+indj[i]*tamblock;
				filtro_secuencial(istride, istride+tamblock, jstride, jstride+tamblock, im, py);
			}
	}
	
	
	//	yfiltered = yblocks * tamblock (hay que sumar primero
	//	xxxxxxxxxxxxxxx		 
	//	x0000000000111x
	//	x0000000000111x
	//	x0000000000111x	primero proceso lo que sobra a la derecha(1s),
	//	x0000000000111x	de esta forma la fila de abajo(2s) puede recibir
	//	x0000000000111x	un valor correcto de los pix superiores
//xFILT>x2222222222222x
	//	x2222222222222x
	//	xxxxxxxxxxxxxxx
	//int filtro_secuencial(int istart, int iend,int jstart,int jend, double *im, int py)
	if(xleft != 0 && yleft != 0){
		filtro_secuencial(1, xfiltered+1, yfiltered+1, py-1, im, py);//1
		filtro_secuencial(xfiltered+1, px-1, 1, py-1,  im, py);//2	
	}else if(xleft != 0 && yleft == 0){//Te faltan solo abajo
		filtro_secuencial(xfiltered+1, px-1, 1, py-1, im, py);
	}else if(xleft == 0 && yleft != 0){//Te faltan solo a la dcha
		filtro_secuencial(1, xfiltered+1, yfiltered+1, py-1, im, py);
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
