#include<stdio.h>
#include<unistd.h>

void Echo (FILE *entrada, FILE *salida);

int main (int argc, char *argv[]) {
  int error=0;
  FILE *salida=NULL, *entrada=stdin;

  if (argc==2) {
    salida=popen(argv[0],"w");
  }
  else {
    salida=stdout;
  }

  if (salida!=NULL) {
    if (salida==stdout) {
      printf("Ingrese texto. Para terminar presione Ctrl+D\n");
    }
    Echo(entrada,salida);

    if (salida!=stdout) {
      error=pclose(salida);
      if (error) {
	perror("pclose");
      }
    }
  }
  else {
    perror("popen");
    error=-1;
  }

  return error;
}


void Echo (FILE *entrada, FILE *salida) {
  int lineas=-1, pid=getpid();
  char buffer[80], *ptr=buffer;

  while (ptr==buffer) {
    ptr=fgets(buffer,80,entrada);
    if (ptr==buffer) {
      fprintf(salida,"%d: %s",pid,buffer);
      fflush(NULL);
    }
    lineas++;
  }
  fprintf(salida,"%d: Se leyeron %d lineas\n",pid,lineas);
}

