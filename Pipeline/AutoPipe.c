#include<stdio.h>
#include<unistd.h>
#include<string.h>

int main (int argc, char *argv[]) {
  int error=0, cantidad1=0, cantidad2=0, tubo[2];
  char buffer1[80], buffer2[80];

  error=pipe(tubo);
  if (!error) {
    fprintf(stdout,"Ingrese una frase: ");
    fgets(buffer1,80,stdin);
    cantidad1=write(tubo[1],buffer1,strlen(buffer1)+1);
    fprintf(stdout,"Se enviaron %d char al pipe\n",cantidad1);

    cantidad2=read(tubo[0],buffer2,80);
    fprintf(stdout,"Se leyeron %d char del pipe\n",cantidad2);
    fprintf(stdout,"Frase: %s - %d %d",buffer2, buffer2[cantidad2-1],'\0'); 
    close(tubo[0]);
    close(tubo[1]);
  }
  else {
    perror("pipe");
  }

  return error;
}

