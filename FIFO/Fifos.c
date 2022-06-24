#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

int main (void) {
  int mififo=0, error=0, cant=0;

  error=mkfifo("/tmp/prueba",0777);
  if ((error<0) && (errno!=EEXIST)) {
      perror("mkfifo");
  }
  else {
    error=0;
    printf("Fifo creada!\n");
  }

  if (!error) {
    mififo=open("/tmp/prueba",O_RDONLY);
    if (mififo<0) {
      perror("open");
      error=mififo;
    }
    else {
      printf("FIFO abierta!\n");
/*       cant=write(mififo,"probando\0",9); */
      if (cant!=9) {
	perror("write");
	error=-1;
      }
      close(mififo);
    }
  }


  if (!error) {
    error=unlink("/tmp/prueba");
    printf("FIFO borrada!\n");
    if(error) {
      perror("unlink");
    }
  }

  return error;
}


