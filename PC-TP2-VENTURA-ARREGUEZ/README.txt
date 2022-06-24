Para poder compilar y ejecutar el programa son necesarios los siguientes pasos:
1. gcc -c DeleteCreate.c -o DeleteCreate.o
2. gcc -pthread -Wall -pedantic TP2-PC-VENTURA-ARREGUEZ.c DeleteCreate.o -lrt
3. ./a.out