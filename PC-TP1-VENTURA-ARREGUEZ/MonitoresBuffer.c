#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>

#include "MonitoresBuffer.h"

struct Monitor_t* CrearMonitor (int tamano) {
  int error=0;
  struct Monitor_t *aux=NULL;

  aux = (struct Monitor_t*)(calloc(1, sizeof(struct Monitor_t)));

  if (aux != NULL) {
    aux->size = tamano;
    aux->dato = (int *)(calloc(aux->size, sizeof(int)));
    error += pthread_cond_init(&aux->condEscritura, NULL);
    if (error)
      perror("pthread_cond_init()");

    error += pthread_cond_init(&aux->condLectura, NULL);
    if (error)
      perror("pthread_cond_init()");

    error += pthread_mutex_init(&aux->mutexEscritura, NULL);
    if (error)
      perror("pthread_mutex_init()");

    error += pthread_mutex_init(&aux->mutexLectura, NULL);
    if (error)
      perror("pthread_mutex_init()");
  }

  pthread_cond_broadcast(&aux->condEscritura);
  pthread_cond_broadcast(&aux->condLectura);
  pthread_mutex_unlock(&aux->mutexEscritura);
  pthread_mutex_unlock(&aux->mutexLectura);

  return aux;
}

void ReiniciarMonitor (struct Monitor_t * m) {
  m->inicio = 0;
  m->fin = 0;
  m->estadoBuffer = 0;
  for(int i = 0; i < m->size; i++){
    m->dato[i] = 0;
  }
  pthread_cond_broadcast(&m->condEscritura);
  pthread_cond_broadcast(&m->condLectura);
  pthread_mutex_unlock(&m->mutexEscritura);
  pthread_mutex_unlock(&m->mutexLectura);
}

int GuardarDato (struct Monitor_t *m, int dato) {
  int error=0;

  error = pthread_mutex_lock(&m->mutexEscritura);
  if (error)
    perror("pthread_mutex_lock()");
  else
    while (m->estadoBuffer >= m->size)
      error = pthread_cond_wait(&m->condEscritura, &m->mutexEscritura);

  if (error)
    perror("pthread_cond_wait()");
  else {
    m->dato[m->fin] = dato;
    m->estadoBuffer++;
    pthread_cond_signal(&m->condLectura);
    m->fin = ++m->fin % m->size;
    pthread_cond_signal(&m->condEscritura);
  }

  if (error)
    perror("pthread_cond_signal()");
  else
    error = pthread_mutex_unlock(&m->mutexEscritura);

  if (error)
    perror("pthread_mutex_unlock()");

  return error;
 }

int LeerDato (struct Monitor_t *m, int *dato) {
  int error=0;

  error = pthread_mutex_lock(&m->mutexLectura);
  if (error)
    perror("pthread_mutex_lock()");
  else
    while (m->estadoBuffer <= 0)
      error = pthread_cond_wait(&m->condLectura, &m->mutexLectura);

  if (error)
    perror("pthread_cond_wait()");
  else {
    *dato = m->dato[m->inicio];
    m->estadoBuffer--;
    pthread_cond_signal(&m->condEscritura);
    m->inicio = ++m->inicio % m->size;
    pthread_cond_signal(&m->condLectura);
  }

  if (error)
    perror("pthread_cond_signal()");
  else
    error = pthread_mutex_unlock(&m->mutexLectura);

  if (error)
    perror("pthread_mutex_unlock()");

  return error;
}

void VerPedidos (struct Monitor_t *m) {
  for(int i=0; i<m->size; i++)
    printf("PEDIDO %d: %d\n", i+1, m->dato[i]);
}

void BorrarMonitor (struct Monitor_t *m) {
  if (m != NULL) {
    pthread_cond_destroy(&m->condEscritura);
    pthread_mutex_destroy(&m->mutexEscritura);
    pthread_cond_destroy(&m->condLectura);
    pthread_mutex_destroy(&m->mutexLectura);
    free(m->dato);
    free(m);
  }
}
