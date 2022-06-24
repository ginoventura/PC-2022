#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>
#define numHilos 20
pthread_t hilos[numHilos];
int valores[10];

// Variable compartida
int indiceMaximo;
int numMax;
int maximoLocal[numHilos];
int inicio;
int l;
int final;
int n,m;
int contador=1;
int i,j;

// Semáforo
sem_t x;

// Función que será ejecutada por todos los hilos
void* BuscarMaximo(void* arg)
{
    int id = *((int*) arg);
    int miMaximo = 0;
    int miIndiceMaximo = 0;
    int inicioLocal = (id * (n / numHilos)) + 1;
    int finalLocal = (id + 1) * (n / numHilos);
    for (i = inicioLocal; i < finalLocal; i++)
    {
        if (valores[i] > miMaximo)
        {
            miMaximo = valores[i];
            miIndiceMaximo = i;
        }
    }
    maximoLocal[id] = miMaximo;
    if (inicioLocal == 1)
    {
        numMax = miMaximo;
        indiceMaximo = miIndiceMaximo;
    }
    for (i = inicioLocal; i < finalLocal; i++)
    {
        if (miMaximo > numMax)
        {
            sem_wait(&x);
            numMax = miMaximo;
            indiceMaximo = miIndiceMaximo;
            sem_post(&x);
        }
    }
    pthread_exit(NULL);
}
int main()
{
    // Inicializamos el semáforo con el valor 1
    sem_init(&x, 0, 1);
    // Obtenemos el número de elementos del arreglo
    do
    {
        printf("Ingresa el tamano del arreglo (entre 2 y 10): ");
        scanf("%d", &n);
    }
    while (n < 2 || n > 10);
    // Llenamos el arreglo al azar
    printf("\n");
    srand(time(NULL));
    for (i = 1; i <= n; i++)
    {
        valores[i] = rand() % 100;
        printf("%d ", valores[i]);
    }
    printf("\n\n");
    // Creamos los hilos
    for (i = 0; i < numHilos; i++)
    {
        int* id = (int*) malloc(sizeof(int));
        *id = i;
        pthread_create(&hilos[i], NULL, BuscarMaximo, (void*) id);
    }
    // Esperamos a que todos los hilos terminen
    for (i = 0; i < numHilos; i++)
    {
        pthread_join(hilos[i], NULL);
    }
    // Imprimimos el resultado
    printf("El valor maximo es %d y se encuentra en la posicion %d\n", numMax, indiceMaximo);
    printf("\nHilos creados: ");
    for (i = 0; i < numHilos; i++)
    {
        printf("%d ", hilos[i]);
    }
    printf("\n");
    pthread_exit(NULL);
}

