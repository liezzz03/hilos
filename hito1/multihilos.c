#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "../sala.h"
#include "../retardo.h"

typedef struct {
    int id_hilo;
} datos_hilo;

void* funcion_hilo(void* arg) {
    datos_hilo* datos = (datos_hilo*)arg;
    int mis_asientos[3];
    int reservas_hechas = 0;

    // 1. Intentar 3 reservas con retardo
    for (int i = 0; i < 3; i++) {
        // Pausa aleatoria para forzar que los hilos se mezclen
        pausa_aleatoria(0.5);

        int asiento = reserva_asiento(datos->id_hilo);
        if (asiento != -1) {
            mis_asientos[reservas_hechas] = asiento;
            reservas_hechas++;
            printf("Hilo %d: Reservado asiento %d\n", datos->id_hilo, asiento);
        } else {
            printf("Hilo %d: Sala llena, terminando reservas...\n", datos->id_hilo);
            break; // Si se llena, paramos de intentar reservar y pasamos a liberar
        }
    }

    // 2. Intentar liberar los asientos que logramos reservar
    for (int i = 0; i < reservas_hechas; i++) {
        pausa_aleatoria(0.5);

        int resultado = libera_asiento(mis_asientos[i]);
        if (resultado == -1 || resultado != datos->id_hilo) {
            // Este error evidencia la corrupción: alguien nos quitó o sobrescribió el asiento
            fprintf(stderr, ">>> ERROR CORRUPCIÓN: Hilo %d no pudo liberar su asiento %d (Estaba ocupado por %d o vacío)\n",
                    datos->id_hilo, mis_asientos[i], resultado);
        } else {
            printf("Hilo %d: Liberado asiento %d\n", datos->id_hilo, mis_asientos[i]);
        }
    }

    free(datos);
    return NULL;
}

void* hilo_estado(void* arg) {
    while (1) {
        pausa_aleatoria(1.0);
        printf("\n--- ESTADO ACTUAL: %d ocupados ---\n\n", asientos_ocupados());
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <num_hilos>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    pthread_t hilos[n];
    pthread_t h_estado;

    // Creamos una sala pequeña para forzar colisiones rápidas
    crea_sala(10);

    // Lanzamos hilo monitor de estado
    pthread_create(&h_estado, NULL, hilo_estado, NULL);
    pthread_detach(h_estado); // No esperamos por él, corre en segundo plano

    // Lanzamos los n hilos de trabajo
    for (int i = 0; i < n; i++) {
        datos_hilo* d = malloc(sizeof(datos_hilo));
        d->id_hilo = 100 + i; // IDs de personas a partir de 100
        pthread_create(&hilos[i], NULL, funcion_hilo, d);
    }

    // Esperamos a que todos terminen
    for (int i = 0; i < n; i++) {
        pthread_join(hilos[i], NULL);
    }

    printf("\nSimulación finalizada.\n");
    elimina_sala();
    return 0;
}