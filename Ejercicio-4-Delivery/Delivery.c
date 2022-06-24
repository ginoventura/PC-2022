/*Se debe realizar el motor de un juego que denominaremos "Delivery". Este juego simulará la actividad de un local de comidas en el cual ingresan los pedidos por teléfono y los empleados tienen que preparar y entregar lo indicado.

En esta primer etapa se deberá trabajar sobre cuatro tipos de hilos:

    encargado que da curso a los pedidos de los clientes y, luego de entregado el pedido, guarda el cobro en la caja,
    telefono que genera los pedidos de los clientes
    cocinero que prepara la comida,
    delivery que lleva los pedidos a los clientes, 

El encargado atiende los pedidos que llegan por telefono separados un tiempo aleatorio. El cocinero prepara el pedido, demorando un tiempo diferente según la comida solicitada; solo puede preparar un pedido por vez. El delivery entrega los pedidos a los clientes (de a uno por vez), demorando un tiempo diferente (aleatorio) según la distancia, y al regresar entrega el monto del pedido al encargado.*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

//Estructura para sincronizar las funciones de telefono, cocinero y delivery
typedef struct {
    pthread_mutex_t * mutex1;
    pthread_mutex_t * mutex2;
    int * llamadas;
}Sincronizacion;

//Estructura para sincronizar las funciones del encargado
typedef struct {
    Sincronizacion * sincronizacion;
    pthread_mutex_t * mutex3;
}Encargado;

//Funciones para inicializar los mutex de sincronizacion
Sincronizacion * inicializarSincronizacion(pthread_mutex_t *, pthread_mutex_t *, int *);
Encargado * inicializarEncargado(Sincronizacion *, pthread_mutex_t *);

//Funciones del telefono:
void *hiloTelefono(void *);
void sonando(Sincronizacion *);        //Suena el telefono
void atendido(Sincronizacion *);       //Se atiende el telefono

//Funciones del encargado:
void *hiloEncargado(void *);
void tomandoPedido(Encargado *);       //Toma el pedido del cliente
void pedidoCargado(Encargado *);       //Carga el pedido del cliente
void cobrarPedido(Encargado *);        //Registra el cobro en la caja

//Funciones del cocinero:
void *hiloCocinero(void *);
void nuevoPedido(Sincronizacion *);         //Toma el pedido al encargado y cocina
void pedidoCocinado(Sincronizacion *);      //El pedido esta listo

//Funciones del delivery:
void *hiloDelivery(void *);
void entregarPedido(Sincronizacion *);      //Pedido en viaje
void pedidoEntregado(Sincronizacion *);     //Entrega el pedido

int main () {

    //Creacion de los mutex, en forma dinamica si no nos saldria violacion del core
    pthread_mutex_t * m1 = (pthread_mutex_t *)(calloc(1, sizeof(pthread_mutex_t)));
    pthread_mutex_t * m2 = (pthread_mutex_t *)(calloc(1, sizeof(pthread_mutex_t)));
    pthread_mutex_t * m3 = (pthread_mutex_t *)(calloc(1, sizeof(pthread_mutex_t)));
    pthread_mutex_t * m4 = (pthread_mutex_t *)(calloc(1, sizeof(pthread_mutex_t)));

    //Creacion de la variable para no recibir mas llamadas.
    int * flag;
    *flag = 1;

    //Creacion de los empleados de la pizzeria
    Sincronizacion * telefono = inicializarSincronizacion(m1, m2, flag);    //El telefono se comunica con el encargado
    Encargado * encargado = inicializarEncargado(telefono, m3);             //El encargado se comunica con el telefono y el cocinero
    Sincronizacion * cocinero = inicializarSincronizacion(m3, m4, flag);    //El cocinero se comunica con el encargado y el delivery
    Sincronizacion * delivery = inicializarSincronizacion(m4, m2, flag);    //El delivery se comunica con el cocinero y el encargado(entregar el dinero)

    //Inicio de los mutex
    pthread_mutex_init(m1, NULL);       //Mutex del telefono
    pthread_mutex_init(m2, NULL);       //Mutex del encargado
    pthread_mutex_init(m3, NULL);       //Mutex del cocinero
    pthread_mutex_init(m4, NULL);       //Mutex del delivery

    //Bloqueo de los mutex
    pthread_mutex_lock(m2);     //El encargado inicia bloqueado, esperando llamada
    pthread_mutex_lock(m3);     //El cocinero inicia bloqueado, esperando pedido
    pthread_mutex_lock(m4);     //El delivery inicia bloqueado, esperando pedido listo

    //Instancia de los hilos de cada empleado
    pthread_t Telefono;         //Hilo del telefono
    pthread_t Encargado;        //Hilo del encargado
    pthread_t Cocinero;         //Hilo del cocinero
    pthread_t Delivery;         //Hilo del delivery

    //Creacion de los hilos de cada empleado
    pthread_create(&Telefono, NULL, hiloTelefono, (void *)(telefono));
    pthread_create(&Encargado, NULL, hiloEncargado, (void *)(encargado));
    pthread_create(&Cocinero, NULL, hiloCocinero, (void *)(cocinero));
    pthread_create(&Delivery, NULL, hiloDelivery, (void *)(delivery));

    //Se espera que terminen todos los hilos
    pthread_join(Telefono, NULL);
    pthread_join(Encargado, NULL);
    pthread_join(Cocinero, NULL);
    pthread_join(Delivery, NULL);
    
    return 0;
}

//Inicializacion de un objeto de tipo Sincronizacion
Sincronizacion * inicializarSincronizacion(pthread_mutex_t *m1, pthread_mutex_t *m2, int *flag) {
    Sincronizacion * tmp = (Sincronizacion *)(calloc(1, sizeof(Sincronizacion)));
    tmp->mutex1 = m1;
    tmp->mutex2 = m2;
    tmp->llamadas = flag;
    return tmp;
}

//Inicializacion de un objeto de tipo Encargado
Encargado * inicializarEncargado(Sincronizacion * s1, pthread_mutex_t *m1){
    Encargado * tmp = (Encargado *)(calloc(1, sizeof(Encargado)));
    tmp->sincronizacion = s1;
    tmp->mutex3 = m1;
    return tmp;
}

//--------------HILO DEL TELEFONO--------------
void * hiloTelefono(void *tmp) {
    Sincronizacion *telefono = (Sincronizacion *) tmp;

    srand(time(NULL)); 
    int pedidos = rand () % 10 + 5;           //cantidad de pedidos en el dia, entre 5 y 15

    for (int i = 0; i < pedidos; i++) {               //for para establecer la cantidad de pedidos que se va a aceptar
        usleep(rand()% 500001 + 50000);
            sonando(telefono);
            atendido(telefono);
    }
    *telefono->llamadas = 0;
    
    pthread_exit(NULL);
}

void sonando(Sincronizacion * telefono) {
    pthread_mutex_lock(telefono->mutex1);
    printf("\tTeléfono sonando...\n");
}

void atendido(Sincronizacion * telefono) {
    pthread_mutex_unlock(telefono->mutex2);     //Se atiende el telefono y procede el encargado
}

//--------------HILO DEL ENCARGADO--------------
void * hiloEncargado(void *tmp) {
    Encargado *encargado = (Encargado *) tmp;
    srand(time(NULL));
    
    while(*encargado->sincronizacion->llamadas != 0){
        tomandoPedido(encargado);
        pedidoCargado(encargado);
        cobrarPedido(encargado);
    }
    pthread_exit(NULL);
}

void tomandoPedido(Encargado * encargado) {

    srand(time(NULL));
    pthread_mutex_lock(encargado->sincronizacion->mutex2);      //Se atiende el telefono
    printf("\t\tTeléfono atendido.\n");
    usleep(rand()% 250001 + 500000);
}

void pedidoCargado(Encargado * encargado) {
    printf("\t\t\tPedido tomado.\n");
    pthread_mutex_unlock(encargado->mutex3);        //El encargado le pasa el pedido al cocinero
}

void cobrarPedido(Encargado * encargado) {
    srand(time(NULL));
    pthread_mutex_lock(encargado->sincronizacion->mutex2);
    printf("\t\t\t\tDinero ingresado en la caja.\n\n");
    usleep(rand()% 150001 + 100000);
    pthread_mutex_unlock(encargado->sincronizacion->mutex1);
}

//--------------FUNCIONES DEL COCINERO--------------
void * hiloCocinero(void *tmp) {
    Sincronizacion *cocinero = (Sincronizacion *) tmp;
    srand(time(NULL));
    
    while(*cocinero->llamadas != 0){
        nuevoPedido(cocinero);
        pedidoCocinado(cocinero);
    }
    pthread_exit(NULL);
}

void nuevoPedido(Sincronizacion * cocinero){
    srand(time(NULL));
    pthread_mutex_lock(cocinero->mutex1);       //Se bloquea el telefono mientras el cocinero prepara el pedido
    printf("\tCocinando pedido...\n");
    usleep(rand()% 1000001 + 1000000);
}

void pedidoCocinado(Sincronizacion * cocinero){
    printf("\t\tPedido cocinado.\n");
    pthread_mutex_unlock(cocinero->mutex2);     //El cocinero pasa el pedido al encargado
}

//--------------FUNCIONES DEL DELIVERY--------------
void * hiloDelivery(void *tmp) {
    Sincronizacion *delivery = (Sincronizacion *) tmp;
    srand(time(NULL));
    while(*delivery->llamadas != 0){
        entregarPedido(delivery);
        pedidoEntregado(delivery);
    }
    pthread_exit(NULL);
}

void entregarPedido(Sincronizacion * delivery) {
    srand(time(NULL));
    pthread_mutex_lock(delivery->mutex1);
    printf("\tEntregando pedido...\n");
    usleep(rand()% 1000001 + 500000);
}

void pedidoEntregado(Sincronizacion * delivery) {
    printf("\t\tPedido entregado.\n");
    srand(time(NULL));
    usleep(rand()% 1000001 + 500000);
    printf("\t\t\tLlevando el dinero a la pizzeria.\n");
    pthread_mutex_unlock(delivery->mutex2);
}

//--------------FUNCIONES DE LIBERACION DE MEMORIA--------------
// Liberacion de memoria de la sincronizacion
void liberarSincronizacion(Sincronizacion* tmp){
    free(tmp->llamadas);
    pthread_mutex_destroy(tmp->mutex1);
    pthread_mutex_destroy(tmp->mutex2);
}

// Liberacion de memoria del encargado, que usa la liberacion de memoria de la sincronizacion
void liberarEncargado(Encargado * tmp){
    liberarSincronizacion(tmp->sincronizacion);
    pthread_mutex_destroy(tmp->mutex3);
}