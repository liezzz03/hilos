#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "sala.h"
#include "retardo.h"

//HITO 3
typedef struct {
    int id_hilo;
} datos_hilo;

void* funcion_hilo(void* arg) {
    datos_hilo* datos = (datos_hilo*)arg;
    int mis_asientos[3];
    int reservas_hechas = 0;

    // intentar 3 reservas con retardo
    for (int i = 0; i < 3; i++) {
        // pausa aleatoria para forzar que los hilos se mezclen
        pausa_aleatoria(0.5);

        int asiento = reserva_asiento(datos->id_hilo);
        if (asiento != -1) {
            mis_asientos[reservas_hechas] = asiento;
            reservas_hechas++;
            printf("Hilo %d: Reservado asiento %d\n", datos->id_hilo, asiento);
        } else {
            printf("Hilo %d: Sala llena, terminando...\n", datos->id_hilo);
            pthread_exit(NULL);
        }
    }

    // intentar 3 liberaciones con retardo
    for (int i = 0; i < reservas_hechas; i++) {
        pausa_aleatoria(0.5);

        int resultado = libera_asiento(mis_asientos[i]);
        if (resultado == -1) {
            // este error evidencia la corrupción: alguien nos quitó el asiento
            fprintf(stderr, "ERROR CORRUPCIÓN: Hilo %d no pudo liberar su asiento %d\n",
                    datos->id_hilo, mis_asientos[i]);
        } else {
            printf("Hilo %d: Liberado asiento %d\n", datos->id_hilo, mis_asientos[i]);
        }
    }

    free(datos);
    pthread_exit(NULL);
}

void* hilo_estado(void* arg) {
    while (1) {
        pausa_aleatoria(1.0);
        printf("\n--- ESTADO ACTUAL: %d ocupados ---\n\n", asientos_ocupados());
    }
    return NULL;
}

void* hilo_reserva(void* arg) {
    int id_persona = *(int*)arg;
    for (int i = 0; i < 3; i++) {
        pausa_aleatoria(0.8);
        int asiento = reserva_asiento(id_persona);
        printf("Hilo Reserva %d: Sentado en asiento %d\n", id_persona, asiento);
    }
    free(arg);
    return NULL;
}

void* hilo_libera(void* arg) {
    int id_hilo = *(int*)arg;
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
    free(arg);
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <n_hilos_reserva> <m_hilos_libera>\n", argv[0]);        return 1;
    }

    int n = atoi(argv[1]);
    int m = atoi(argv[2]);
    pthread_t h_res[n], h_lib[m];

    // creamos una sala pequeña para forzar colisiones
    crea_sala(5);

    for (int i = 0; i < n; i++) {
        int* id = malloc(sizeof(int)); *id = 100 + i;
        pthread_create(&h_res[i], NULL, hilo_reserva, id);
    }
    for (int i = 0; i < m; i++) {
        int* id = malloc(sizeof(int)); *id = 200 + i;
        pthread_create(&h_lib[i], NULL, hilo_libera, id);
    }

    for (int i = 0; i < n; i++) pthread_join(h_res[i], NULL);
    for (int i = 0; i < m; i++) pthread_join(h_lib[i], NULL);

    return 0;
}