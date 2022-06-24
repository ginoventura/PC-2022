#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

int ProcesoPadre (void);
int ProcesoHijo  (void);

int EsPrimo (int nro);

int main (void) {
  int error=0, errorHijo=0, pid=0, pidHijo=0;

  /* Se intenta generar la primer FIFO */
  error=mkfifo("/tmp/primosP2C", 0777);
  if ((error) && (errno!=EEXIST)) {
    perror("mkfifo");
  }
  else {
    error=0;
  }

  if (!error) {
    /* Se intenta generar la segunda FIFO */
    error=mkfifo("/tmp/primosC2P", 0777);
    if ((error) && (errno!=EEXIST))  {
      perror("mkfifo");
    }
    else {
      error=0;
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
      error=ProcesoPadre();

      /* Esperamos que termine de ejecutarse el Hijo */
      pidHijo=wait(&errorHijo);
      if (errorHijo) {
	perror("Hijo");
	error=errorHijo;
      }
    }
    /* Si PID == 0 estamos en el proceso Hijo */
    else {
      error=ProcesoHijo();
    }
  }

  /* Se verifica si existe la FIFO para borrarla */
  if (!access("/tmp/primosP2C",F_OK)) {
    error=unlink("/tmp/primosP2C");
    if (error) {
      perror("unlink");
    }
  }

  /* Se verifica si existe la FIFO para borrarla */
  if (!access("/tmp/primosC2P",F_OK)) {
    error=unlink("/tmp/primosC2P");
    if (error) {
      perror("unlink");
    }
  }
    
  return error;
}


int ProcesoPadre (void) {
  int cant=1, nros=1, num=3, error=0, leidos=0;
  int fifoIn=0, fifoOut=0;
  char numero[10], ack[2];

  /* Se abre el canal para envio de datos */
  fifoOut=open("/tmp/primosP2C",O_WRONLY);
  if (fifoOut<0) {
    error=fifoOut;
    perror("fifo open");
  }

  if (!error) {
    /* Se abre el canal para recepcion de datos */
    fifoIn=open("/tmp/primosC2P",O_RDONLY);
    if (fifoIn<0) {
      error=fifoIn;
      perror("fifo open");
    }
  }

  if (!error) {
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
      leidos=write(fifoOut,numero,10);
      if (leidos<0) {
	perror("fifo write");
	error=leidos;
      }
      else {
	/* Si no hubo error, se toma el numero siguiente y se verifica */
	num+=2;
	if (EsPrimo(num)) {
	  printf("PADRE: %d es primo\n",num);
	  nros++;
	}
	/* Se espera la respuesta del proceso Hijo */
	leidos=read(fifoIn,ack,2);
	if (leidos<0) {
	  perror("fifo read");
	  error=leidos;
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
    leidos=write(fifoOut,numero,10);
    if (leidos<0) {
      perror("fifo write");
      error=leidos;
    }
  }

  /* Se cierran los canales de comunicacion */
  close(fifoIn);
  close(fifoOut);
  return error;
}


int ProcesoHijo  (void) {
  int num=1, error=0, leidos=0;
  int fifoIn=0, fifoOut=0;
  char numero[10], ack[2]="y\0";

  /* Se abre el canal de recepcion (al reves que el Padre para evitar bloqueo) */
  fifoIn=open("/tmp/primosP2C",O_RDONLY);
  if (fifoIn<0) {
    error=fifoIn;
    perror("fifo open");
  }

  if (!error) {
    /* Se abre el canal de envio */
    fifoOut=open("/tmp/primosC2P",O_WRONLY);
    if (fifoOut<0) {
      error=fifoOut;
      perror("fifo open");
    }
  }

  if (!error) {
    while ((num>0) && (!error)) {
      /* Se lee un numero de la FIFO y se verifica si hubo error */
      leidos=read(fifoIn,numero,10);
      if (leidos<0) {
	perror("fifo read");
	error=leidos;
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
	  leidos=write(fifoOut,ack,2);
	  if (leidos<0) {
	    perror("fifo write");
	    error=leidos;
	  }
	}
      }
    }
  }

  /* Se cierran los canales de comunicacion */
  close(fifoIn);
  close(fifoOut);
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

