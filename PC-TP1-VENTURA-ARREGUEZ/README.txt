Como compilar y ejecutar el programa.

Paso 1) gcc -c -pthread -Wall -pedantic MonitoresBuffer.c -o MonitoresBuffer.o
Paso 2) gcc -pthread -Wall -pedantic TRABAJO-PC.c MonitoresBuffer.o -lrt
Paso 3) ./a.out

!! En caso de que nos tire el error: "Violación de segmento (`core' generado)"
deberemos utilizar el programa adjunto "BorrarSemaforoPosix.c" para borrar todos 
los semaforos del programa, listados aqui abajo:
- "/semTelefono"
- "/semLlamadas"
- "/semPedidosPorCobrar"
- "/semDejarDinero"
- "/semCobrarDinero"

Una vez borrados todos los semaforos, podremos ejecutar nuevamente el programa.