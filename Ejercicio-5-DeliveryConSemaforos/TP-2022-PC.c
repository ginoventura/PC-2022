#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <pthread.h>

#include "MonitorRestaurante.h"

int cierre = 0, juego = 1;

/* Estructura para hacer split de los semaforos 
   y poder intercambiar pedidos entre el hiloCocinero
   y el hiloPedido */
typedef struct {
  sem_t *sem1, *sem2;
  int codigo_pedido, id;
} SplitSemaforo;

/* Estructura para que los procesos y los hilos de 
   cada proceso se compartan los semaforos, mutexs
   y variables */
typedef struct {
  SplitSemaforo pedido;
  SplitSemaforo atender;
  SplitSemaforo enviar;
  SplitSemaforo cobrar;
  SplitSemaforo entregar;
  sem_t *pedido_esperando, *cantidad_pedidos;
  pthread_mutex_t mtx_cocineros[3];
  pthread_mutex_t mtx_deliverys[2];
  int cocineros_libre[3];
  int deliverys_libre[2];
  double pago_pedido;
  struct Monitor_t monitor_cobrar;
} Restaurante;

/* Estructura del cocinero */
typedef struct {
  int idCocinero;
  Restaurante *Restaurante;
} Cocineros;

/* Estructura del delivery */
typedef struct {
  int idDelivery;
  Restaurante *Restaurante;
} Deliverys;

/* Estructura del pedido */
typedef struct {
  int idPedido;
  Restaurante *Restaurante;
} Pedido;

/* ------ FUNCIONES DEL JUEGO ------*/

int inicializarRecursos(Restaurante *rc);
void liberarRecursos(Restaurante *rc);

void ProcesoTelefono (Restaurante *);
void ProcesoCocinero (Restaurante *);
void ProcesoDelivery (Restaurante *);
void ProcesoEncargado (int, int, int, Restaurante *);

void *Menu_Principal (void *);
void *HiloCocinero (void *);
void *HiloPedido (void *);
void *HiloDelivery (void *);

void cerrarRestaurante (int);

void ingresarPedido (SplitSemaforo *, int, int);
int sacarPedido (SplitSemaforo *, int *);
void ingresarID (SplitSemaforo *, int);
int sacarID (SplitSemaforo *);

/* ------ MAIN ------ */
int main () {

  int PID1, PID2, PID3, error = 0;
  Restaurante *Restaurante = NULL;
  int memoria_compartida = 0;

  srand(time(NULL));

  /* Inicializamos la memoria compartida para que los procesos compartan
     la estructura restaurante y se puedan comunicar. */
  memoria_compartida = shm_open("/memCompartida", O_CREAT | O_RDWR, 0660);
  ftruncate(memoria_compartida, sizeof(int));
  Restaurante = mmap(NULL, sizeof(Restaurante), PROT_READ | PROT_WRITE, MAP_SHARED, memoria_compartida, 0);

  // Se crean los recursos necesarios junto a la inicializacion de variables.
  error = inicializarRecursos(Restaurante);

  // Creacion de los procesos de cocinero(hijo), telefono (hijo), delivery(hijo) y encargado (padre)
  if (!error) {
      PID1 = fork();
        if (PID1 == 0) {
          ProcesoCocinero(Restaurante);
        }
        else if (PID1 > 0) {
          PID2 = fork();
            if (PID2 == 0) {
              ProcesoTelefono(Restaurante);  
            }
        }
        else if (PID2 > 0) {
          PID3 = fork();
            if (PID3 == 0) {
              ProcesoDelivery(Restaurante);
            }            
        }
        else if (PID3 > 0) {
          ProcesoEncargado(PID1, PID2, PID3, Restaurante);
        }
        else {
          error = PID3;
        }
      }
  else {
    printf("Ha ocurrido un error al iniciar los recursos\n");
  }

  /*Espera a los procesos para que terminan */
  wait(NULL);
  wait(NULL);
  wait(NULL);

  /* Eliminacion de los recursos iniciados */
  liberarRecursos(Restaurante);

  /* Liberacion de la memoria compartida */
  munmap((void *)(Restaurante), 2 * sizeof(Restaurante));    
  shm_unlink("/memCompartida");

  printf("\n");
  return 0;
}

/* Utilizando el struct de Restaurante que tienen en comun los
   procesos, se crean e inicializan los semaforos y mutexs. Y
   el monitor se inicializa.
   Si alguno no se puede crear, se devuelve un error con -1*/

int inicializarRecursos (Restaurante *rc) {
  int error = 0, i;

  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);

  rc->atender.sem1 = sem_open("/semAtender1", O_CREAT, O_RDWR, 1);
  if (rc->atender.sem1 == SEM_FAILED) {
    perror("sem_open()");
    error = -1;
  }
  if (error != -1) {
    rc->atender.sem2 = sem_open("/semAtender2", O_CREAT, O_RDWR, 0);
  }
   if (rc->atender.sem2 == SEM_FAILED) {
    perror("sem_open()");
    error = -1;
  }

  if (error != -1) {
    rc->pedido_esperando = sem_open("/semPedidoEsperando", O_CREAT, O_RDWR, 0);
  }
  if (rc->pedido_esperando == SEM_FAILED) {
    perror("sem_open()");
    error = -1;
  }

  if (error != -1) {
    rc->pedido.sem1 = sem_open("/semPedido1", O_CREAT, O_RDWR, 1);
  }
  if (rc->pedido.sem1 == SEM_FAILED) {
    perror("sem_open()");
    error = -1;
  }

  if (error != -1) {
    rc->pedido.sem2 = sem_open("/semPedido2", O_CREAT, O_RDWR, 0);
  }
  if (rc->pedido.sem2 == SEM_FAILED) {
    perror("sem_open()");
    error = -1;    
  }

  if (error != -1) {
    rc->cobrar.sem1 = sem_open("/semCobrar1", O_CREAT, O_RDWR, 1);
  }
  if (rc->cobrar.sem1 == SEM_FAILED) {
    perror("sem_open()");
    error = -1;
  }

  if (error != -1) {
    rc->cobrar.sem2 = sem_open("/semCobrar2", O_CREAT, O_RDWR, 0);
  }
  if (rc->cobrar.sem2 == SEM_FAILED) {
    perror("sem_open()");
    error = -1;
  }

  if (error != -1) {
    rc->entregar.sem2 = sem_open("/entregar1", O_CREAT, O_RDWR, 0);
  }
  if (rc->cobrar.sem2 == SEM_FAILED) {
    perror("sem_open()");
    error = -1;
  }

  if (error != -1) {
    rc->entregar.sem2 = sem_open("/entregar2", O_CREAT, O_RDWR, 0);
  }
  if (rc->cobrar.sem2 == SEM_FAILED) {
    perror("sem_open()");
    error = -1;
  }

  if (error != -1) {
    rc->cantidad_pedidos = sem_open("/semCantPedido", O_CREAT, O_RDWR, 0);
  }
  if (rc->cantidad_pedidos == SEM_FAILED) {
        perror("sem_open()");
        error = -1;
  }

  for (i = 0; i < 3; i++) {
    pthread_mutex_init(&rc->mtx_cocineros[i], &attr);
    pthread_mutex_lock(&rc->mtx_cocineros[i]);
  }

  IniciarMonitor(&rc->monitor_cobrar);

  return error;
}

void liberarRecursos (Restaurante *rc) {
  sem_close(rc->cantidad_pedidos);
  sem_close(rc->pedido_esperando);    
  sem_close(rc->pedido.sem1);
  sem_close(rc->pedido.sem2);
  sem_close(rc->atender.sem1);
  sem_close(rc->atender.sem2);
  sem_close(rc->entregar.sem1);
  sem_close(rc->entregar.sem2);
  sem_close(rc->cobrar.sem1);
  sem_close(rc->cobrar.sem2);

  sem_unlink("/semCantPedido");
  sem_unlink("/semPedidoEsperando");
  sem_unlink("/semPedido1");
  sem_unlink("/semPedido2");
  sem_unlink("/semAtender1");
  sem_unlink("/semAtender2");
  sem_unlink("/semEntregar1");
  sem_unlink("/semEntregar2");
  sem_unlink("/semCobrar1");
  sem_unlink("/semCobrar2");

  for (int i = 0; i < 3; i++) {
        pthread_mutex_destroy(&rc->mtx_cocineros[i]);
  }

    for (int i = 0; i < 2; i++) {
        pthread_mutex_destroy(&rc->mtx_deliverys[i]);
  }

  BorrarMonitor(&rc->monitor_cobrar);
}

void ProcesoEncargado (int PID1, int PID2, int PID3, Restaurante *Restaurante) {
  int opcion, cant_pedidos = 0, cantidad_pedidos_caja = 0, id_pedido = 0;
  pthread_t menu;

  pthread_create(&menu, NULL, Menu_Principal, Restaurante);

  do {
    do {
      //Ingreso de una opcion.
      scanf("%d", &opcion);
        if (opcion < 1 || opcion > 5) {
          printf("Opcion incorrecta, por favor selecciona una nueva opcion.");
        }
    } while (opcion < 1 || opcion > 5);

    /*Se asigna el cocinero elegido*/
    if (opcion == 1 || opcion == 2 || opcion == 3) {
      sem_getvalue(Restaurante->cantidad_pedidos, &cant_pedidos);
        if (cant_pedidos > 0) {
          if (Restaurante->cocineros_libre[opcion -1]) {
            printf("Cocinero %d asignado\n", opcion -1);
            printf("\n");
            pthread_mutex_unlock(&Restaurante->mtx_cocineros[opcion - 1]);
            Restaurante->cocineros_libre[opcion - 1] = 0;
          }
          else {
            printf("El cocinero %d esta ocupado\n", opcion -1);
            printf("\n");
          }
        }
        else {
          printf("\n");
          printf("No hay pedidos para cocinar\n");
          printf("\n");
        }
      }

    //El encargado cobra los pedidos
    if (opcion == 4) {
      if (Restaurante->monitor_cobrar.cantidad > 0) {
        printf("Cobrando pedido\n");

      /*El dato es extraido del buffer circular 
        una vez que el Cocinero libero al cliente, 
        entonces se toma el id_pedido liberado */

        id_pedido = LeerDato(&Restaurante->monitor_cobrar);
        printf("Cobrando pedido %d.", id_pedido);

      /*El encargado le cobra al cliente*/
      sem_wait(Restaurante->cobrar.sem2);
      printf("Se recibio un pago de $ %f\n", Restaurante->pago_pedido);
      sem_post(Restaurante->cobrar.sem1);
      }
      else {
        printf("No hay pedidos pendientes de cobro.\n");
      }
    }
    
    if (opcion == 5) {
      juego = 0;
      kill(PID1, SIGTERM);
      kill(PID2, SIGTERM);
    }
  } while (opcion != 5);
}

// El Menu_Principal es lanzado por el ProcesoEncargado, se encarga 
// de mostrar al usuario cierta informacion para que le permita 
// tomar decisiones en el juego.

void *Menu_Principal(void *temp) {
  Restaurante *rc = (Restaurante *)temp;
  int cantidad_pedidos = 0, cantidad_pedidos_caja = 0, cantidad_pedidos_entregar = 0, i;

  while (juego) 
  {
    sleep(3);
    sem_getvalue(rc->cantidad_pedidos, &cantidad_pedidos);
    printf("Pedidos esperando ser atendidos: %d\n", cantidad_pedidos);
      printf("Cocineros desocupados: \n");
    for (i=0; i<3; i++) {
      if (rc->cocineros_libre[i])
      {
        printf("Cocinero Nº%d\n", i+1);
      }
    }
    printf("Pedidos esperando ser entregados: %d\n", cantidad_pedidos_entregar);
    printf("Deliverys desocupados: \n");
    for (i=0; i<2; i++) {
      if (rc->deliverys_libre[i])
      {
        printf("Delivery Nº%d\n", i+1);
      }
    }
    
    cantidad_pedidos_caja = rc->monitor_cobrar.cantidad;
    printf("Pedidos esperando en caja: %d\n", cantidad_pedidos_caja);
    printf("\n");
    printf("Elija al cocinero para atender el pedido: \n");
    printf("1. Cocinero 0\n");
    printf("2. Cocinero 1\n");
    printf("3. Cocinero 2\n");
    printf("\n");
    printf("4. Cobrar pedido\n");
    printf("\n");
    printf("5. Salir del juego\n");
    printf("\n");

    cantidad_pedidos_entregar= rc->monitor_cobrar.cantidad;
    printf("Pedidos pendientes de entrega: %d\n", cantidad_pedidos_entregar);
    printf("\n");
    printf("Elija al delivery para entregar el pedido: \n");
    printf("1. Delivery 0\n");
    printf("2. Delivery 1\n");
    printf("\n");
    printf("5. Salir del juego\n");
    printf("\n");
  }
  pthread_exit(NULL);
}

// El proceso cocinero se ocupa de lanzar el hilo de cada cocinero
// y de inicializarlos.
void ProcesoCocinero (Restaurante *Restaurante) {
  int i;
  pthread_t cocinero[3];
  Cocineros cocineros[3];

  for (i=0; i<3; i++) {
    cocineros[i].idCocinero = i;
    cocineros[i].Restaurante = Restaurante;
    Restaurante->cocineros_libre[i] = 1;
    pthread_create(&cocinero[i], NULL, HiloCocinero, &cocineros[i]);
  }
  for (i=0; i<3; i++) {
    pthread_join(cocinero[i], NULL);
  }
  if (Restaurante != NULL) {
    munmap((void *)(Restaurante), 2 * sizeof(Restaurante));
  }
}

// El proceso delivery se ocupa de lanzar el hilo de cada delivery
// y de inicializarlos.
void ProcesoDelivery (Restaurante *Restaurante) {
  int i;
  pthread_t delivery[2];
  Deliverys deliverys[2];

  for (i=0; i<2; i++) {
    deliverys[i].idDelivery = i;
    deliverys[i].Restaurante = Restaurante;
    Restaurante->deliverys_libre[i] = 1;
    pthread_create(&delivery[i], NULL, HiloDelivery, &deliverys[i]);
  }
  for (i=0; i<3; i++) {
    pthread_join(delivery[i], NULL);
  }
  if (Restaurante != NULL) {
    munmap((void *)(Restaurante), 2 * sizeof(Restaurante));
  }
}

// El proceso telefono se ocupa de lanzar los hilos pedidos 
// hasta que suene una alarma
void ProcesoTelefono(Restaurante *rc) {
  signal(SIGALRM, cerrarRestaurante);
  pthread_t hilosPedidos[30];
  Pedido pedidos[30];

  int id = 0, terminar = 20;

  alarm(terminar);

  while (!cierre) {
    sleep(5);
    pedidos[id].idPedido = id;
    pedidos[id].Restaurante = rc;
    pthread_create(&hilosPedidos[id], NULL, HiloPedido, &pedidos[id]);
  }

  for (int i=0; i<id; i++) {
    pthread_join(hilosPedidos[i], NULL);
  }

  if (rc != NULL) {
    munmap((void*)(rc), 2 * sizeof(Restaurante));
  }
}

// cerrarRestaurante es la funcion que indica el ultimo cliente
// y cierra el proceso para que no lleguen mas pedidos.
void cerrarRestaurante(int sig_num) {
  printf("\n");
  printf("ULTIMO PEDIDO!\n");
  printf("\n");
  cierre = 1;
}

// Hilo cocinero son los hilos que se ocupan de atender a los 
// pedidos que reciben del proceso encargado.
void *HiloCocinero(void *temp) {
  Cocineros *rc = (Cocineros *)temp;
  char *carta[6] = {"", "Pizza", "Hamburguesa", "Empanadas", "Lomito", "Sandwich de Milanesa"};
  int codigo_pedido = 0, pago = 0, id_pedido = 0;

  while (juego) {
    
    //Si se eligio el cocinero
    if (rc->Restaurante->cocineros_libre[rc->idCocinero] == 0) {
      
      //Entonces el cocinero espera un pedido
      sem_post(rc->Restaurante->pedido_esperando);

      //Obtengo el id_pedido y el codigo de cliente por medio del semaforo
      codigo_pedido = sacarPedido(&rc->Restaurante->pedido, &id_pedido);

      printf(" Cocinero %d prepara el pedido %d: %s\n", rc->idCocinero, id_pedido, carta[codigo_pedido]);

      //Se ingresa el id del cocinero por medio del semaforo de atender.
      ingresarID(&rc->Restaurante->atender, rc->idCocinero);

      //Demora tiempo en preparar, segun el pedido.
      sleep(codigo_pedido);

      //Se desbloquea al cocinero segun el ID.
      pthread_mutex_unlock(&rc->Restaurante->mtx_cocineros[rc->idCocinero]);

      printf("\n");
      printf("Pedido %d listo para cobrar.\n", id_pedido);
      printf("\n");

      //Almacenamos el id_pedido en el buffer circular implementado con monitor
      GuardarDato(&rc->Restaurante->monitor_cobrar, id_pedido);
      
      //Se libera el cocinero y queda esperando la proxima asignacion
      rc->Restaurante->cocineros_libre[rc->idCocinero] = 1;
      pthread_mutex_lock(&rc->Restaurante->mtx_cocineros[rc->idCocinero]);
    }
  }
  pthread_exit(NULL);
}

// Hilo cocinero son los hilos que se ocupan de atender a los 
// pedidos que reciben del proceso encargado.
void *HiloDelivery(void *temp) {
  Deliverys *rc = (Deliverys *)temp;
  int codigo_pedido = 0, pago = 0, id_pedido = 0;

  while (juego) {
    
    //Si se eligio el delivery
    if (rc->Restaurante->deliverys_libre[rc->idDelivery] == 0) {
      
      //Entonces el delivery espera un pedido
      sem_post(rc->Restaurante->pedido_esperando);

      //Obtengo el id_pedido y el codigo de cliente por medio del semaforo
      codigo_pedido = sacarPedido(&rc->Restaurante->pedido, &id_pedido);

      printf("Delivery %d entrega el pedido %d: %s\n", rc->idDelivery, id_pedido);

      //Se ingresa el id del delivery por medio del semaforo de entregar.
      ingresarID(&rc->Restaurante->entregar, rc->idDelivery);

      //Demora tiempo en entregar, segun la distancia
      sleep(codigo_pedido);

      //Se desbloquea al delivery segun el ID.
      pthread_mutex_unlock(&rc->Restaurante->mtx_deliverys[rc->idDelivery]);

      printf("\n");
      printf("Pedido %d listo para cobrar.\n", id_pedido);
      printf("\n");

      //Almacenamos el id_pedido en el buffer circular implementado con monitor
      GuardarDato(&rc->Restaurante->monitor_cobrar, id_pedido);
      
      //Se libera el delivery y queda esperando la proxima asignacion
      rc->Restaurante->deliverys_libre[rc->idDelivery] = 1;
      pthread_mutex_lock(&rc->Restaurante->mtx_deliverys[rc->idDelivery]);
    }
  }
  pthread_exit(NULL);
}

//Hilo pedido se ocupa de realizar los pedidos pero si el 
//restaurante tiene cierta cantidad de pedidos pendientes o 
//pasaron un tiempo sin ser atendidos se cancelan.
void *HiloPedido(void *temp) {
  Pedido *pedidos = (Pedido *)temp;

  int cancelar = 0;
  int cantidad_pedidos = 0, id_cocinero = 0;
  int codigo_pedido = 1 + rand() % 5;
  char *carta[6] = {"", "Pizza", "Hamburguesa", "Empanadas", "Lomito", "Sandwich de Milanesa"};

  struct timespec espera;
  clock_gettime(CLOCK_REALTIME, &espera);

  espera.tv_sec += 8;

  //Se verifica cuantos pedidos hay en la cola y si son 5, se cancela
  sem_getvalue(pedidos->Restaurante->cantidad_pedidos, &cantidad_pedidos);

  if (cantidad_pedidos == 5) {
    printf("El pedido %d se cancela por limite de pedidos pendientes.\n", pedidos->idPedido);
    sem_wait(pedidos->Restaurante->cantidad_pedidos);
    pthread_exit(NULL);
  }

  //Aumenta en 1 el semaforo de cantidad de pedidos
  sem_post(pedidos->Restaurante->cantidad_pedidos);
  printf("\n");
  printf("El pedido %d espera ser atendido... \n", pedidos->idPedido);

  cancelar = sem_timedwait(pedidos->Restaurante->pedido_esperando, &espera);

  if (cancelar == 0) {
    
    //El cliente ingresa el pedido y su id.
    ingresarPedido(&pedidos->Restaurante->pedido, codigo_pedido, pedidos->idPedido);

    //Tomamos el ID del cocinero que lo va a atender para poder bloquear al mismo.
    id_cocinero = sacarID(&pedidos->Restaurante->atender);
    pthread_mutex_lock(&pedidos->Restaurante->mtx_cocineros[id_cocinero]);

    //Se calcula el monto del pedido segun el codigo del pedido.
    pedidos->Restaurante->pago_pedido = (codigo_pedido * 50);

    //Se le cobra al cliente con un semaforo
    sem_wait(pedidos->Restaurante->cobrar.sem1);
    pedidos->Restaurante->pago_pedido = (codigo_pedido * 50);
    sem_post(pedidos->Restaurante->cobrar.sem2);
  }
  //Si el tiempo del timedwait se pasa, el cliente se va.
  else {
    printf("\n");
    printf("Pedido numero %d cancelado por que no se atendio.\n", pedidos->idPedido);
    printf("\n");
  }
  
  //Se resta 1 en el semaforo de la cantidad de pedidos por el pedido cancelado
  sem_wait(pedidos->Restaurante->cantidad_pedidos);

  pthread_exit(NULL);
}

//ingresarPedido: se deposita el pedido y el id de quien realizo el 
// pedido para que otra funcion pueda utilizar esa informacion.
void ingresarPedido(SplitSemaforo *sem, int cod_pedido, int id) {
  sem_wait(sem->sem1);
  sem->codigo_pedido = cod_pedido;
  sem->id = id;
  sem_post(sem->sem2);
}

//sacarPedido: extrae el pedido y la id de lo depositado en la funcion 
//ingresarPedido.
int sacarPedido(SplitSemaforo *sem, int *id) {
  int cod_pedido;
  sem_wait(sem->sem2);
  cod_pedido = sem->codigo_pedido;
  *id = sem->id;
  sem_post(sem->sem1);
  return cod_pedido;
}

//ingresarID: deposita la id por medio de un semaforolibre
void ingresarID(SplitSemaforo *sem, int id) {
  
  sem_wait(sem->sem1);
  sem->id = id;
  sem_post(sem->sem2);
}

int SacarID(SplitSemaforo *sem) {
  int id;
  sem_wait(sem->sem2);
  id = sem->id;
  sem_post(sem->sem1);

  return id;
}