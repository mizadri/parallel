#include <stdlib.h>
#include <stdio.h>
#include "omp.h"

#define NMAX 200
#define DEBUG 1

struct nodo {
   int inputdata;
   int outdata;
   struct nodo* next;
};

int fibo(int n) {
   int x, y;
   if (n < 2) {
      return (n);
   } else {
      x = fibo(n - 1);
      y = fibo(n - 2);
      return (x + y);
   }
}

void updatenodo(struct nodo* p) 
{
   p->outdata = fibo(p->inputdata);
}

struct nodo* init_list(struct nodo* p, int size, int firstnum) {
    int i;
    struct nodo* head = NULL;
    struct nodo* temp = NULL;
    
    
    head = malloc(sizeof(struct nodo));
    p = head;
    p->inputdata = firstnum;
    p->outdata = 0;
    for (i=0; i< (size-1); i++) {
       temp  = malloc(sizeof(struct nodo));
       p->next = temp;
       p = temp;
       p->inputdata = firstnum + i + 1;
       p->outdata = i+1;
    }
    p->next = NULL;
    return head;
}

int main(int argc, char *argv[]) {
    double tinicio, tfin;
    struct nodo *puntero=NULL;
    struct nodo *temp=NULL;
    struct nodo *iniciolista=NULL;
    struct nodo *arraypunteros[NMAX]; 
    
    int contador= 0,i;
    
    int tamanyolista, numinicial;
    
    if(argc ==3){
        tamanyolista=atoi(argv[1]);
        numinicial=atoi(argv[2]);
    } else if(argc ==2){
        tamanyolista=atoi(argv[1]);
        numinicial=1;
    } else {
        tamanyolista=40;
        numinicial=1;
    }
    
    printf("La funcion init_list inicializa una lista enlazada de tamaño %d (el tamaño debe ser menor que NMAX=%d)\n",tamanyolista,NMAX);
    printf("La funcion updatenodo procesa un nodo de la lista\n");
    printf("Cada nodo almacenara como salida la sucession de fibonacci, empezando en %d\n", numinicial);      
 
    puntero = init_list(puntero,tamanyolista,numinicial);
    iniciolista = puntero;

    tinicio = omp_get_wtime();
    	
    while (puntero != NULL) {
	arraypunteros[contador]= puntero;
	puntero = puntero->next;
	contador++;
    }
	#pragma omp parallel for firstprivate(contador) schedule(dynamic)
	for(i = 0 ; i < contador; i++){	
		updatenodo(arraypunteros[i]);
	
	}

    tfin = omp_get_wtime();
    
    printf("Tiempo procesamiento serie lista: %lf segundos\n", tfin - tinicio);
    
    
    
    
 
    puntero=iniciolista; 
    while (puntero != NULL) {
        if(DEBUG) printf("%d : %d\n",puntero->inputdata, puntero->outdata);
        temp = puntero->next;
        free (puntero);
        puntero = temp;
    }  

    

     return 0;
}

