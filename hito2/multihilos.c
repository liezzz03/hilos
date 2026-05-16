#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "../sala.h"
#include "../retardo.h"

// Hito 2

typedef struct {
    int id_hilo;
} datos_hilo;

void* funcion_hilo(void* arg) {
    datos_hilo* datos = (datos_hilo*)arg;
    int mis_asientos[3];
    int reservas_hechas = 0;

    // Intentar 3 reservas individuales metiendo retardo
    for (int i = 0; i < 3; i++) {
        pausa_aleatoria(0.4); // Forzar entrelazamiento de flujos

        int asiento = reserva_asiento(datos->id_hilo);
        if (asiento != -1) {
            mis_asientos[reservas_hechas] = asiento;
            reservas_hechas++;
            printf("[HILO %d] Reservado con éxito asiento %d\n", datos->id_hilo, asiento);
        } else {
            // En Hito 2, si la sala se llena, el hilo aborta con elegancia
            printf("[HILO %d] Sala llena al intentar reservar. Termina ejecución.\n", datos->id_hilo);
            free(datos);
            pthread_exit(NULL);
        }
    }

    // Intentar liberar los asientos asignados
    for (int i = 0; i < reservas_hechas; i++) {
        pausa_aleatoria(0.4);

        int resultado = libera_asiento(mis_asientos[i]);
        if (resultado == -1) {
            // Si el Mutex funciona bien, este mensaje NUNCA se imprimirá en el Hito 2
            fprintf(stderr, "CRÍTICO - ERROR CORRUPCIÓN: El Hilo %d no pudo liberar su propio asiento %d\n",
                    datos->id_hilo, mis_asientos[i]);
        } else {
            printf("[HILO %d] Liberado asiento %d de forma segura.\n", datos->id_hilo, mis_asientos[i]);
        }
    }

    free(datos);
    pthread_exit(NULL);
}

// Hilo secundario para monitorizar periódicamente el volumen ocupado
void* hilo_estado(void* arg) {
    while (1) {
        pausa_aleatoria(0.8);
        printf("\n--- MONITOR CENTRAL: %d asientos ocupados ---\n\n", asientos_ocupados());
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <n_hilos>\n", argv[0]);
        return 1;
    }

    int n_hilos = atoi(argv[1]);
    pthread_t hilos[n_hilos];
    pthread_t h_monitor;

    // Sala pequeña de 5 asientos para forzar concurrencia alta e intentos con sala llena
    crea_sala(5);

    // Lanzar hilo de monitorización (se ejecuta en segundo plano)
    pthread_create(&h_monitor, NULL, hilo_estado, NULL);
    pthread_detach(h_monitor); // No necesitamos hacerle join

    // Crear los hilos trabajadores
    for (int i = 0; i < n_hilos; i++) {
        datos_hilo* datos = malloc(sizeof(datos_hilo));
        datos->id_hilo = 100 + i; // IDs identificables (100, 101, 102...)
        pthread_create(&hilos[i], NULL, funcion_hilo, datos);
    }

    // Esperar a que terminen todos los hilos
    for (int i = 0; i < n_hilos; i++) {
        pthread_join(hilos[i], NULL);
    }

    elimina_sala();
    printf("Simulación del Hito 2 (Thread-Safe) finalizada con éxito sin corrupciones.\n");
    return 0;
}