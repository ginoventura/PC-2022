#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void Buscar(int ini, int tam);

int main () {

    int tam = 20000;
    int ini = 0;
    
    srand(time(NULL));
    Buscar(ini,tam);

    printf("Fin del programa\n");
    return 0;
}

void Buscar(int ini , int tam) {
    int *arreglo, i;
    arreglo = (int *) calloc(tam, sizeof(int));
    int aleatorio = rand () % tam; // numero random entre el el inicio y fin del arreglo

    //for para insertar los 0 y el 1 en una posicion aleatoria
    for (i=0; i<20000; i++){        
        arreglo[i] = 0;        
        arreglo[aleatorio-1] = 1;
    }

    //for para mostrar la posicion del 1 
    for(int i= ini ; i<tam ; i++){
        if(arreglo[i]== 1){
          printf("La posicion del numero 1 encontrado es:  %d.\n", i);
        }
    }
}