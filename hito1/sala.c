#include <stdlib.h>
#include "../sala.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "../retardo.h"

static int* asientos = NULL;
static int n_asientos = 0;

int crea_sala(int capacidad) {
    if (asientos != NULL || capacidad < 1) return -1;
    asientos = (int*) malloc(capacidad * sizeof(int));
    if (asientos == NULL) return -1;

    n_asientos = capacidad;
    for (int i = 0; i < n_asientos; i++) {
        asientos[i] = -1;
    }
    return n_asientos;
}

int capacidad_sala() {
    if (asientos == NULL) return -1;
    return n_asientos;
}

int asientos_ocupados() {
    if (asientos == NULL) return -1;
    int ocupados = 0;
    for (int i = 0; i < n_asientos; i++) {
        if (asientos[i] != -1) ocupados++;
    }
    return ocupados;
}

int asientos_libres() {
    if (asientos == NULL) return -1;
    int ocupados = asientos_ocupados();
    if (ocupados == -1) return -1;
    return n_asientos - ocupados;
}

int reserva_asiento(int id_persona) {
    if (asientos == NULL || id_persona <= 0) return -1;

    for (int i = 0; i < n_asientos; i++) {
        if (asientos[i] == -1) {
            // Simulamos que el sistema operativo cambia de hilo justo aquí
            pausa_aleatoria(0.1);
            asientos[i] = id_persona;
            return (i + 1);
        }
    }
    return -1;
}

int libera_asiento(int id_asiento) {
    if (asientos == NULL || id_asiento < 1 || id_asiento > n_asientos) return -1;

    int index = id_asiento - 1;
    if (asientos[index] == -1) return -1;

    int id_persona = asientos[index];
    asientos[index] = -1;
    return id_persona;
}

int libera_persona(int id_persona) {
    if (asientos == NULL || id_persona <= 0) return -1;
    for (int i = 0; i < n_asientos; i++) {
        if (asientos[i] == id_persona) {
            asientos[i] = -1;
            return (i + 1);
        }
    }
    return -1;
}

int elimina_sala() {
    if (asientos == NULL) return -1;
    free(asientos);
    asientos = NULL;
    n_asientos = 0;
    return 0;
}

int estado_asiento(int id_asiento) {
    if (asientos == NULL || id_asiento < 1 || id_asiento > n_asientos) return -1;
    int val = asientos[id_asiento - 1];
    return (val == -1) ? 0 : val;
}

int reserva_asiento_especifico(int id_asiento, int id_persona) {
    if (asientos == NULL || id_persona <= 0) return -1;
    if (id_asiento < 1 || id_asiento > n_asientos) return -1;

    if (asientos[id_asiento - 1] != -1) return -1;

    asientos[id_asiento - 1] = id_persona;
    return id_asiento;
}

int reserva_multiple(int npersonas, int* lista_id) {
    if (npersonas <= 0 || lista_id == NULL) return -1;
    if (asientos_libres() < npersonas) return -1;

    int exitos = 0;
    for (int i = 0; i < npersonas; i++) {
        if (reserva_asiento(lista_id[i]) != -1) exitos++;
    }
    return exitos;
}
