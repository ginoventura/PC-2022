#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char *argv[]) {
  const int N = 16;
  int i, tid, chunk = 1;
  double a[N], b[N], c[N], d[N];

  /* Some initializations */
  for (i=0; i < N; i++)
    a[i] = b[i] = i * 1.5;

  if (argc > 2)
    chunk = atoi(argv[1]);

  #pragma omp parallel shared(a,b,c,d) private(i, tid)
  {
    tid = omp_get_thread_num();
    printf("Hola desde hilo = %d\n", tid);
    #pragma omp sections nowait
    {
      #pragma omp section
      for (i=0; i < N; i++) {
	c[i] = a[i] + b[i];
	printf(" %3d (%3d) -> %f  +  %f   =   %f  \n", tid, i, a[i], b[i], c[i]);  
      }
      #pragma omp section
      for (i=0; i < N; i++) {
	d[i] = a[i] * b[i];
	printf(" %3d (%3d) -> %f  *  %f   =   %f  \n", tid, i, a[i], b[i], d[i]);  
      }
    }  /* end of sections */
  }  /* end of parallel section */

  return 0;
}
