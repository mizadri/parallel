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
	int i, j;	
	double sum, promedio; 
	double start, end, elapsed;
	char respuesta[50];
	int nread;

	if(argc != 4){
		fprintf(stderr,"Usage: %s px-width px-height block-size\n", argv[0]);
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
	
	if(xblocks+yblocks > 2000){
			printf("Declarar tantas tareas en MARE podría provocar una violación de segmento.\n");
			printf("¿Desea continuar a pesar de ello?(y/n)\n");
			nread = read(0, respuesta, 50);
			if(nread==-1){
					perror("Fallo al hacer read");
					exit(EXIT_FAILURE);
			}
			respuesta[nread] = '\0';
			if(respuesta[0]!='y'){
				exit(EXIT_FAILURE);
			}
	}
	
	mare::task_ptr tasks[xblocks][yblocks];
	mare::task_ptr faltan_derecha;
	mare::task_ptr faltan_abajo;
	
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
	
	tasks[0][0] = mare::create_task([im,py,tamblock]{
		filtro_secuencial(1, 1+tamblock, 1, 1+tamblock, im, py);
	});

	//Crear columna 1
	for(i=1; i < xblocks; i++){
		istride = 1+i*tamblock;
		jstride = 1;
		tasks[i][0] = mare::create_task([im,py,tamblock,istride,jstride]{
			filtro_secuencial(istride, istride+tamblock, jstride, jstride+tamblock, im, py);
		});
		mare::after(tasks[i-1][0],tasks[i][0]);
	}

	//Crear fila 1
	for(j=1; j < yblocks; j++){
		istride = 1;
		jstride = 1+j*tamblock;
		tasks[0][j] = mare::create_task([im,py,tamblock,istride,jstride]{
			filtro_secuencial(istride, istride+tamblock, jstride, jstride+tamblock, im, py);
		});
		mare::after(tasks[0][j-1],tasks[0][j]);
	}

	//Crear cuerpo de bloques
	for(i=1; i < xblocks; i++){
		for(j=1; j < yblocks; j++){
			istride = 1+i*tamblock;
			jstride = 1+j*tamblock;
			tasks[i][j] = mare::create_task([im,py,tamblock,istride,jstride]{
				filtro_secuencial(istride, istride+tamblock, jstride, jstride+tamblock, im, py);
			});
			mare::after(tasks[i][j-1],tasks[i][j]);
			mare::after(tasks[i-1][j],tasks[i][j]);		
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
	//Crear filtro de elementos que sobran
	if(xleft != 0 && yleft != 0){
		
		faltan_derecha = mare::create_task([im,py,xfiltered,yfiltered]{
			filtro_secuencial(1, xfiltered+1, yfiltered+1, py-1, im, py);//1
		});
		faltan_abajo = mare::create_task([im,py,xfiltered,px]{
			filtro_secuencial(xfiltered+1, px-1, 1, py-1,  im, py);//2
		});
		mare::after(tasks[xblocks-1][yblocks-1],faltan_derecha);
		mare::after(faltan_derecha, faltan_abajo);
			
	}else if(xleft != 0 && yleft == 0){//Te faltan solo abajo
		
		faltan_abajo = mare::create_task([im,py,xfiltered,px]{
			filtro_secuencial(xfiltered+1, px-1, 1, py-1,  im, py);//2
		});
		mare::after(tasks[xblocks-1][yblocks-1],faltan_abajo);
		
	}else if(xleft == 0 && yleft != 0){//Te faltan solo a la dcha
		faltan_derecha = mare::create_task([im,py,xfiltered,yfiltered]{
			
			filtro_secuencial(1, xfiltered+1, yfiltered+1, py-1, im, py);//1
		});
		mare::after(tasks[xblocks-1][yblocks-1],faltan_derecha);
	}
	
	GET_TIME(start);
	for(i=0; i < xblocks; i++){
		for(j=0; j < yblocks; j++){
			mare::launch(tasks[i][j]);		
		}
	}
	
	if(xleft != 0 && yleft != 0){
		mare::launch(faltan_derecha);
		mare::launch(faltan_abajo);
		mare::wait_for(faltan_abajo);	
	}else if(xleft != 0 && yleft == 0){//Te faltan solo abajo
		mare::launch(faltan_abajo);
		mare::wait_for(faltan_abajo);	
	}else if(xleft == 0 && yleft != 0){//Te faltan solo a la dcha
		mare::launch(faltan_derecha);
		mare::wait_for(faltan_derecha);		
	}else{//No hay pixeles sobrantes
		mare::wait_for(tasks[xblocks-1][yblocks-1]);
	}
	
	GET_TIME(end);

	/* Otra forma de hacerlo(sin tareas mare)-> Los sobrantes no se pueden paralelizar
	 if(xleft != 0 && yleft != 0){
		filtro_secuencial(xfiltered+1, xfiltered+1+xleft, 1, py-1, im, py);//1
		filtro_secuencial(1, xfiltered, yfiltered+1, yfiltered+1+yleft, im, py);//2
	}else if(xleft != 0 && yleft == 0){
		filtro_secuencial(xfiltered+1, xfiltered+1+xleft, 1, py-1, im, py);
	}else if(xleft == 0 && yleft != 0){
		filtro_secuencial(1, px-1, yfiltered+1, yfiltered+1+yleft, im, py);
	}
	*/
	
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
	elapsed = end-start;
    printf("tiempo-ejecucion: %.6lf \n", elapsed);

	return EXIT_SUCCESS;
}
