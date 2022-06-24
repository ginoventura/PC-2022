#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>

#include "MonitoresSimple.h"

void *ProcesoHijo (void *mm);

int main () {
  int error=0, numero=0, cant=0;
  pthread_t hijo;
  struct Monitor_t *conexion=NULL;

  srand(time(NULL));

  conexion = CrearMonitor();
  if (conexion != NULL)
    printf("Monitor creado!\n");
  else {
    perror("CrearMonitor()");
    error = -1;
  }

  if (!error) {
    error = pthread_create(&hijo, NULL, ProcesoHijo, (void *)(conexion));
    if (error)
      perror("pthread_create()");
  }

  while ((!error) && (cant < 20)) {
    fflush(NULL);
    numero = ++cant == 20 ? -1 : rand() % 100;
    printf("Enviando dato [%d] a monitor\n", numero);
    error = GuardarDato(conexion, numero);
    if (error)
      perror("GuardarDato()");
  }

  if (!error) {
    error = pthread_join(hijo, NULL);
    if (error)
      perror("pthread_join()");
  }

  BorrarMonitor(conexion);

  return error;
}


void *ProcesoHijo (void *mm) {
  int numero=0, error=0;
  struct Monitor_t *m = (struct Monitor_t*)(mm);

  printf("Hilo HIJO [%p] creado!\n",(void *)(pthread_self())); 

  while ((!error) && (numero >= 0)) {
    error = LeerDato(m, &numero);
    if (!error) {
      if (numero > 0)
	printf("Se encontro [%d]\n", numero);
      fflush(NULL);
    }
    else
      perror("LeerDato()");
  }

  pthread_exit(NULL);
}

