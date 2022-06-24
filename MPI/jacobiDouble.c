#include <stdio.h>
#include <stdlib.h>

double**  CrearMatriz   (int rows, int cols);
void     BorrarMatriz  (double **m, int rows);
void     CargarMatriz  (double **m, int rows, int cols);
void     MostrarMatriz (double **m, int rows, int cols);

double    Jacobi   (double **ori, double **aux, int rows, int cols, int iters);

int main (int argc, char *argv[]) {
  int error=0, matrixRows=0, matrixCols=0, maxIters=0;
  double **matrizOriginal=NULL, **matrizAuxiliar=NULL, maxDiferencia=0.0;

  /* Verificacion de la cantidad de argumentos */
  if (argc<3) {
    fprintf(stderr,"Utilizar:\n\t%s iters rows [cols]\n",argv[0]);
    error=1;
  }
  else {
    /* Se toma la cantidad de elementos de la matriz (cuadrada) */
    maxIters=atoi(argv[1]);
    matrixRows=atoi(argv[2]);
    matrixCols= argc==4 ? atoi(argv[3]) : matrixRows;
    /* Se reserva la memoria para la matriz principal */
    matrizOriginal=CrearMatriz(matrixRows,matrixCols);
    if (matrizOriginal==NULL) {
      error=1;
    }
  }
  if (!error) {
    /* Se reserva la memoria para la matriz auxiliar */
    matrizAuxiliar=CrearMatriz(matrixRows,matrixCols);
    if (matrizAuxiliar==NULL) {
      error=1;
    }
  }

  if (!error) {
    /* Se inicializan ambas matrices con los mismos valores */
    CargarMatriz(matrizOriginal,matrixRows,matrixCols);
    CargarMatriz(matrizAuxiliar,matrixRows,matrixCols);
/*     MostrarMatriz(matrizOriginal,matrixRows,matrixCols); */
  }

  if (!error) {
    /* Se llama a la funcion que implementa la Iteracion de Jacobi */
    /* que devuelve la maxima diferencia encontrada */
    maxDiferencia=Jacobi(matrizOriginal,matrizAuxiliar,
			 matrixRows,matrixCols,maxIters);
    printf("\nLa maxima diferencia es %g\n\n",maxDiferencia);
/*     MostrarMatriz(matrizOriginal,matrixRows,matrixCols); */
  }

  /* Se libera la memoria reservada para ambas matrices */
  BorrarMatriz(matrizOriginal,matrixRows);
  BorrarMatriz(matrizAuxiliar,matrixRows);

  return error;
}


/* Esta funcion reserva en memoria espacio para una matriz */
double**  CrearMatriz (int rows, int cols) {
  int i=0;
  double **m=NULL;

  m=(double**)(calloc(rows,sizeof(double*)));
/* printf("Allocate: %p |",m); */
  if (*m!=NULL) {
    perror("Chan!");
  }
  while (i<rows) {
    m[i]=(double*)(calloc(cols,sizeof(double)));
/* printf("  %p",m[i]); */
    if (m[i]==NULL) {
      perror("Chan!");
    }
    i++;
  }
/* printf("\n"); */

  return m;
}


/* Esta funcion libera la memoria utilizada para una matriz */
void BorrarMatriz (double **m, int rows) {
  int i=0;
/* printf("Freeing: "); */
  while (i<rows) {
/* printf("%p  ",m[i]); */
    free(m[i]);
    i++;
  }
/* printf("| %p\n",m); */
  free(m);
}


/* Esta funcion carga los valores iniciales en una matriz */
void CargarMatriz (double **m, int rows, int cols) {
  int i=0, fin=cols-1;
  for (i=0; i<rows; i++) {
    m[i][0]=128.0;
    m[i][fin]=10.0;
  }
}


/* Esta funcion muestra en pantalla el contenido de una matriz */
/* No conviene utilizarla con matrices grandes por la cantidad */
/* de informacion que genera */
void MostrarMatriz (double **m, int rows, int cols) {
  int i=0, j=0;
  for (i=0; i<rows; i++) {
    for (j=0; j<cols; j++) {
      printf("%6.2f  ",m[i][j]);
    }
    printf("\n");
  }
}


/* ************************************************** */
/* Esta es la Iteracion de Jacobi Secuencial          */
/* ************************************************** */
double Jacobi (double **ori, double **aux, int rows, int cols, int iters) {
  int i=0, row=0, col=0, uptoRow=rows-1, uptoCol=cols-1;
  int progress=0, step=(iters/10), now=0;
  double dif=0.0, tmp=0.0;

  printf("Progress     ");
  for (i=0; i<iters; i+=2) {
    if (i==now) {
      printf("\b\b\b\b%3d%%",progress);
      fflush(stdout);
      progress+=10;
      now+=step;
    }
    for (row=1; row<uptoRow; row++) {
      for (col=1; col<uptoCol; col++) {
	aux[row][col] = (ori[row-1][col] + ori[row+1][col] +
			 ori[row][col-1] + ori[row][col+1]) * 0.25;
      }
    }
    for (row=1; row<uptoRow; row++) {
      for (col=1; col<uptoCol; col++) {
	ori[row][col] = (aux[row-1][col] + aux[row+1][col] +
			 aux[row][col-1] + aux[row][col+1]) * 0.25;
      }
    }
/*     printf("\n"); */
/*     MostrarMatriz(aux,size); */
/*     printf("\n"); */
/*     MostrarMatriz(ori,size); */
  }

  for (row=1; row<rows; row++) {
    for (col=1; col<cols; col++) {
      tmp = ori[row][col] - aux[row][col];
      if (tmp < 0) {
	tmp = -tmp;
      }
      if (tmp > dif) {
	dif = tmp;
      }
    }
  }
  printf("\b\b\b\b%3d%%\nDone!\n",progress);
  return dif;
}
/* ************************************************** */
/* Fin de la Iteracion de Jacobi Secuencial           */
/* ************************************************** */




