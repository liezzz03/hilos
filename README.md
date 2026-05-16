**Pasamos de procesos (donde cada uno tiene su memoria) a hilos (threads), donde todos comparten la misma memoria 
(el array de asientos).**

[Ficha_practica-hilos_2025_2026.pdf](https://github.com/user-attachments/files/27273768/Ficha_practica-hilos_2025_2026.pdf)

Para ejecutar por hitos se puede ejecutar las órdenes:

> $ gcc -o hito1/multihilos hito1/multihilos.c hito1/sala.c retardo.c -lpthread

> $ ./hito1/multihilos 20

*Para el hito1*

Similar para los otros dos hitos, mientras que para el reto con hacer los siguientes comandos es suficiente:

> gcc -o reto reto.c hito3/sala.c retardo.c -lpthread

> ./reto 