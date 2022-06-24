#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define MAX_THREADS 1

int *array;
int max;
int max_pos;
int P;
int N;

pthread_mutex_t mutex;

void *buscar(void *arg) {
    int id = *(int *)arg;
    int ini = id * (N / MAX_THREADS);
    int fin = (id + 1) * (N / MAX_THREADS);
    int i;
    int local_max = 0;
    int local_max_pos = 0;

    for (i = ini; i < fin; i++) {
        if (array[i] > local_max) {
            local_max = array[i];
            local_max_pos = i;
        }
    }

    pthread_mutex_lock(&mutex);
    
    if (local_max > max) {
        max = local_max;
        max_pos = local_max_pos;
    }
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    int i;
    int *ids;
    pthread_t threads[MAX_THREADS];

    srand(time(NULL));

    //Pedimos los datos al usuario
    printf("Ingrese el tamaño del arreglo: ");
    scanf("%d", &N);
    printf("Ingrese el valor maximo del arreglo: ");
    scanf("%d", &P);

    array = (int *)malloc(sizeof(int) * N);
    ids = (int *)malloc(sizeof(int) * MAX_THREADS);

    for (i=0; i<N; i++) {
        array[i] = rand() % P;
    }

    max_pos = rand() % N;
    max = array[max_pos];
    array[max_pos] = P;

    for (i=0; i<N; i++) {
        printf("%d ", array[i]);
    }

    printf("\n");

    //Crea la cantidad de hilos
    for (i=0; i<MAX_THREADS; i++) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, buscar, (void *)&ids[i]);
    }

    for (i=0; i<MAX_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("El valor maximo es %d y esta en la posicion %d\n", max, max_pos);

    free(array);
    free(ids);

    return 0;
}
