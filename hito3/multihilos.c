#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "../sala.h"
#include "../retardo.h"

typedef struct {
    int id_hilo;
} datos_hilo;

int liberadores_activos = 0;
pthread_mutex_t mutex_activos = PTHREAD_MUTEX_INITIALIZER;

int quedan_liberadores() {
    pthread_mutex_lock(&mutex_activos);
    int res = liberadores_activos;
    pthread_mutex_unlock(&mutex_activos);
    return res;
}

// Indicamos que esta función está definida en sala.c
extern void despierta_hilos_bloqueados();

void* hilo_reserva(void* arg) {
    int id_persona = *(int*)arg;
    for (int i = 0; i < 3; i++) {
        pausa_aleatoria(0.8);
        int asiento = reserva_asiento(id_persona);
        if (asiento != -1) {
            printf("Hilo Reserva %d: Sentado en asiento %d\n", id_persona, asiento);
        } else {
            printf("Hilo Reserva %d: No pudo sentarse (Sala llena y sin liberadores). Terminando...\n", id_persona);
            break; // Rompe el bucle si la API deniega la reserva definitivamente
        }
    }
    free(arg);
    return NULL;
}

void* hilo_libera(void* arg) {
    int id_hilo = *(int*)arg;

    // Al arrancar, este hilo se suma a los activos
    pthread_mutex_lock(&mutex_activos);
    liberadores_activos++;
    pthread_mutex_unlock(&mutex_activos);

    int liberaciones = 0;
    while (liberaciones < 3) {
        pausa_aleatoria(1.0);
        for (int i = 1; i <= 5; i++) {
            int p = libera_asiento(i);
            if (p != -1) {
                printf("Hilo Libera %d: He echado a la persona %d del asiento %d\n", id_hilo, p, i);
                liberaciones++;
                break; // Encontramos y liberamos uno, pasamos a la siguiente ronda del while
            }
        }
    }

    // Al terminar sus 3 vueltas y antes de morir, se resta
    pthread_mutex_lock(&mutex_activos);
    liberadores_activos--;

    // Si era el ÚLTIMO hilo liberador vivo, despertamos de golpe a los reservas bloqueados
    if (liberadores_activos == 0) {
        despierta_hilos_bloqueados();
    }
    pthread_mutex_unlock(&mutex_activos);

    free(arg);
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <n_hilos_reserva> <m_hilos_libera>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    int m = atoi(argv[2]);
    pthread_t h_res[n], h_lib[m];

    // Creamos una sala pequeña de 5 asientos
    crea_sala(5);

    // Lanzar hilos de reserva (IDs 100+)
    for (int i = 0; i < n; i++) {
        int* id = malloc(sizeof(int)); *id = 100 + i;
        pthread_create(&h_res[i], NULL, hilo_reserva, id);
    }

    // Lanzar hilos de liberación (IDs 200+)
    for (int i = 0; i < m; i++) {
        int* id = malloc(sizeof(int)); *id = 200 + i;
        pthread_create(&h_lib[i], NULL, hilo_libera, id);
    }

    // Esperar a que terminen las ejecuciones
    for (int i = 0; i < n; i++) pthread_join(h_res[i], NULL);
    for (int i = 0; i < m; i++) pthread_join(h_lib[i], NULL);

    elimina_sala();
    printf("Simulación finalizada de forma segura.\n");
    return 0;
}