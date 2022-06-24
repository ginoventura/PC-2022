#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define MAX_THREADS 9

int *array;
int max;
int max_pos;
int n;
int P;

pthread_mutex_t mutex;

void *Buscar(void *arg) {
    int *id = (int *)arg;
    int i;
    int pos = -1;
    int max_local = -1;

    for (i=(*id) * (n/MAX_THREADS); i < ((*id) + 1) * (n/MAX_THREADS); i++) {
        if (array[i] > max_local) {
            max_local = array[i];
            pos = i;
        }
    }

    //Inicio de la Seccion Critica
    pthread_mutex_lock(&mutex);
    if (max_local > max) {
        max = max_local;
        max_pos = pos;
    }

    pthread_mutex_unlock(&mutex);
    //Fin de la Seccion Critica

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    int i;
    int *id;
    pthread_t threads[MAX_THREADS];

    srand(time(NULL));

    printf("Ingrese el tamaño del arreglo: ");
    scanf("%d", &n);

    array = (int *)malloc(sizeof(int) * n);

    P = rand() % n;
    max = P;
    max_pos = rand() % n;
    array[max_pos] = P;

    for (i=0; i<n; i++) {
        if (i != max_pos) {
            array[i] = rand() % P;
        }
    }

    for (i = 0; i < MAX_THREADS; i++) {
        id = (int *)malloc(sizeof(int));
        *id = i;
        pthread_create(&threads[i], NULL, Buscar, (void *)id);
    }

    for (i = 0; i < MAX_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("El valor máximo es %d y se encuentra en la posición %d\n", max, max_pos);

    return 0;
}

