#include "DeleteCreate.h"

sem_t * crearSemaforo(char * nomSemaforo, char * identificador, int valorInicio) {
   char open [50] = "sem_open_";
   char openF [50];
   strcat(open, nomSemaforo);
   strcat(open, "_");
   strcat(open, identificador);
   strcpy(openF, open);
   strcat(open, "_ok()");
   strcat(openF, "_failed()");

   sem_t * semaforo = (sem_t *)(calloc(1, sizeof(sem_t)));
   semaforo = sem_open(nomSemaforo, O_CREAT, O_RDWR, valorInicio);
   if(semaforo == SEM_FAILED)
      perror(openF);
   else
      printf("%s\n", open);
   return semaforo;
}

mqd_t crearColaMensaje(char * nomCola, char * identificador, int apertura){
   mqd_t temp;

   struct mq_attr attr;
   attr.mq_flags = 0;
   attr.mq_maxmsg = 10;
   attr.mq_msgsize = 4;
   attr.mq_curmsgs = 0;

   char nomColaConBarra [50] = "/";
   strcat(nomColaConBarra, nomCola);
   char open [50] = "mq_open_";
   char openF [50];
   strcat(open, nomCola);
   strcat(open, "_");
   strcat(open, identificador);
   strcpy(openF, open);
   strcat(openF, identificador);
   strcat(open, "_ok()");
   strcat(openF, "_failed()");

   temp = mq_open(nomColaConBarra , apertura, 0777, &attr);
   if(temp == -1)
      perror(openF);
   else
      printf("%s\n",open);
      
   return temp;
}

int cerrarSemaforo(sem_t * semaforo, char * nomSemaforo, char * identificador) {
   char close [50] = "sem_close_";
   char closeF [50];
   strcat(close, nomSemaforo);
   strcat(close, "_");
   strcat(close, identificador);
   strcpy(closeF, close);
   strcat(close, "_ok()");
   strcat(closeF, "_failed()");

   int status = sem_close(semaforo);
   if(status) {
      perror(closeF);
      return -1;
   }
   else {
      printf("%s\n", close);
      return 0;
   }
}

void borrarSemaforo(sem_t * semaforo, char * nomSemaforo, char * identificador) {
   char nomSemaforoConBarra [50] = "/";
   strcat(nomSemaforoConBarra, nomSemaforo);
   char unlink [50] = "sem_unlink_";
   char unlinkF [50];
   strcat(unlink, nomSemaforo);
   strcat(unlink, "_");
   strcat(unlink  , identificador);
   strcpy(unlinkF, unlink);
   strcat(unlink, "_ok()");
   strcat(unlinkF, "_failed()");

   int error = cerrarSemaforo(semaforo, nomSemaforo, identificador);
   if(error)
      return;
   else {
      error = sem_unlink(nomSemaforoConBarra);
         if(error)
            perror(unlinkF);
         else
            printf("%s\n", unlink);
   }
}

int cerrarColaMensaje(mqd_t cola, char * nomCola, char * identificador) {
   char close [50] = "mq_close_";
   char closeF [50];
   strcat(close, nomCola);
   strcat(close, "_");
   strcat(close, identificador);
   strcpy(closeF, close);
   strcat(close, "_ok()");
   strcat(closeF, "_failed()");

   int status = mq_close(cola);
   if(status) {
      perror(closeF);
      return -1;
   }
   else {
      printf("%s\n", close);
      return 0;
   }
}

void borrarColaMensaje(mqd_t cola, char * nomCola, char * identificador){
   char nomColaConBarra [50] = "/";
   strcat(nomColaConBarra, nomCola);
   char unlink [50] = "mq_unlink_";
   char unlinkF [50];
   strcat(unlink, nomCola);
   strcat(unlink, "_");
   strcat(unlink, identificador);
   strcpy(unlinkF, unlink);
   strcat(unlink, "_ok()");
   strcat(unlinkF, "_failed()");

   int error = cerrarColaMensaje(cola, nomCola, identificador);
   if(error)
      return;
   else {
      error = mq_unlink(nomColaConBarra);
      if(error)
         perror(unlinkF);
      else
         printf("%s\n", unlink);
   }
}

int crearFIFO(char * nomFIFO){
   char nomFIFOBarra [50] = "/tmp/";
   strcat(nomFIFOBarra, nomFIFO);
   char open [50] = "FIFO_open_";
   char openF [50];
   strcat(open, nomFIFO);
   strcpy(openF, open);
   strcat(open, "_ok()");
   strcat(openF, "_failed()");

   int tempFIFO = mkfifo(nomFIFOBarra, 0777);
   if ((tempFIFO) && (errno!=EEXIST))
      perror(openF);
   else
      printf("%s\n", open);
   return tempFIFO;
}

void borrarFIFO(char * nomFIFO){
   char nomFIFOBarra [50] = "/tmp/";
   char unlinkS [50] = "FIFO_unlink_";
   char unlinkF [50];
   strcat(nomFIFOBarra, nomFIFO);
   strcat(unlinkS, nomFIFO);
   strcpy(unlinkF, unlinkS);
   strcat(unlinkS, "_ok()");
   strcat(unlinkF, "_failed()");
   int status = status = unlink(nomFIFOBarra);
   if(status)
      perror(unlinkF);
   else
      printf("%s\n", unlinkS);
}