/*Realizar un programa que busca el valor máximo de un arreglo de datos. El tamaño del arreglo 
lo determina el usuario ingresándolo por teclado.

El arreglo contendrá valores aleatorios entre 0 y P-1 y el valor máximo P deberá colocarse en 
una posición aleatoria del arreglo.

La búsqueda debe realizarse mediante una función que reciba por parámetro las posiciones 
inicial y final. El valor máximo estará contenido en una variable común (compartida) entre 
todos los hilos.

El programa deberá entregarse en las siguientes versiones:
    - Proceso secuencial,
    - Proceso para 2 hilos.
La versión para 2 hilos debe distribuir equitativamente la tarea de búsqueda entre todos los hilos.
DESAFÍO:
implemente el programa para que maneje n hilos, con n entre 2 y 9.*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

pthread_t hilos[2];

int tam=0;

void *Buscar(void *arg);

int main () {

    int id0=0, id1=1;

    printf("Introduzca un numero entero para establecer el tamaño del arreglo: ");
    scanf("%d", &tam);

    pthread_create(&hilos[0], NULL, Buscar, (void *)&id0);
    pthread_create(&hilos[1], NULL, Buscar, (void *)&id1);
    
    pthread_join(hilos[0], NULL);
    pthread_join(hilos[1], NULL);
    
    printf("\nFin del programa\n");
    return 0;
}

void *Buscar(void *arg) {

    int *id=(int *)arg;
    int inicio = (*id)*(tam/2);
    int fin = inicio + (tam/2);

    int *arreglo, i;
    arreglo = (int *) calloc(tam, sizeof(int));

    srand(time(NULL));
    int posicion_aleatoria = rand () % tam; // posicion del arreglo en la que se va a insertar el numero mayor
    int numero_mayor = tam;

    for (i=0; i < tam; i++){
        int numeros_aleatorios = rand () % (tam-1);   
        arreglo[i] = numeros_aleatorios;
    }

    arreglo[posicion_aleatoria] = numero_mayor;
    printf("\nArreglo: ");
    for (i=0; i<tam; i++){
        printf("%d ", arreglo[i]);
    }

    for (int i = inicio; i < fin; i++){
        if (arreglo[i] == numero_mayor) {
            printf("\nLa posicion del numero mayor encontrada es: %d. Encontrado por el hilo: %d\n", i, *id);
        }        
    }
}