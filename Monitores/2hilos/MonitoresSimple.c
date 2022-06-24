#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>

#include "MonitoresSimple.h"

struct Monitor_t* CrearMonitor  () {
  int error=0;
  struct Monitor_t *aux=NULL;

  aux = (struct Monitor_t*)(calloc(1, sizeof(struct Monitor_t)));

  if (aux != NULL) {
    aux->ready=0;
    error += pthread_cond_init(&aux->cond, NULL);
    if (error)
      perror("pthread_cond_init()");

    error += pthread_mutex_init(&aux->mutex, NULL);
    if (error)
      perror("pthread_mutex_init()");

    pthread_cond_broadcast(&aux->cond);
    pthread_mutex_unlock(&aux->mutex);
  }

  return aux;
}


int GuardarDato (struct Monitor_t *m, int dato) {
  int error=0;

  error = pthread_mutex_lock(&m->mutex);
  if (error)
    perror("pthread_mutex_lock()");
  else
    while (m->ready)
      error = pthread_cond_wait(&m->cond, &m->mutex);

  if (error)
    perror("pthread_cond_wait()");
  else {
    m->ready = 1;
    m->dato = dato;
    pthread_cond_signal(&m->cond);
  }

  if (error)
    perror("pthread_cond_signal()");
  else
    error = pthread_mutex_unlock(&m->mutex);

  if (error)
    perror("pthread_mutex_unlock()");

  return error;
 }


int LeerDato (struct Monitor_t *m, int *dato) {
  int error=0;

  error = pthread_mutex_lock(&m->mutex);
  if (error)
    perror("pthread_mutex_lock()");
  else
    while (!m->ready)
      error = pthread_cond_wait(&m->cond, &m->mutex);

  if (error)
    perror("pthread_cond_wait()");
  else {
    m->ready = 0;
    *dato = m->dato;
    pthread_cond_signal(&m->cond);
  }

  if (error)
    perror("pthread_cond_signal()");
  else
    error = pthread_mutex_unlock(&m->mutex);

  if (error)
    perror("pthread_mutex_unlock()");


  return error;
 }


void BorrarMonitor (struct Monitor_t *m) {
  if (m != NULL) {
    pthread_cond_destroy(&m->cond);
    pthread_mutex_destroy(&m->mutex);
    free(m);
  }
}

