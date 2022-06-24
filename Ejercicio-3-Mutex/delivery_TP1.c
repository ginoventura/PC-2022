#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/mman.h>

// LIBRERIAS PROPIAS
#include "MonitoresBuffer.h"

// COLORES
#define RESET_COLOR  "\x1b[0m"
#define NEGRO_T      "\x1b[30m"
#define ROJO_F       "\x1b[41m"
#define VERDE_F      "\x1b[42m"
#define AMARILLO_F   "\x1b[43m"
#define MAGENTA_F    "\x1b[45m"
#define BLANCO_T     "\x1b[37m"
#define BLANCO_F     "\x1b[47m"

// DATOS DEL JUEGO
#define ENCARGADOS 1
#define COCINEROS 3
#define DELIVERIES 2
#define BUFFERCOMANDAS 4
#define BUFFERPEDIDOS 4
#define CARTA 5
#define ALARMA 10
#define TIEMPOLLAMADA 1
#define ULTIMOPEDIDO -1
#define PEDIDOSPORCARGAR 10

int timeout = 1; // INDICA CUANDO EL JUEGO VA A TERMINAR

/*----------------------------------------------------------------------------*/
/*---------------------------------ESTRUCTURAS--------------------------------*/
// ESTRUCTURA DE LA MEMORIA
typedef struct {
  sem_t * semaforoPedidosPorCobrar;
  sem_t * semaforoDejarDinero;
  sem_t * semaforoCobrarDinero;
  int dato;
}Memoria;

// ESTRUCTURA DEL TELEFONO
typedef struct {
  sem_t * semaforoTelefono;
  sem_t * semaforoLlamadas;
  int pedido;
  int * puntuacion;
}Telefono;

// ESTRUCTURA DEL ENCARGADO
typedef struct {
  Telefono * telefono;
  Memoria * memoria;
  struct Monitor_t *monitorComandas;
  int ubiMemoria;
  int pedidos[PEDIDOSPORCARGAR]; // UTILIZADO PARA PODER TENER MAS DE UNA COMANDA EN LA MANO ANTES DE CARGARLO EN EL BUFFER, Y ASI PODER ATENDER EL TELEFONO VARIAS VECES ANTES DE CARGAR LOS PEDIDOS EN EL MONITOR DE COMANDAS
  float precios[CARTA];
}Encargado;

// ESTRUCTURA DEL COCINERO
typedef struct {
  struct Monitor_t *monitorComandas;
  struct Monitor_t *monitorPedidos;
  int cantCocineros;
}Cocinero;

// ESTRUCTURA DEL DELIVERY
typedef struct {
  struct Monitor_t *monitorPedidos;
  int cantDeliveries;
  int ubiMemoria;
  Memoria * memoria;
}Delivery;

/*----------------------------------------------------------------------------*/
/*-----------------------FUNCIONES DE INICIALIZACION--------------------------*/
// ACTORES
Telefono * crearTelefono(int *);
Encargado * crearEncargado(Telefono *, struct Monitor_t *, int);
Cocinero * crearCocinero(struct Monitor_t *, struct Monitor_t *);
Delivery * crearDelivery(struct Monitor_t *, int);

// MEMORIA
int crearMemoria();
void llenarMemoria(int);

/*----------------------------------------------------------------------------*/
/*-------------------------FUNCIONES DEL JUGADOR------------------------------*/
void mostrarMenu();
int comenzarJuego();
void jugar();
void guardarPuntuacion(int);
void verPuntuacion();
void salir(int *);

/*----------------------------------------------------------------------------*/
/*---------------------FUNCIONES DE ACTORES DEL JUEGO-------------------------*/
// FUNCIONES DEL TELEFONO
void * gestionTelefono( void *);
void recibirLlamada(Telefono * telefono);
void TimeOut();

// FUNCIONES DEL ENCARGADO
void atenderPedido(Encargado *);
void cargarPedido(Encargado *);
void cobrarPedido(Encargado *, int *);

// FUNCIONES DEL COCINERO
void * gestionCocinero(void *);
void cocinarPedido(Cocinero *, int *);
void pedidoCocinado(Cocinero *);
void pedidoListo(Cocinero *, int);

//FUNCIONES DEL COCINERO
void * gestionDelivery(void *);
void repartirPedido(Delivery *, int *);
void avisarCobro(Delivery *, int);


/*----------------------------------------------------------------------------*/
/*-------------------FUNCIONES DE LIBREACION DE MEMORIA-----------------------*/
// BORRADO DE SEMAFOROS Y MEMORIA
void borrarSemMem(Encargado *, int);

/*-----------------------------------------------------------------------------*/
/*----------------------------------MAIN---------------------------------------*/
int main(){
  srand(time(NULL));

  int terminar = 1;
  char eleccion;
  do
  {
    mostrarMenu(); // INTERFAZ DE USUARIO

    do{
      eleccion = getchar();
      __fpurge(stdin);
      if(eleccion != '1' && eleccion != '2' && eleccion != '3') {
        printf("\nOpcion invalida, por favor seleccion 1 2 o 3\nIngrese una opcion: ");
      }
    }while(eleccion != '1' && eleccion != '2' && eleccion != '3');


    switch (eleccion)
    {
    case '1':
        system("clear");
        int puntuacion = comenzarJuego();;
        guardarPuntuacion(puntuacion);
        break;
    case '2':
        system("clear");
        verPuntuacion();
        break;
    case '3':
        system("clear");
        salir(&terminar);
        break;
    default:
      break;
    }
  } while (terminar);

  return 0;
}

void mostrarMenu() {
  system("clear");
  printf(NEGRO_T BLANCO_F"|------------------------------------|"RESET_COLOR"\n");
  printf(NEGRO_T BLANCO_F"|-------------"BLANCO_T MAGENTA_F" PIZZERIA "NEGRO_T BLANCO_F"-------------|"RESET_COLOR"\n");
  printf(NEGRO_T BLANCO_F"|                                    |"RESET_COLOR"\n");
  printf(NEGRO_T BLANCO_F"|         "NEGRO_T BLANCO_F"1. Comenzar juego."NEGRO_T BLANCO_F"         |"RESET_COLOR"\n");
  printf(NEGRO_T BLANCO_F"|         "NEGRO_T BLANCO_F"2. Ver puntuacion."NEGRO_T BLANCO_F"         |"RESET_COLOR"\n");
  printf(NEGRO_T BLANCO_F"|         "NEGRO_T BLANCO_F"3. Salir."NEGRO_T BLANCO_F"                  |"RESET_COLOR"\n");
  printf(NEGRO_T BLANCO_F"|                                    |"RESET_COLOR"\n");
  printf(NEGRO_T BLANCO_F"|************************************|"RESET_COLOR"\n");
  printf(NEGRO_T BLANCO_F"|             "BLANCO_T MAGENTA_F"Como jugar"NEGRO_T BLANCO_F"             |"RESET_COLOR"\n");
  printf(NEGRO_T BLANCO_F"|                                    |"RESET_COLOR"\n");
  printf(NEGRO_T BLANCO_F"|         "NEGRO_T BLANCO_F"a: Atender telefono"NEGRO_T BLANCO_F"        |"RESET_COLOR"\n");
  printf(NEGRO_T BLANCO_F"|         "NEGRO_T BLANCO_F"s: Cargar pedido"NEGRO_T BLANCO_F"           |"RESET_COLOR"\n");
  printf(NEGRO_T BLANCO_F"|         "NEGRO_T BLANCO_F"d: Cobrar pedido"NEGRO_T BLANCO_F"           |"RESET_COLOR"\n");
  printf(NEGRO_T BLANCO_F"|                                    |"RESET_COLOR"\n");
  printf(NEGRO_T BLANCO_F"|------------------------------------|"RESET_COLOR"\n");
  printf("Ingrese una opcion: ");
}

int comenzarJuego(){
  timeout = 1;
  int puntuacion = 0; // Guarda la puntuacion del juego

  // Creamos los monitores
  struct Monitor_t * monitorComandas = CrearMonitor(BUFFERCOMANDAS);
  struct Monitor_t * monitorPedidos = CrearMonitor(BUFFERPEDIDOS);

  // Creamos la memoria y traemos su ubiacion
  int memoria =  crearMemoria();

  // Se crean los actores del juego
  Telefono * telefono = crearTelefono(&puntuacion);
  Encargado * encargado = crearEncargado(telefono, monitorComandas, memoria);
  Cocinero * cocinero = crearCocinero(monitorComandas, monitorPedidos);
  Delivery * delivery = crearDelivery(monitorPedidos, memoria);

  // Se instancian las variables hilos de cada objeto
  pthread_t hiloTelefono;
  pthread_t hilosCocineros[COCINEROS];
  pthread_t hilosDeliveries[DELIVERIES];

  // Se crean los hilos de cada objeto
  pthread_create(&hiloTelefono, NULL, gestionTelefono, (void *)(telefono));
  for(int i = 0; i < COCINEROS; i++) {
    pthread_create(&hilosCocineros[i], NULL, gestionCocinero, (void *)(cocinero));
  }
  for(int i = 0; i < DELIVERIES; i++) {
    pthread_create(&hilosDeliveries[i], NULL, gestionDelivery, (void *)(delivery));
  }

  // Se mapea la memoria del encargado
  encargado->memoria = mmap(NULL, sizeof(Memoria), PROT_READ | PROT_WRITE, MAP_SHARED, encargado->ubiMemoria, 0);

  // Arranca el ciclo de juego del encargado, donde se detecta las teclas que presiona
  jugar(encargado);

  // Se desmapea la memoria del encargado
  if (encargado->memoria != NULL) {
    int error = munmap((void*)(encargado->memoria), 2 * sizeof(Memoria));
    if (error)
      perror("encargado_munmap()");
  }

  // Se espera que terminen todos los hilos
  pthread_join(hiloTelefono, NULL);
  for(int i = 0; i < COCINEROS; i++)
    pthread_join(hilosCocineros[i], NULL);
  for(int i = 0; i < DELIVERIES; i++)
    pthread_join(hilosDeliveries[i], NULL);

  // Se liberan los semaforos
  borrarSemMem(encargado, memoria);

  // Borramos los monitores
  BorrarMonitor(monitorComandas);
  BorrarMonitor(monitorPedidos);

  // Se libera la memoria de los objetos creados
  free(telefono);
  free(encargado);
  free(cocinero);
  free(delivery);

  return puntuacion;
}

void jugar(Encargado * encargado){
  int * terminado = (int *)(calloc(1, sizeof(int)));
  char eleccion;
  while(* terminado != -1){
    do
    {
      eleccion = getchar();
    } while (eleccion != 'a' && eleccion != 's' && eleccion != 'd');
    switch (eleccion){
    case 'a':
      atenderPedido(encargado);
      break;
    case 's':
      cargarPedido(encargado);
      break;
    case 'd':
      cobrarPedido(encargado, terminado);
      break;
    default:
      break;
    }
  }
  free(terminado);
}

// Hilo Encargado
void atenderPedido(Encargado * encargado) {
  
  // Verifica si hay alguna llamada entrante
  int error = sem_trywait(encargado->telefono->semaforoLlamadas);
  if(!error) {
    printf("\t\ttelefono atendido\n");
    usleep(rand()% 100001 + 250000); // Tiempo que tarda en tomar el pedido
    int codigoPedido = encargado->telefono->pedido;

    for(int i = 0; i < PEDIDOSPORCARGAR; i++){
      if(encargado->pedidos[i] == -2){
        encargado->pedidos[i] = codigoPedido;
        printf(BLANCO_T MAGENTA_F"\t\tcomanda de pedido %d lista para cargar"RESET_COLOR"\n", codigoPedido);
        break;
      }
    }

    // Cuelga el telefono
    sem_post(encargado->telefono->semaforoTelefono);
  }
}

void cargarPedido (Encargado * encargado) {

  for(int  i = 0; i < PEDIDOSPORCARGAR; i++){
    int codigoPedido = encargado->pedidos[i];
    if(codigoPedido != -2 && codigoPedido != ULTIMOPEDIDO){
      encargado->pedidos[i] = -2;
      // Carga el pedido a los cocineros
      int error = GuardarDato(encargado->monitorComandas, codigoPedido);
      if(error)
        perror("GuardarDato()");
    }else if(codigoPedido != -2 && codigoPedido == ULTIMOPEDIDO){
      encargado->pedidos[i] = -2;
      for (int i = 0; i < COCINEROS; i++){
        // Si el que sigue es el ultimo pedido, avisa a cada cocinero que cierren la cocina
        int error =  GuardarDato(encargado->monitorComandas, codigoPedido);
        if(error)
          perror("GuardarDato()");
      }
    }
  }
}

void cobrarPedido(Encargado * encargado, int * terminado) {

  int cobrosPendientes = 0;
  // Se fija si hay algun delivery esperando para que le cobre
  int error = sem_getvalue(encargado->memoria->semaforoPedidosPorCobrar, &cobrosPendientes);
  if(!error) {
    if(cobrosPendientes > 0){
      sem_post(encargado->memoria->semaforoDejarDinero);
      sem_wait(encargado->memoria->semaforoCobrarDinero);
      int pedido = encargado->memoria->dato;

      // Si el delivery le avisa que ya termino, cierra el local
      if( pedido != ULTIMOPEDIDO)
        printf("\t\t$%.0f guardados de pedido %d\n", encargado->precios[pedido], pedido);
      else {
        printf("\t\tCerrando local\n");
        * terminado = -1;
      }
      sem_trywait(encargado->memoria->semaforoPedidosPorCobrar);
    }
  }
  else{
    perror("encargado_sem_getvalue()");
  }
}

// Hilo telefono
void * gestionTelefono(void * tmp){

  Telefono * telefono = (Telefono *) tmp;

  // Seteamos la alarma del juego e iniciamos el contador
  signal(SIGALRM, TimeOut);
  alarm(ALARMA);

  // Ciclo de recibir llamadas, finaliza cuando suena la alarma
  while (timeout)
    recibirLlamada(telefono);

  // Envia el último pedido
  sem_wait(telefono->semaforoTelefono);
  telefono->pedido= ULTIMOPEDIDO;
  printf(BLANCO_T ROJO_F"\tDueño llamando para cerrar local"RESET_COLOR"\n");
  sem_post(telefono->semaforoLlamadas);

  // Termina el hilo
  pthread_exit(NULL);
}

void recibirLlamada(Telefono * telefono) {

  // El telefono comienza a recibir llamadas mientras esta colgado
  sem_wait(telefono->semaforoTelefono);
  usleep(rand()% 750001 + 250000);
  telefono->pedido = rand() % CARTA;
  printf(BLANCO_T ROJO_F"\ttelefono sonando"RESET_COLOR"\n");

  // El telefono comienza a sonar
  sem_post(telefono->semaforoLlamadas);
  sleep(TIEMPOLLAMADA); // Tiempo que el cliente espera antes de cortar
  int telefonoSonando = 0;

  // Si el telefono no es atendido, se pierde la llamada
  sem_getvalue(telefono->semaforoLlamadas, &telefonoSonando);
  if(telefonoSonando > 0) {
    int error = sem_trywait(telefono->semaforoLlamadas);
    if(!error) {
      printf("\tSe perdio la llamada\n");
      sem_post(telefono->semaforoTelefono);
    }
  }
  else
    *telefono->puntuacion = *telefono->puntuacion+1; // Si se atiende, suma un punto
}

void TimeOut() {
  timeout = 0;
}

// Hilo Cocinero
void * gestionCocinero(void * tmp) {

  Cocinero *cocinero = (Cocinero *) tmp;
  int * terminado = (int *)(calloc(1, sizeof(int)));

  // Ciclo de cocinar pedidos, finaliza cuando el encargado avisa
  while(*terminado != -1)
    cocinarPedido(cocinero, terminado);

  free(terminado);

  // Termina el hilo
  pthread_exit(NULL);
}

void cocinarPedido(Cocinero * cocinero, int * terminado) {

  // Toma una comanda para empezar a cocinar
  int pedidoActual = 0;
  int error = LeerDato(cocinero->monitorComandas, &pedidoActual);
  if(error)
    perror("LeerDato()");
  else {
    // Comienza a cocinar
    if( pedidoActual != ULTIMOPEDIDO) {
      usleep(rand()% 500001 + 1000000); // Tiempo que se demora en cocinar
      pedidoListo(cocinero, pedidoActual);
    }
    // Recibe el aviso para terminar
    else {
      cocinero->cantCocineros--;
      * terminado = -1;
      // El ultimo cocinero es el encargado de avisar a los deliveries que ya cerró la cocina
      if(cocinero->cantCocineros == 0) {
        for(int i = 0; i < DELIVERIES; i++)
          pedidoListo(cocinero, ULTIMOPEDIDO);
      }
    }
  }
}

void pedidoListo(Cocinero * cocinero, int pedidoListo){
  
  // Deja el pedido en el mostrador
  int error = GuardarDato(cocinero->monitorPedidos, pedidoListo);
  if(error)
    perror("GuardarDato()");
}

// Hilo Delivery
void * gestionDelivery(void * tmp){

  Delivery * delivery = (Delivery *)(tmp);
  int * terminado = (int *)(calloc(1, sizeof(int)));

  // Se mapea la memoria compartida una sola vez
  if(delivery->memoria == NULL)
    delivery->memoria = mmap(NULL, sizeof(Memoria), PROT_READ | PROT_WRITE, MAP_SHARED, delivery->ubiMemoria, 0);

  // Ciclo de repartir pedidos
  while(*terminado != -1)
    repartirPedido(delivery, terminado);

  // Si es el ultimo delivery, se desmapea la memoria
  if(delivery->cantDeliveries == 0){
      if (delivery->memoria != NULL) {
        int error = munmap((void*)(delivery->memoria), 2 * sizeof(Memoria));
        if (error)
          perror("delivery_munmap()");
      }
  }

  free(terminado);

  // Termino el hilo
  pthread_exit(NULL);
}

void repartirPedido(Delivery * delivery, int * terminado) {
  int error = 0;
  int pedidoRepartir = 0;
  error = LeerDato(delivery->monitorPedidos, &pedidoRepartir);
  if(error)
    perror("LeerDato()");
  else {
    // Sale a repartir el pedido
    if( pedidoRepartir != ULTIMOPEDIDO) {
      usleep(rand()% 500001 + 500000);
      avisarCobro(delivery, pedidoRepartir);
    }
    // Recibe mensaje de un cocinero para que se retire
    else {
      delivery->cantDeliveries--;
      * terminado = -1;
      // Si es el ultimo delivery que termina, le avisa al encargado
      if(delivery->cantDeliveries == 0)
        avisarCobro(delivery, ULTIMOPEDIDO);
    }
  }
}

void avisarCobro(Delivery * delivery, int pedidoCobrar){

  // Avisa al encargado que esta listo para dejar dinero
  sem_post(delivery->memoria->semaforoPedidosPorCobrar);

  // Deja el dinero
  if(pedidoCobrar != ULTIMOPEDIDO) 
    printf(NEGRO_T AMARILLO_F"\t\t\t\tPedido %d listo para cobrar"RESET_COLOR"\n", pedidoCobrar);  
  // Avisa que se va
  else 
    printf(BLANCO_T VERDE_F"\t\t\t\tPresione d para cerrar el local"RESET_COLOR"\n");
  
  sem_wait(delivery->memoria->semaforoDejarDinero);
  delivery->memoria->dato = pedidoCobrar;
  sem_post(delivery->memoria->semaforoCobrarDinero);
}

/*----------------------------------------------------------------------------*/
/*-----------------------FUNCIONES DE INICIALIZACION--------------------------*/
// Creacion de Telefono
Telefono * crearTelefono(int * puntuacion) {
  Telefono * telefono = (Telefono *)(calloc(1, sizeof(Telefono)));
  telefono->semaforoTelefono = (sem_t *)(calloc(1, sizeof(sem_t)));
  telefono->semaforoTelefono = sem_open("/semTelefono", O_CREAT, O_RDWR, 1);
  telefono->semaforoLlamadas = (sem_t *)(calloc(1, sizeof(sem_t)));
  telefono->semaforoLlamadas = sem_open("/semLlamadas", O_CREAT, O_RDWR, 0);
  telefono->puntuacion = puntuacion;
  return telefono;
}

// Creacion de Encargado
Encargado * crearEncargado(Telefono *telefono, struct Monitor_t * monitorComandas, int memoria){
  Encargado * encargado = (Encargado *)(calloc(1, sizeof(Encargado)));
  encargado->telefono = telefono;
  encargado->monitorComandas = monitorComandas;
  encargado->ubiMemoria = memoria;
  encargado->memoria = NULL;
  for(int i = 0; i < CARTA; i++)
    encargado->precios[i] = 100 * (i+1);
  for(int i = 0; i < PEDIDOSPORCARGAR; i++)
    encargado->pedidos[i] = -2;
  return encargado;
}

// Creacion de Cocinero
Cocinero * crearCocinero(struct Monitor_t * monitorComandas, struct Monitor_t * monitorPedidos) {
  Cocinero * cocinero = (Cocinero *)(calloc(1, sizeof(Cocinero)));
  cocinero->monitorComandas = monitorComandas;
  cocinero->monitorPedidos = monitorPedidos;
  cocinero->cantCocineros = COCINEROS;
  return cocinero;
}

// Creacion de Delivery
Delivery * crearDelivery(struct Monitor_t * monitorPedidos, int memoria) {
  Delivery * delivery = (Delivery *)(calloc(1, sizeof(Delivery)));
  delivery->monitorPedidos = monitorPedidos;
  delivery->cantDeliveries = DELIVERIES;
  delivery->ubiMemoria = memoria;
  delivery->memoria = NULL;
  return delivery;
}

// Creacion de Memoria
int crearMemoria() {
  int error = 0;

  // Creo la memoria
  int ubiMemoria = shm_open("/memCompartida", O_CREAT | O_RDWR, 0660);
  if (ubiMemoria < 0) {
    perror("shm_open()");
    error = -1;
  }
  if (!error) {
    error = ftruncate(ubiMemoria, sizeof(Memoria));
    if (error)
      perror("ftruncate()");
  }

  // Lleno la memoria
  llenarMemoria(ubiMemoria);

  // Devuelvo la ubicacion de la memoria.
  return ubiMemoria;
}

void llenarMemoria(int ubicacion) {
  // Mapeo una referencia a la memoria e inicializo la estructura que lleva adentro.
  Memoria * temp = mmap(NULL, sizeof(Memoria), PROT_READ | PROT_WRITE, MAP_SHARED, ubicacion, 0);

  temp->semaforoPedidosPorCobrar = (sem_t *)(calloc(1, sizeof(sem_t)));
  temp->semaforoPedidosPorCobrar = sem_open("/semPedidosPorCobrar", O_CREAT, O_RDWR, 0);

  temp->semaforoDejarDinero = (sem_t *)(calloc(1, sizeof(sem_t)));
  temp->semaforoDejarDinero = sem_open("/semDejarDinero", O_CREAT, O_RDWR, 0);

  temp->semaforoCobrarDinero = (sem_t *)(calloc(1, sizeof(sem_t)));
  temp->semaforoCobrarDinero = sem_open("/semCobrarDinero", O_CREAT, O_RDWR, 0);

  temp->dato = 0;

  // Desmapeo la referencia.
  if (temp != NULL) {
    int error = munmap((void*)(temp), 2 * sizeof(Memoria));
    if (error) {
      perror("creacion_memoria_munmap()");
    }
  }
}

/*----------------------------------------------------------------------------*/
/*--------------------FUNCIONES DE LIBERACION DE MEMORIA----------------------*/

// Borrado de semaforos y memoria
void borrarSemMem(Encargado * enc, int ubiMemoria) {
  Memoria * temp = mmap(NULL, sizeof(Memoria), PROT_READ | PROT_WRITE, MAP_SHARED, ubiMemoria, 0);
  int status=0;
  // Semaforo Telefono
  status = sem_close(enc->telefono->semaforoTelefono);
  if (!status) {
    status = sem_unlink("/semTelefono");
    if (status)
      perror("sem_unlink()");
  }
  else
    perror("sem_close()");

  // Semaforo Llamadas
  status = sem_close(enc->telefono->semaforoLlamadas);
  if (!status) {
    status = sem_unlink("/semLlamadas");
    if (status)
      perror("sem_unlink()");
  }
  else
    perror("sem_close()");

  // Semaforo PedidosPorCobrar
  status = sem_close(temp->semaforoPedidosPorCobrar);
  if (!status) {
    status = sem_unlink("/semPedidosPorCobrar");
    if (status)
      perror("sem_unlink()");
  }
  else
    perror("sem_close()");

  // Semaforo DejarDinero
  status = sem_close(temp->semaforoDejarDinero);
  if (!status) {
    status = sem_unlink("/semDejarDinero");
    if (status)
      perror("sem_unlink()");
  }
  else
    perror("sem_close()");

  // Semaforo CobrarDinero
  status = sem_close(temp->semaforoCobrarDinero);
  if (!status) {
    status = sem_unlink("/semCobrarDinero");
    if (status)
      perror("sem_unlink()");
  }
  else
    perror("sem_close()");

  // Desmapeo la memoria
  if (temp != NULL) {
    int error = munmap((void*)(temp), 2 * sizeof(Memoria));
    if (error) {
      perror("temp_munmap()");
    }
  }

  // Memoria Compartida
  if (ubiMemoria > 0) {
    status = shm_unlink("/memCompartida");
    if (status) {
      perror("unlink()");
    }
  }
}

/*----------------------------------------------------------------------------*/
/*-----------------------FUNCIONES DE LA PUNTUACION---------------------------*/

int  chequearPuntuacion(int score) {
  return 1;
}

void guardarPuntuacion(int score) {
  FILE * archivoPuntuacion;
  archivoPuntuacion = fopen("./puntuacion.txt", "a");
  if(archivoPuntuacion == NULL)
    perror("fopen()");
  
  printf("Escriba su nombre: ");
  char * nombre = (char*)(calloc(20,sizeof(char)));
  scanf("%s", nombre);
  __fpurge(stdin);

  fprintf(archivoPuntuacion,"%s: %d\n", nombre, score);
  fflush(archivoPuntuacion);

  free(nombre);
  int error = fclose(archivoPuntuacion);
  if(error)
    perror("fclose()");
}

void verPuntuacion(){
  FILE * archivoPuntuacion;
  archivoPuntuacion = fopen("./puntuacion.txt", "r");
  if(archivoPuntuacion == NULL)
    perror("fopen()");
  
  char * nombre = (char*)(calloc(20,sizeof(char)));
  int  * score  = (int *)(calloc(1,sizeof(int)));;

  int leido = fscanf(archivoPuntuacion, "%s %d", nombre, score);
  while(leido != EOF) {
    leido = fscanf(archivoPuntuacion, "%s %d", nombre, score);
    printf("%s %d\n", nombre, *score);
  }

  int temp = getchar();
  __fpurge(stdin);
  if(temp){}

  free(nombre);
  free(score);

  int error = fclose(archivoPuntuacion);
  if(error)
    perror("fclose()");
}

void salir(int * terminar){
  printf(MAGENTA_F BLANCO_T"\t\t\tGracias por jugar!!!"RESET_COLOR"\n");
  sleep(1);
  *terminar = 0;
  system("clear");
}