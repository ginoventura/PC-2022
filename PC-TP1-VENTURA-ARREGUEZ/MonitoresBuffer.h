#ifndef _MONITORBUFFER_H_
#define _MONITORBUFFER_H_

struct Monitor_t {
  int inicio, fin;
  int *dato;
  int size;
  int estadoBuffer;
  pthread_cond_t  condEscritura, condLectura;
  pthread_mutex_t mutexEscritura, mutexLectura;
};

struct Monitor_t* CrearMonitor          (int tamano);
void              ReiniciarMonitor      (struct Monitor_t *m);
int               GuardarDato           (struct Monitor_t *m, int dato);
int               LeerDato              (struct Monitor_t *m, int *dato);
void              BorrarMonitor         (struct Monitor_t *m);
void              VerPedidos            (struct Monitor_t *m);

#endif