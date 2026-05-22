#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "../sala.h"
#include "../retardo.h"

int hombres_en_sala = 0;
int mujeres_en_sala = 0;
pthread_mutex_t mutex_reto = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_ratio = PTHREAD_COND_INITIALIZER;

void* hilo_persona(void* arg) {
    int id = *(int*)arg;
    // Convención: IDs < 200 son hombres, IDs >= 200 son mujeres
    int es_mujer = (id >= 200); 

    // control de ratio 60%
    pthread_mutex_lock(&mutex_reto);
    int total = hombres_en_sala + mujeres_en_sala;

    // Regla: si total >= 10, ningún sexo puede superar el 60%
    while (total >= 10 && (
          (es_mujer && (mujeres_en_sala + 1) > (total + 1) * 0.6) || 
          (!es_mujer && (hombres_en_sala + 1) > (total + 1) * 0.6)
    )) {
        printf("Hilo %d (%s) ESPERANDO: la ratio superaría el 60%%.\n", id, es_mujer ? "Mujer" : "Hombre");
        pthread_cond_wait(&cond_ratio, &mutex_reto);
        total = hombres_en_sala + mujeres_en_sala; // Recalcular al despertar
    }

    // Intentamos la reserva real en la sala
    int asiento = reserva_asiento(id);
    if (asiento != -1) {
        if (es_mujer) mujeres_en_sala++; else hombres_en_sala++;
        printf("Hilo %d (%s) ENTRA. Asiento %d. (Hombres: %d, Mujeres: %d)\n", 
                id, es_mujer ? "Mujer" : "Hombre", asiento, hombres_en_sala, mujeres_en_sala);
    }
    pthread_mutex_unlock(&mutex_reto);

    // Simulación de tiempo de permanencia
    pausa_aleatoria(1.5);

    // salida
    pthread_mutex_lock(&mutex_reto);
    if (libera_asiento(asiento) != -1) {
        if (es_mujer) mujeres_en_sala--; else hombres_en_sala--;
        printf("Hilo %d (%s) SALE. (Hombres: %d, Mujeres: %d)\n", 
                id, es_mujer ? "Mujer" : "Hombre", hombres_en_sala, mujeres_en_sala);
        
        // Al salir alguien, la ratio cambia; avisamos a los que esperan
        pthread_cond_broadcast(&cond_ratio); 
    }
    pthread_mutex_unlock(&mutex_reto);

    free(arg);
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <nhombres> <nmujeres>\n", argv[0]);
        return 1;
    }

    int nh = atoi(argv[1]);
    int nm = atoi(argv[2]);
    pthread_t hilos[nh + nm];

    // Creamos una sala con capacidad suficiente para pasar el umbral de 10
    crea_sala(20); 

    // Lanzar hilos hombres
    for (int i = 0; i < nh; i++) {
        int* id = malloc(sizeof(int)); *id = 100 + i;
        pthread_create(&hilos[i], NULL, hilo_persona, id);
    }
    // Lanzar hilos mujeres
    for (int i = 0; i < nm; i++) {
        int* id = malloc(sizeof(int)); *id = 200 + i;
        pthread_create(&hilos[nh + i], NULL, hilo_persona, id);
    }

    for (int i = 0; i < (nh + nm); i++) {
        pthread_join(hilos[i], NULL);
    }

    elimina_sala();
    printf("Simulación del reto finalizada.\n");
    return 0;
}