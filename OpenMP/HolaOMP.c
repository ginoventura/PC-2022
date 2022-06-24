#include <omp.h>
#include <stdio.h>

int main () {
  int nthreads, tid;
  #pragma omp parallel private(nthreads, tid)
  {
    tid = omp_get_thread_num();
    printf("Hola desde hilo = %d\n", tid);
    if (tid == 0) {
        nthreads = omp_get_num_threads();
        printf("Numero de hilos = %d\n", nthreads);
    }
  }

  return 0;
}
