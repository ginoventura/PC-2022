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
  int cantidad=0, error=0;
  char frase[80];

  fprintf(stdout,"Ingrese una frase: ");
  fgets(frase,80,stdin);
  cantidad=write(salida,frase,strlen(frase)+1);
  if (cantidad>0) {
    fprintf(stdout,"Se enviaron %d char al pipe\n",cantidad);
  }
  else {
    perror("pipe write");
    error=cantidad;
  }
  return error;
}


int EscribirPantalla (int entrada) {
  int cantidad=0, error=0;
  char frase[80];

  cantidad=read(entrada,frase,80);
  if (cantidad>0) {
    fprintf(stdout,"Se leyeron %d char del pipe\n",cantidad);
    fprintf(stdout,"Frase: %s",frase);
  }
  else {
    perror("pipe read");
    error=cantidad;
  }
  return error;
}


