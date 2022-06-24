#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<sys/wait.h>

int LeerTeclado      (int salida);
int EscribirPantalla (int entrada);

int main (int argc, char *argv[]) {
  int error=0, errorHijo=0, pid=0, tubo[2];

  error=pipe(tubo);
  if (!error) {
    pid=fork();
    if (pid<0) {
      perror("fork");
    }
    else if (pid==0) {
      close(tubo[0]);
      error=LeerTeclado(tubo[1]);
      close(tubo[1]);
    }
    else {
      close(tubo[1]);
      error=EscribirPantalla(tubo[0]);
      close(tubo[0]);
      pid=wait(&errorHijo);
      if (pid<0) {
	perror("Child error");
      }
    }
  }
  else {
    perror("pipe");
  }

  return error;
}

int LeerTeclado (int salida) {
  int cantidad=0, error=0, total=0;
  char frase[80];

  fprintf(stdout,"Ingrese texto. Para terminar ingrese el cero '0'\n");
  while ((frase[0]!='0') && (!error)) {
    fgets(frase,80,stdin);
    cantidad=write(salida,frase,strlen(frase)+1);
    if (cantidad>0) {
      total+=cantidad;
    }
    else {
      perror("pipe write");
      error=cantidad;
    }
  }

  fprintf(stdout,"Se enviaron %d char al pipe\n",total);

  return error;
}


int EscribirPantalla (int entrada) {
  int cantidad=0, error=0, total=0;
  char frase[80];

  while ((frase[0]!='0') && (!error)) {
    cantidad=read(entrada,frase,80);
    if (cantidad>0) {
      fprintf(stdout,"Frase: %s\n",frase);
      total+=cantidad;
    }
    else {
      perror("pipe read");
      error=cantidad;
    }
  }

  fprintf(stdout,"Se leyeron %d char del pipe\n",total);

  return error;
}


