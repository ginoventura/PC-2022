/*Tarea del 18/03 

Hacer un programa que busca un 1 en un arreglo de ceros

* = calloc(20000, sizeof(int));

Versión 1 -> 1 hilo (secuencial) -> Ojo! Usar funciones!
Versión 2 -> 2 hilos de búsqueda, cada hilo busca en la mitad
Versión 3 -> (opcional) La búsqueda se reparte entre n hilos                  
            con n = 2, 3, 4, ..., */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

pthread_t hilos[2];
#define tam 20000

void *Buscar(void *arg);

int main () {

    int id0=0, id1=1;

    pthread_create(&hilos[0], NULL, Buscar, (void *)&id0);
    pthread_create(&hilos[1], NULL, Buscar, (void *)&id1);
    
    pthread_join(hilos[0], NULL);
    pthread_join(hilos[1], NULL);
    
    //sleep(0.5);

    printf("Fin del programa\n");

    return 0;
}

void *Buscar (void *arg) {

    int *id=(int *)arg;
    int inicio = (*id)*(tam/2);
    int fin = inicio+(tam/2);
    
    int *arreglo, i, ini = 0;
    arreglo = (int *) calloc(tam, sizeof(int));
    
    srand (time(NULL));
    int aleatorio = rand() % (tam);     
     
    arreglo[aleatorio] = 1;
     
    for(int i = inicio; i < fin; i++){
        if(arreglo[i] == 1){
            printf("La posicion del numero 1 encontrado es:  %d. Encontrado por el hilo %d\n", i, *id);
        }
    }
}