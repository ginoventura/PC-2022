/* Realizar un programa que busca el valor máximo de un arreglo de datos. El tamaño del arreglo 
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
implemente el programa para que maneje n hilos, con n entre 2 y 9. */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void Buscar(int ini, int tam);

int main () {

    int tam = 0;
    int ini = 0;

    printf("Introduzca un numero entero para establecer el tamaño del arreglo: ");
    scanf("%d", &tam);

    srand(time(NULL));
    Buscar(ini,tam);

    printf("\nFin del programa\n");
    return 0;
}

void Buscar(int ini , int tam) {

    int *arreglo, i;
    arreglo = (int *) calloc(tam, sizeof(int));
    int posicion_aleatoria = rand () % tam; // posicion del arreglo en la que se va a insertar el numero mayor
    int numero_mayor = tam;

    for (i=0; i<tam; i++){
        int numeros_aleatorios = rand () % (tam-1);   
        arreglo[i] = numeros_aleatorios;   
    }

    arreglo[posicion_aleatoria] = numero_mayor;
    
    printf("\nArreglo: ");
    for (i=0; i<tam; i++){
        printf("%d ", arreglo[i]);
    }

    for (int i=ini; i<tam; i++){
        if (arreglo[i] == numero_mayor) {
            printf("\nLa posicion del numero mayor encontrada es: %d.\n", i);
        }        
    }
}
