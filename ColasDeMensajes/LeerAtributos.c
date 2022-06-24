#include <stdio.h>
#include <mqueue.h>

int main (void) {
  int error=0;
  mqd_t cola, dummy;
  struct mq_attr parametros;

  cola=mq_open("/LecturaAtributos",O_RDONLY|O_CREAT,0660,NULL);
  if (cola==-1) {
    error=cola;
    perror("mq_open");
  }

  if (!error) {
    dummy=mq_getattr(cola,&parametros);
    if (dummy==-1) {
      error=dummy;
      perror("mq_getattr");
    }
  }

  if (!error) {
    printf("Flags: %d\n",parametros.mq_flags);
    printf("Cant. Max. Msg.: %d\n",parametros.mq_maxmsg);
    printf("Tam. Max.: %d\n",parametros.mq_msgsize);
    printf("Msg. en cola: %d\n",parametros.mq_curmsgs);

    dummy=mq_close(cola);
    if (dummy==-1) {
      error=dummy;
      perror("mq_close");
    }
  }

  if (!error) {
    dummy=mq_unlink("/LecturaAtributos");
    if (dummy==-1) {
      error=dummy;
      perror("mq_unlink");
    }
  }

  return error;
}


