**Pasamos de procesos (donde cada uno tiene su memoria) a hilos (threads), donde todos comparten la misma memoria 
(el array de asientos).**

[Ficha_practica-hilos_2025_2026.pdf](https://github.com/user-attachments/files/27273768/Ficha_practica-hilos_2025_2026.pdf)

Para ejecutar por hitos se puede ejecutar las órdenes:

> $ gcc -o hito1/multihilos hito1/multihilos.c hito1/sala.c retardo.c -lpthread

> $ ./hito1/multihilos 20

*Para el hito1*

> $ gcc -o hito2/multihilos hito2/multihilos.c hito2/sala.c retardo.c -lpthread

> $ ./hito2/multihilos 100

*Para el hito2*

> $ gcc -o hito3/multihilos hito3/multihilos.c hito3/sala.c retardo.c -lpthread

> $ ./hito3/multihilos 6 6

*Para el hito3*


> $ gcc -o reto1/reto reto1/sala.c reto1/reto.c retardo.c -lpthread

> $ ./reto1/reto 12 2

*Para el reto1* 