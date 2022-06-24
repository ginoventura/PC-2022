#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char *argv[]) {
  const int N = 16;
  int i = 0, chunk = 1, tid;
  double a[N], b[N], c[N];

  if (argc > 2)
    chunk = atoi(argv[1]);

  /* Some initializations */
  for (i=0; i < N; i++)
    a[i] = b[i] = i * 1.0;
  
  printf("Hilo (pos) -> a[i]  +  b[i]  =  c[i] \n");
  #pragma omp parallel shared(a, b, c, chunk) private(i, tid)
  {
    tid = omp_get_thread_num();
    printf("Hola desde hilo = %d\n", tid);
    #pragma omp for schedule(guided, chunk) nowait
/*    #pragma omp for schedule(static, chunk) nowait */
/*    #pragma omp for schedule(dynamic, chunk) nowait */
    for (i=0; i < N; i++) {
      c[i] = a[i] + b[i];
      printf(" %3d (%3d) -> %f   +   %f   =   %f  \n", tid, i, a[i], b[i], c[i]);  
    }
  }  /* end of parallel section */

  return 0;
}


