#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char *argv[]) {
  const int N = 16;
  int i = 0, chunk = 1, tid;
  double a[N], b[N], result = 0.0;

  if (argc > 2)
    chunk = atoi(argv[1]);

  for (i = 0; i < N; i++) {
    a[i] = i * 1.0;
    b[i] = i * 2.0;
  }

  #pragma omp parallel for default(shared) private(i, tid) schedule(static,chunk) reduction(+:result)
      for (i = 0; i < N; i++) {
	result = result + (a[i] * b[i]);
	printf(" %3d (%3d) -> Acumulado =   %f  \n", omp_get_thread_num(), i, result);  
      }
  /* end omp parallel */

  printf("Resultado = %f\n", result);

  return 0;
}
