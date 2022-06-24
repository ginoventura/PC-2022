#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

float**  CrearMatriz   (int rows, int cols);
void     BorrarMatriz  (float **m, int rows);
void     CargarMatriz  (float **m, int rows, int cols);
void     MostrarMatriz (float **m, int rows, int cols);

float    Jacobi   (float **ori, float **aux, int rows, int cols, int iters, int rank, int nodos);

int main (int argc, char *argv[]) {
  int error=0, matrixRows=0, matrixCols=0, maxIters=0;
  float **matrizOriginal=NULL, **matrizAuxiliar=NULL, maxDiferencia=0.0;

  /* Para MPI */
  int rank=0, clusterSize=0, alto=0;
/*   float miDif=0.0, otraDif=0.0; */

  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  MPI_Comm_size(MPI_COMM_WORLD,&clusterSize);

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
    /* Se determina el ancho de las franjas */
    alto = matrixRows / clusterSize;
    printf("Proceso %2d: desde fila %d hasta fila %d\n",rank,alto*clusterSize,alto*clusterSize+1);
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
			 matrixRows,matrixCols,maxIters,rank,clusterSize);
    printf("\nLa maxima diferencia es %g\n\n",maxDiferencia);
/*     MostrarMatriz(matrizOriginal,matrixRows,matrixCols); */
  }

  /* Se libera la memoria reservada para ambas matrices */
  BorrarMatriz(matrizOriginal,matrixRows);
  BorrarMatriz(matrizAuxiliar,matrixRows);

  MPI_Finalize();

  return error;
}


/* Esta funcion reserva en memoria espacio para una matriz */
float**  CrearMatriz (int rows, int cols) {
  int i=0;
  float **m=NULL;

  m=(float**)(calloc(rows,sizeof(float*)));
/* printf("Allocate: %p |",m); */
  if (*m!=NULL) {
    perror("Chan!");
  }
  while (i<rows) {
    m[i]=(float*)(calloc(cols,sizeof(float)));
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
void BorrarMatriz (float **m, int rows) {
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
void CargarMatriz (float **m, int rows, int cols) {
  int i=0, fin=cols-1;
  for (i=0; i<rows; i++) {
    m[i][0]=128.0;
    m[i][fin]=10.0;
  }
}


/* Esta funcion muestra en pantalla el contenido de una matriz */
/* No conviene utilizarla con matrices grandes por la cantidad */
/* de informacion que genera */
void MostrarMatriz (float **m, int rows, int cols) {
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
float Jacobi (float **ori, float **aux, int rows, int cols, int iters, int rank, int nodos) {
  int i=0, row=0, col=0, fromRow=0, uptoRow=0, uptoCol=cols-1;
  int franja=rows/nodos, progress=0, step=(iters/10), now=0;
  float maxDif=0.0, miDif=0.0, difOtros=0.0, tmp=0.0;
  MPI_Status estado;

  /* Se determina Fila de inicio y Fila de fin */
  fromRow=nodos*franja;
  uptoRow=(nodos+1)*franja;

  if (!rank) {
    printf("Progress     ");
  }
  for (i=0; i<iters; i+=2) {
    /* El avance del calculo lo hace solamente el nodo 0 */
    if ((i==now) && (!rank)) {
      printf("\b\b\b\b%3d%%",progress);
      fflush(stdout);
      progress+=10;
      now+=step;
    }
    /* Se calculan las diferencias sobre el interior de la franja */
    for (row=fromRow+1; row<uptoRow; row++) {
      for (col=1; col<uptoCol; col++) {
	aux[row][col] = (ori[row-1][col] + ori[row+1][col] +
			 ori[row][col-1] + ori[row][col+1]) * 0.25;
      }
    }
    /* Se envian los valores de la franja a los vecinos */
    if (rank > 1) { /* Hacia vecinos de arriba */
      MPI_Send(aux[fromRow],cols,MPI_FLOAT,rank-1,1,MPI_COMM_WORLD);
    }
    if (rank < nodos) { /* Hacia vecionos de abajo */
      MPI_Send(aux[uptoRow-1],cols,MPI_FLOAT,rank+1,1,MPI_COMM_WORLD);
    }
    /* Se calculan las nuevas diferencias sobre el interior de la franja */
    for (row=fromRow+1; row<uptoRow; row++) {
      for (col=1; col<uptoCol; col++) {
	ori[row][col] = (aux[row-1][col] + aux[row+1][col] +
			 aux[row][col-1] + aux[row][col+1]) * 0.25;
      }
    }
    /* Se reciben los valores de la franja de los vecinos */
    if (rank > 1) { /* Se recibe de los vecinos de arriba */
      MPI_Recv(aux[fromRow],cols,MPI_FLOAT,rank-1,1,MPI_COMM_WORLD,&estado);
    }
    if (rank < nodos) { /* Se recibe de los vecinos de abajo */
      MPI_Recv(aux[fromRow],cols,MPI_FLOAT,rank+1,1,MPI_COMM_WORLD,&estado);
    }
    /* Se calculan los nuevos valores de borde */
    row=fromRow;
    for (col=1; col<uptoCol; col++) {
      aux[row][col] = (ori[row-1][col] + ori[row+1][col] +
		       ori[row][col-1] + ori[row][col+1]) * 0.25;
    }
    row=uptoRow-1;
    for (col=1; col<uptoCol; col++) {
      ori[row][col] = (aux[row-1][col] + aux[row+1][col] +
		       aux[row][col-1] + aux[row][col+1]) * 0.25;
    }
    
/*     if (!rank) { */
/*       printf("\n"); */
/*       MostrarMatriz(aux,size); */
/*       printf("\n"); */
/*       MostrarMatriz(ori,size); */
/*     } */
  }
  /* Se calcula la maxima diferencia en la franja */
  for (row=fromRow+1; row<uptoRow; row++) {
    for (col=1; col<cols; col++) {
      tmp = ori[row][col] - aux[row][col];
      if (tmp < 0) {
	tmp = -tmp;
      }
      if (tmp > miDif) {
	miDif = tmp;
      }
    }
  }
  /* El nodo padre debe calcular la diferencia maxima de toda la matriz */
  if (!rank) {
    printf("\b\b\b\b%3d%%\nDone!\n",progress);
    maxDif=miDif;
    for (row=1; row<nodos; row++) {
      MPI_Recv(&difOtros,1,MPI_FLOAT,row,1,MPI_COMM_WORLD,&estado);
      if (difOtros > maxDif) {
	maxDif=difOtros;
      }
    }
  }
  else { /* Los otros nodos envian la maxima diferencia en la franja */
    MPI_Send(&miDif,1,MPI_FLOAT,0,1,MPI_COMM_WORLD);
  }
  return maxDif;
}
/* ************************************************** */
/* Fin de la Iteracion de Jacobi Secuencial           */
/* ************************************************** */




