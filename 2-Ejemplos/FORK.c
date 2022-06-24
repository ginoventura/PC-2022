#include<stdio.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<errno.h>

int ProcesoCocinero();
int ProcesoTelefono();
int ProcesoDelivery();
int ProcesoEncargado(int, int, int);

int main () {

  int PID1=0, PID2=0, PID3=0, error = 0;

    if (!error) {
        PID1 = fork();
            if (PID1 == 0) {
               ProcesoCocinero();
            }
        else if (PID1 > 0) {
        PID2 = fork();
            if (PID2 == 0) {
                ProcesoTelefono();  
            }
        }
        else if (PID2 > 0) {
        PID3 = fork();
            if (PID3 == 0) {
                ProcesoDelivery();
            }
            else if (PID3 > 0) {
                ProcesoEncargado(PID1, PID2, PID3);
            }
        }
    }
    else {
        printf("Hubo un error al crear los recursos.");
    }

    return error;    
}

int ProcesoCocinero() {
    printf("cocinero\n");
}

int ProcesoTelefono() {
    printf("telefono\n");
}

int ProcesoDelivery () {
    printf("delivery\n");
}

int ProcesoEncargado (int PID1, int PID2, int PID3){
    printf("encargado\n");
}