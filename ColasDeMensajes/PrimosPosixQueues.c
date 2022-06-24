#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <mqueue.h>

#define TAMMSG 8192

int ProcesoPadre (mqd_t enviar, mqd_t recibir);
int ProcesoHijo  (mqd_t enviar, mqd_t recibir);

int EsPrimo (int nro);

int main (void) {
  int error=0, errorHijo=0, pid=0, pidHijo=0;
  mqd_t padreHijo, hijoPadre;

  /* Se intenta generar la primer Cola de Mensajes  */
  padreHijo=mq_open("/padreHijo",O_RDWR | O_CREAT, 0777, NULL);
  if (padreHijo==-1) {
    perror("mq_open 1");
    error=padreHijo;
  }

  /* Se intenta generar la segunda Cola de Mensajes  */
  if (!error) {
    hijoPadre=mq_open("/hijoPadre",O_RDWR | O_CREAT, 0777, NULL);
    if (hijoPadre==-1) {
      perror("mqueue 2");
      error=hijoPadre;
    }
  }


  /* Se intenta generar el proceso Hijo */
  if (!error) {
    pid=fork();
    if (pid<0) {
      error=pid;
      perror("fork");
    }
  }

  if (!error) {
    /* Si no hubo error y PID mayor a cero estamos en el proceso Padre */
    if (pid>0) {
      error=ProcesoPadre(padreHijo,hijoPadre);

      /* Esperamos que termine de ejecutarse el Hijo */
      pidHijo=wait(&errorHijo);
      if (errorHijo) {
	perror("Hijo");
	error=errorHijo;
      }
    }
    /* Si PID == 0 estamos en el proceso Hijo */
    else {
      error=ProcesoHijo(hijoPadre,padreHijo);
    }
  }
    
  if (!access("./padreHijo",F_OK)) {
    padreHijo=mq_close(padreHijo);
    if (padreHijo) {
      perror("mq_close 1");
      error=padreHijo;
    }
    padreHijo=mq_unlink("./padreHijo");
    if (padreHijo) {
      perror("mq_close 1");
      error=padreHijo;
    }
  }
  if (!access("./hijoPadre",F_OK)) {
    hijoPadre=mq_close(hijoPadre);
    if (hijoPadre) {
      perror("mq_close 2");
      error=hijoPadre;
    }
    hijoPadre=mq_unlink("./hijoPadre");
    if (hijoPadre) {
      perror("mq_close 2");
      error=hijoPadre;
    }
  }

  return error;
}


int ProcesoPadre (mqd_t enviar, mqd_t recibir) {
  int cant=1, nros=1, num=3, error=0, enviado=0;
  char numero[10], ack[TAMMSG];

  printf("Ingrese cantidad de numeros primos: ");
  scanf("%d",&cant);

  if (cant>0) {
    printf("PADRE: 2 es primo\n");
  }
  else {
    printf("Valor incorrecto\n");
  }
    
  while (nros < cant) {
    /* Se envia un numero al proceso Hijo para que lo verifique */
    snprintf(numero,10,"%9d",num);
    enviado=mq_send(enviar,numero,10,0);
    if (enviado==-1) {
      perror("PADRE mq_send");
      error=enviado;
    }
    else {
      /* Si no hubo error, se toma el numero siguiente y se verifica */
      num+=2;
      if (EsPrimo(num)) {
	printf("PADRE: %d es primo\n",num);
	nros++;
      }
      /* Se espera la respuesta del proceso Hijo */
      enviado=mq_receive(recibir,ack,TAMMSG,NULL);
      if (enviado==-1) {
	perror("PADRE mq_receive");
	error=enviado;
      }
      else {
	/* Si el Hijo encontro un primo, se incrementa el contador */
	if (ack[0]=='y') {
	  nros++;
	}
	num+=2;
      }
    }
  }
  /* Cuando se encontraron los primos solicitados, se envia al Hijo */
  /* un numero negativo para que termine su ejecucion */
  snprintf(numero,10,"%9d",-1);
  enviado=mq_send(enviar,numero,10,0);
  if (enviado==-1) {
    perror("PADRE mq_send");
    error=enviado;
  }

  return error;
}


int ProcesoHijo  (mqd_t enviar, mqd_t recibir) {
  int num=1, error=0, enviado=0;
  char numero[TAMMSG], ack[2]="y\0";

  while ((num>0) && (!error)) {
    /* Se lee un numero del Pipe y se verifica si hubo error */
    enviado=mq_receive(recibir,numero,TAMMSG,NULL);
    if (enviado==-1) {
      perror("HIJO mq_receive");
      error=enviado;
    }
    else {
      /* Si no hubo error, se verifica si es mayor a cero. */
      /* Sino, se termina la ejecucion */
      num=atoi(numero);
      if (num>0) {
	if (EsPrimo(num)) {
	  printf("HIJO:  %d es primo\n",num);
	  ack[0]='y';
	}
	else {
	  ack[0]='n';
	}
	/* Se avisa al proceso Padre que si se encontro o no un numero primo */
	enviado=mq_send(enviar,ack,2,0);
	if (enviado==-1) {
	  perror("HIJO mq_send");
	  error=enviado;
	}
      }
    }
  }

  return error;
}


int EsPrimo (int nro) {
  int comp=3, Primo=1;

  if (nro%2==0) {
    Primo=0;
  }
  while ((Primo) && (comp<nro)) {
    if (nro%comp==0) {
      Primo=0;
    }
    else {
      comp+=2;
    }
  }

  return Primo;
}

