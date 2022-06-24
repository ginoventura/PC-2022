#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <omp.h>

double SimpsonRule (double a, double b, int n);

int main(int argc, char** argv)
{
	const double pi = 3.14159265359;
	double a = 3 * pi / 4, b = 2 * pi, resultado = 0.0;
/*	double a = 0.0, b = pi, resultado = 0.0; */
/*	double a = 0.0, b = 2 * pi, resultado = 0.0; */
	int error = 0, n = 9;
	
	if (argc == 2)
		n = atoi(argv[1]);
	
	printf("Regla de Simpson para integral definida de sin(x)\n");
	if (n % 2 == 0)
		printf("La cantidad de intervalos debe ser impar\n");
	else {
		printf("Rango [%f, %f] con %d intervalos\n", a, b, n);
		resultado = SimpsonRule(a, b, n);
		printf("Resultado: %f\n", resultado);
	}

	return error;

}

double SimpsonRule (double a, double b, int n) {
	double r = 0.0 , h = (b - a) / n, x = a + h;
	int c = 4, var = -2, i = 0;
	int chunk = 0;
	
	#pragma omp parallel default(shared) private(i, c, x, var)
	{
		chunk = (n - 2) / omp_get_num_threads();
/*		printf("Hola desde hilo %d/%d\n", omp_get_thread_num(), omp_get_num_threads()); */
		var = omp_get_thread_num();
		#pragma omp for schedule(static,chunk) reduction(+:r)
		for (i = 1; i < n - 1; i++) {
			x = h * i + a;
			c = 2 + i % 2 * 2;
			r += c * sin(x);
			printf("hilo %d - i = %d - r = %f - c = %d - sin(%f) = %f - h = %f (%d)\n", var, i, r, c, x, sin(x), h, chunk);
		}
	}

	r += sin(a) + sin(b);
	
	r *= h / 3;
	
	return r;
}

