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

#include "MonitoresBuffer.h"

//Datos del juego
#define ENCARGADOS 1
#define COCINEROS 3
#define DELIVERIES 2
#define BUFFERPEDIDOS 4
#define BUFFERTICKET 4
#define CARTA 5
#define ALARMA 10
#define TIEMPOLLAMADA 1
#define ULTIMOPEDIDO -1
#define PEDIDOSPORCARGAR 10

//Variable para indicar cuando termina el juego
int timeout = 1;

// ESTRUCTURAS
// Estructura de la memoria 
typedef struct {
    sem_t * semaforoPedidosPorCobrar;
    sem_t * semaforoDejarDinero;
    sem_t * semaforoCobrarDinero;
    int dato;
} Memoria;

// Estructura del telefono
typedef struct {
    sem_t * semaforoTelefono;
    sem_t * semaforoLlamadas;
    int pedido;
} Telefono;

// Estructura del encargado
typedef struct {
    Telefono * telefono;
    Memoria * memoria;
    struct Monitor_t *monitorTicket;
    int ubiMemoria;
    int pedidos[PEDIDOSPORCARGAR]; 
    float precios[CARTA];
} Encargado;

// Estructura del cocinero
typedef struct {
    struct Monitor_t *monitorTicket;
    struct Monitor_t *monitorPedidos;
    int cantCocineros;
} Cocinero;

// Estructura del delivery
typedef struct {
    struct Monitor_t *monitorPedidos;
    int cantDeliveries;
    int ubiMemoria;
    Memoria * memoria;
} Delivery;

// -------------FUNCIONES DE INICIALIZACION-------------
// Actores 
Telefono * crearTelefono();
Encargado * crearEncargado(Telefono *, struct Monitor_t *, int);
Cocinero * crearCocinero(struct Monitor_t *, struct Monitor_t *);
Delivery * crearDelivery(struct Monitor_t *, int);

// Memoria 
int crearMemoria();
void llenarMemoria(int);

// -------------FUNCIONES DEL JUEGO-------------
void mostrarMenu();
int comenzarJuego();
void jugar();
void salir(int *);

// -------------FUNCIONES DE LOS ACTORES-------------
// Funciones del telefono
void * gestionTelefono(void *);
void recibirLlamada(Telefono *telefono);
void TimeOut();

// Funciones del encargado
void atenderPedido(Encargado *);
void cargarPedido(Encargado *);
void cobrarPedido(Encargado *, int *);

// Funciones del cocinero
void * gestionCocinero(void *);
void cocinarPedido(Cocinero *, int *);
void pedidoCocinado(Cocinero *);
void pedidoListo(Cocinero *, int);

// Funciones del delivery 
void * gestionDelivery(void *);
void repartirPedido(Delivery *, int *);
void avisarCobro(Delivery *, int);

// Borrar semaforos y memoria
void borrarSemMem(Encargado *, int);

//--------------MAIN--------------
int main (){
    srand(time(NULL));

    int terminar = 1;
    char eleccion;

    do {
        mostrarMenu();

        do {
            eleccion = getchar();
            __fpurge(stdin);
            if(eleccion != '1' && eleccion != '2') {
                printf("\nOpcion invalida, por favor, seleccione una opcion correcta.\nIngrese una opcion: ");
            }
        } while (eleccion != '1' && eleccion != '2');

        switch (eleccion)
        {
        case '1':
            system("clear");
            comenzarJuego();
            break;
        case '2':
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
    printf("\t\t\t _____________________________________________________________________\n");
    printf("\t\t\t|---------------------------------------------------------------------|\n");
    printf("\t\t\t|-------------------- RESTAURANTE EL CONCURRENTE ---------------------|\n");
    printf("\t\t\t|---------------------------------------------------------------------|\n");
    printf("\t\t\t|------------------------- 1. Comenzar juego -------------------------|\n");
    printf("\t\t\t|----------------------------- 2. Salir ------------------------------|\n");
    printf("\t\t\t|---------------------------------------------------------------------|\n");
    printf("\t\t\t|---------------------------------------------------------------------|\n");
    printf("\t\t\t|--------------------- Instrucciones del juego: ----------------------|\n");
    printf("\t\t\t|Tecla 'a': Para atender el telefono cuando este sonando.             |\n");
    printf("\t\t\t|Tecla 'b': Para cargar el pedido una vez que se lo tomó al cliente.  |\n");
    printf("\t\t\t|Tecla 'c': Para cobrar pedido una vez que el delivery entregó        |\n"); 
    printf("\t\t\t|el pedido y regresó al restaurante.                                  |\n");
    printf("\t\t\t|---------------------------------------------------------------------|\n");
    printf("\t\t\t|_____________________________________________________________________|\n");
    printf("\t\t\t Ingrese una opcion:");
}

int comenzarJuego(){
    timeout = 1;
    
    //Crear monitores
    struct Monitor_t * monitorTicket = CrearMonitor(BUFFERTICKET);
    struct Monitor_t * monitorPedidos = CrearMonitor(BUFFERPEDIDOS);

    //Crear memoria
    int memoria = crearMemoria();

    //Crear actores del juego
    Telefono * telefono = crearTelefono();
    Encargado * encargado = crearEncargado(telefono, monitorTicket, memoria);
    Cocinero * cocinero = crearCocinero(monitorTicket, monitorPedidos);
    Delivery * delivery = crearDelivery(monitorPedidos, memoria);

    //Instanciar las variables hilo de cada objeto
    pthread_t hiloTelefono;
    pthread_t hilosCocineros[COCINEROS];
    pthread_t hilosDeliveries[DELIVERIES];

    //Crear hilos de cada objeto
    pthread_create(&hiloTelefono, NULL, gestionTelefono, (void *)(telefono));
    for(int i=0; i<COCINEROS; i++) {
        pthread_create(&hilosCocineros[i], NULL, gestionCocinero, (void *)(cocinero));
    }
    for(int i=0; i<DELIVERIES; i++) {
        pthread_create(&hilosDeliveries[i], NULL, gestionDelivery, (void *)(delivery));
    }

    //Mapeo de memoria del encargado
    encargado->memoria = mmap(NULL, sizeof(Memoria), PROT_READ | PROT_WRITE, MAP_SHARED, encargado->ubiMemoria, 0);

    //Arranca el ciclo de juego
    jugar(encargado);

    //Desmapeo de la memoria del encargado
    if (encargado->memoria != NULL) {
        int error = munmap((void*)(encargado->memoria), 2 * sizeof(Memoria));
        if (error) 
            perror("encargado_munmap()");
    }

    //Esperar que terminen todos los hilos 
    pthread_join(hiloTelefono, NULL);
    for(int i = 0; i < COCINEROS; i++)
        pthread_join(hilosCocineros[i], NULL);
    for(int i = 0; i < DELIVERIES; i++)
        pthread_join(hilosDeliveries[i], NULL);

    //Liberar semaforos
    borrarSemMem(encargado, memoria);

    //Borrar monitores
    BorrarMonitor(monitorTicket);
    BorrarMonitor(monitorPedidos);

    //Liberar memoria de objetos creados
    free(telefono);
    free(encargado);
    free(cocinero);
    free(delivery);

    return 0;
}

void jugar(Encargado * encargado) {
    int * terminado = (int *)(calloc(1, sizeof(int)));
    char eleccion;

    while(* terminado != -1) {
        do {
            eleccion = getchar();
        } while (eleccion != 'a' && eleccion != 'b' && eleccion != 'c');
        switch (eleccion) {
        case 'a':
            atenderPedido(encargado);
            break;
        case 'b':
            cargarPedido(encargado);
            break;
        case 'c':
            cobrarPedido(encargado, terminado);
            break;
        default:
            break;
        }
    }
    free(terminado);
}

// Hilo encargado
void atenderPedido(Encargado * encargado) {

    //Verificar si hay llamada entrante
    int error = sem_trywait(encargado->telefono->semaforoLlamadas);
    if(!error) {
        printf("\tTelefono atendido...\n");
        usleep(rand()% 100001 + 250000);
        int codigoPedido = encargado->telefono->pedido;

        for(int i = 0; i < PEDIDOSPORCARGAR; i++){
            if(encargado->pedidos[i] == -2){
                encargado->pedidos[i] = codigoPedido;
                printf("\t\tEncargo %d listo para cargar\n", codigoPedido);
                break;
            }
        }
    // Cuelga el telefono
    sem_post(encargado->telefono->semaforoTelefono);
    }
}

void cargarPedido (Encargado * encargado) {

    for(int  i = 0; i < PEDIDOSPORCARGAR; i++) {
            int codigoPedido = encargado->pedidos[i];
        if (codigoPedido != -2 && codigoPedido != ULTIMOPEDIDO) {
            encargado->pedidos[i] = -2;
            //Le pasa el pedido a los cocineros
            int error = GuardarDato(encargado->monitorTicket, codigoPedido);
            if(error)
            perror("GuardarDato()");
        } else if (codigoPedido != -2 && codigoPedido == ULTIMOPEDIDO) {
            encargado->pedidos[i] = -2;
            for (int i=0; i<COCINEROS; i++) {
                int error = GuardarDato(encargado->monitorTicket, codigoPedido);
                if (error) 
                    perror("GuardarDato()");
            }
        }
    }
}

void cobrarPedido (Encargado * encargado, int * terminado) {
    int cobrosPendientes = 0;
    int error = sem_getvalue(encargado->memoria->semaforoPedidosPorCobrar, &cobrosPendientes);

    if(!error) {
        if (cobrosPendientes > 0) {
            sem_post(encargado->memoria->semaforoDejarDinero);
            sem_wait(encargado->memoria->semaforoCobrarDinero);
            int pedido = encargado->memoria->dato;

            //Si el delivery avisa que ya termino, cierro el restaurante
            if (pedido != ULTIMOPEDIDO) {
                printf("\t$%.0f guardados del pedido Nº %d\n", encargado->precios[pedido], pedido);
            }
            else {
                printf("\t\tCerrando el restaurante...\n");
                * terminado = -1;
            }
            sem_trywait(encargado->memoria->semaforoPedidosPorCobrar);
        }
    }
    else {
        perror("encargado_sem_getvalue()");
    }
}

// Hilo telefono
void * gestionTelefono(void * tmp) {

    Telefono * telefono = (Telefono *) tmp;

    //Iniciamos la alarma y el contador
    signal(SIGALRM, TimeOut);
    alarm(ALARMA);

    //While de recibir llamadas, finaliza cuando suena la alarma
    while (timeout) {
        recibirLlamada(telefono);
    }

    //Se envia el ultimo pedido
    sem_wait(telefono->semaforoTelefono);
    telefono->pedido = ULTIMOPEDIDO;
    printf("\tEncargado llamado para cerrar el restaurante\n");
    sem_post(telefono->semaforoLlamadas);

    //Termina el hilo
    pthread_exit(NULL);
}

void recibirLlamada(Telefono * telefono) {
    
    //El telefono empieza a recibir llamadas con pedidos aleatorios
    sem_wait(telefono->semaforoTelefono);
    usleep(rand()% 750001 + 250000);
    telefono->pedido = rand() % CARTA;
    printf("\tTelefono sonando...\n");

    //El telefono empieza a sonar
    sem_post(telefono->semaforoLlamadas);
    sleep(TIEMPOLLAMADA);   //Tiempo que el cliente hace sonar el telefono
    int telefonoSonando = 0;

    //Si el encargado no atiende el telefono, se pierde la llamada
    sem_getvalue(telefono->semaforoLlamadas, &telefonoSonando);
    if (telefonoSonando > 0) {
        int error = sem_trywait(telefono->semaforoLlamadas);
        if(!error) {
            printf("\t\tLlamada perdida\n");
            sem_post(telefono->semaforoTelefono);
        }
    }
}

void TimeOut() {
    timeout = 0;
}

// Hilo Cocinero 
void * gestionCocinero(void * tmp) {

    Cocinero *cocinero = (Cocinero *) tmp;
    int * terminado = (int *)(calloc(1, sizeof(int)));

    //Ciclo de cocinar los pedidos, finaliza cuando el encargado avisa que no hay mas pedidos
    while (*terminado != -1) {
        cocinarPedido(cocinero, terminado);
    }

    free(terminado);

    //Termina el hilo del cocinero
    pthread_exit(NULL);
}

void cocinarPedido(Cocinero * cocinero, int * terminado) {

    //El cocinero toma un ticket para empezar a cocinar
    int pedidoActual = 0;
    int error = LeerDato(cocinero->monitorTicket, &pedidoActual);
    if (error) {
        perror("LeerDato()");
    }
    else {
        //El cocinero empieza a cocinar
        if (pedidoActual != ULTIMOPEDIDO) {
            usleep(rand()% 500001 + 1000000); //Tiempo que se demora en cocinar el pedido
            pedidoListo(cocinero, pedidoActual);
        }
        //Recibe el aviso para terminar de cocinar
        else {
            cocinero->cantCocineros--;
            * terminado = -1;
            //El ultimo cocinero se encarga de avisar a los deliveries que la cocina cerró
            if(cocinero->cantCocineros == 0) {
                for(int i=0; i<DELIVERIES; i++){
                    pedidoListo(cocinero, ULTIMOPEDIDO);
                }
            }
        }
    }
}

void pedidoListo(Cocinero * cocinero, int pedidoListo) {
    
    //El cocinero avisa que el pedido esta listo
    int error = GuardarDato(cocinero->monitorPedidos, pedidoListo);
    if(error) {
        perror("GuardarDato()");
    }
}

// Hilo Delivery
void * gestionDelivery(void * tmp){
    Delivery * delivery = (Delivery *)(tmp);
    int * terminado = (int *)(calloc(1, sizeof(int)));

    //Se mapea la memoria compartida una sola vez, para indicar como esta distribuida la memoria
    if(delivery->memoria == NULL) {
        delivery->memoria = mmap(NULL, sizeof(Memoria), PROT_READ | PROT_WRITE, MAP_SHARED, delivery->ubiMemoria, 0);  
    }

    //Ciclo de repartir pedidos
    while(*terminado != -1) {
        repartirPedido(delivery, terminado);
    }

    //Si es el ultimo delivery, desmapea la memoria
    if(delivery->cantDeliveries == 0) {
        if(delivery->memoria != NULL) {
            int error = munmap((void *)(delivery->memoria), 2 * sizeof(Memoria));
            if(error) {
                perror("delivery_munmap()");
            }
        }
    }

    free(terminado);

    pthread_exit(NULL);
}

void repartirPedido(Delivery * delivery, int * terminado) {
    int error = 0;
    int pedidoRepartir = 0;
    error = LeerDato(delivery->monitorPedidos, &pedidoRepartir);

    if(error) {
        perror("LeerDato()");
    }
    else {
        //El delivery sale a repartir el pedido
        if(pedidoRepartir != ULTIMOPEDIDO) {
            usleep(rand()% 500001 + 500000);
            avisarCobro(delivery, pedidoRepartir);
        }
        //Recibe el mensaje de un cocinero para que se retire
        else {
            delivery->cantDeliveries--;
            * terminado = -1;
            //Si es el ultimo delivery que termina, le avisa al encargado
            if(delivery->cantDeliveries == 0) {
                avisarCobro(delivery, ULTIMOPEDIDO);
            }
        }
    }
}

void avisarCobro(Delivery * delivery, int pedidoCobrar) {

    //El delivery avisa al encargado que esta listo para dejar el dinero del pedido
    sem_post(delivery->memoria->semaforoPedidosPorCobrar);

    //Deja el dinero
    if(pedidoCobrar != ULTIMOPEDIDO) {
        printf("\tPedido Nº. %d listo para cobrar\n", pedidoCobrar);
    } 
    //Avisa que cierra el local
    else {
        printf("\t\tPresione la letra d para cerrar el local\n");
    }
    sem_wait(delivery->memoria->semaforoDejarDinero);
    delivery->memoria->dato = pedidoCobrar;
    sem_post(delivery->memoria->semaforoCobrarDinero);
}

// Funciones de inicializacion
// Creacion de telefono
Telefono * crearTelefono() {
  Telefono * telefono = (Telefono *)(calloc(1, sizeof(Telefono)));
  telefono->semaforoTelefono = (sem_t *)(calloc(1, sizeof(sem_t)));
  telefono->semaforoTelefono = sem_open("/semTelefono", O_CREAT, O_RDWR, 1);
  telefono->semaforoLlamadas = (sem_t *)(calloc(1, sizeof(sem_t)));
  telefono->semaforoLlamadas = sem_open("/semLlamadas", O_CREAT, O_RDWR, 0);
    return telefono;
}

//Creacion de encargado
Encargado * crearEncargado(Telefono *telefono, struct Monitor_t * monitorTicket, int memoria) {
  Encargado * encargado = (Encargado *)(calloc(1, sizeof(Encargado)));
  encargado->telefono = telefono;
  encargado->monitorTicket = monitorTicket;
  encargado->ubiMemoria = memoria;
  encargado->memoria = NULL;
  for(int i = 0; i < CARTA; i++)
    encargado->precios[i] = 100 * (i+1);
  for(int i = 0; i < PEDIDOSPORCARGAR; i++)
    encargado->pedidos[i] = -2;
  return encargado;
}

//Creacion de cocinero 
Cocinero * crearCocinero(struct Monitor_t * monitorTicket, struct Monitor_t * monitorPedidos) {
    Cocinero * cocinero = (Cocinero *)(calloc(1, sizeof(Cocinero)));
    cocinero->monitorTicket = monitorTicket;
    cocinero->monitorPedidos = monitorPedidos;
    cocinero->cantCocineros = COCINEROS;

    return cocinero;
}

Delivery * crearDelivery(struct Monitor_t * monitorPedidos, int memoria) {
    Delivery * delivery = (Delivery *)(calloc(1, sizeof(Delivery)));
    delivery->monitorPedidos = monitorPedidos;
    delivery->cantDeliveries = DELIVERIES;
    delivery->ubiMemoria = memoria; 
    delivery->memoria = NULL;
    return delivery;
}

//Creacion de memoria
int crearMemoria() {
    int error = 0;

    //Se crea la memoria
    int ubiMemoria = shm_open("/memCompartida", O_CREAT | O_RDWR, 0660);
    if(ubiMemoria < 0) {
        perror("shm_open()");
        error = -1;
    }
    if (!error) {
        error = ftruncate(ubiMemoria, sizeof(Memoria));
        if (error)
            perror("ftruncate()");
    }

    //Se llena la memoria
    llenarMemoria(ubiMemoria);

    //Devuelve la ubicacion de la memoria
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

// Funciones para liberar memoria

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

void salir(int *terminar) {
    printf("\n\n");
    printf("\t\t\tGRACIAS POR JUGAR! HASTA PRONTO! (ツ)_/\n");
    sleep(5);
    *terminar = 0;
    system("clear");
}
