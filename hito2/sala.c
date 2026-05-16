#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h> // REQUISITO HITO 2: Para el Mutex
#include "../sala.h"

static int* asientos = NULL; 
static int n_asientos = 0;   

// Cerrojo global para la exclusión mutua
static pthread_mutex_t cerrojo = PTHREAD_MUTEX_INITIALIZER;

// Función auxiliar interna (SÓLO se llama si ya se posee el mutex)
// Evita el interbloqueo (deadlock) al no intentar bloquear el mutex otra vez.
static int asientos_libres_interna() {
    int ocupados = 0;
    for (int i = 0; i < n_asientos; i++) {
        if (asientos[i] != -1) ocupados++;
    }
    return n_asientos - ocupados;
}

int crea_sala(int capacidad) {
    pthread_mutex_lock(&cerrojo);
    if (asientos != NULL || capacidad < 1) {
        pthread_mutex_unlock(&cerrojo);
        return -1;
    }
    asientos = (int*) malloc(capacidad * sizeof(int));
    if (asientos == NULL) {
        pthread_mutex_unlock(&cerrojo);
        return -1; 
    }

    n_asientos = capacidad;
    for (int i = 0; i < n_asientos; i++) {
        asientos[i] = -1;
    }
    pthread_mutex_unlock(&cerrojo);
    return n_asientos;
}

int capacidad_sala() {
    pthread_mutex_lock(&cerrojo);
    if (asientos == NULL) {
        pthread_mutex_unlock(&cerrojo);
        return -1;
    }
    int res = n_asientos;
    pthread_mutex_unlock(&cerrojo);
    return res;
}

int asientos_ocupados() {
    pthread_mutex_lock(&cerrojo);
    if (asientos == NULL) {
        pthread_mutex_unlock(&cerrojo);
        return -1;
    }
    int ocupados = 0;
    for (int i = 0; i < n_asientos; i++) {
        if (asientos[i] != -1) ocupados++;
    }
    pthread_mutex_unlock(&cerrojo);
    return ocupados;
}

int asientos_libres() {
    pthread_mutex_lock(&cerrojo);
    if (asientos == NULL) {
        pthread_mutex_unlock(&cerrojo);
        return -1;
    }
    int libres = asientos_libres_interna();
    pthread_mutex_unlock(&cerrojo);
    return libres;
}

int reserva_asiento(int id_persona) {
    pthread_mutex_lock(&cerrojo);
    if (asientos == NULL || id_persona <= 0) {
        pthread_mutex_unlock(&cerrojo);
        return -1;
    }

    // HITO 2: Si está llena, salimos inmediatamente con -1 (no bloqueamos el hilo todavía)
    if (asientos_libres_interna() == 0) {
        pthread_mutex_unlock(&cerrojo);
        return -1; 
    }

    for (int i = 0; i < n_asientos; i++) {
        if (asientos[i] == -1) { 
            asientos[i] = id_persona;
            pthread_mutex_unlock(&cerrojo);
            return (i + 1); 
        }
    }
    pthread_mutex_unlock(&cerrojo);
    return -1; 
}

int libera_asiento(int id_asiento) {
    pthread_mutex_lock(&cerrojo);
    if (asientos == NULL || id_asiento < 1 || id_asiento > n_asientos) {
        pthread_mutex_unlock(&cerrojo);
        return -1;
    }

    int index = id_asiento - 1;
    if (asientos[index] == -1) {
        pthread_mutex_unlock(&cerrojo);
        return -1; 
    }

    int id_persona = asientos[index];
    asientos[index] = -1; 
    pthread_mutex_unlock(&cerrojo);
    return id_persona;
}

int libera_persona(int id_persona) {
    pthread_mutex_lock(&cerrojo);
    if (asientos == NULL || id_persona <= 0) {
        pthread_mutex_unlock(&cerrojo);
        return -1;
    }
    for (int i = 0; i < n_asientos; i++) {
        if (asientos[i] == id_persona) {
            asientos[i] = -1; 
            pthread_mutex_unlock(&cerrojo);
            return (i + 1);    
        }
    }
    pthread_mutex_unlock(&cerrojo);
    return -1; 
}

int elimina_sala() {
    pthread_mutex_lock(&cerrojo);
    if (asientos == NULL) {
        pthread_mutex_unlock(&cerrojo);
        return -1;
    }
    free(asientos);
    asientos = NULL;
    n_asientos = 0;
    pthread_mutex_unlock(&cerrojo);
    return 0;
}

int estado_asiento(int id_asiento) {
    pthread_mutex_lock(&cerrojo);
    if (asientos == NULL || id_asiento < 1 || id_asiento > n_asientos) {
        pthread_mutex_unlock(&cerrojo);
        return -1;
    }
    int val = asientos[id_asiento - 1];
    int res = (val == -1) ? 0 : val;
    pthread_mutex_unlock(&cerrojo);
    return res;
}

int reserva_asiento_specifico(int id_asiento, int id_persona) {
    pthread_mutex_lock(&cerrojo);
    if (asientos == NULL || id_persona <= 0 || id_asiento < 1 || id_asiento > n_asientos) {
        pthread_mutex_unlock(&cerrojo);
        return -1;
    }
    if (asientos[id_asiento - 1] != -1) {
        pthread_mutex_unlock(&cerrojo);
        return -1; 
    }
    asientos[id_asiento - 1] = id_persona;
    pthread_mutex_unlock(&cerrojo);
    return id_asiento;
}

int reserva_multiple(int npersonas, int* lista_id) {
    pthread_mutex_lock(&cerrojo);
    if (npersonas <= 0 || lista_id == NULL || asientos == NULL) {
        pthread_mutex_unlock(&cerrojo);
        return -1;
    }

    if (asientos_libres_interna() < npersonas) {
        pthread_mutex_unlock(&cerrojo);
        return -1;
    }

    int exitos = 0;
    for (int i = 0; i < npersonas; i++) {
        for (int j = 0; j < n_asientos; j++) {
            if (asientos[j] == -1) {
                asientos[j] = lista_id[i];
                exitos++;
                break;
            }
        }
    }
    pthread_mutex_unlock(&cerrojo);
    return exitos;
}

int guarda_estado_sala(const char* ruta_fichero){
    pthread_mutex_lock(&cerrojo);
    if (asientos == NULL) { pthread_mutex_unlock(&cerrojo); return -1; }
    int fd = open(ruta_fichero, O_WRONLY | O_CREAT | O_TRUNC, 0664);
    if (fd == -1) { pthread_mutex_unlock(&cerrojo); return -1; }
    if (write(fd, &n_asientos, sizeof(int)) != sizeof(int)) { close(fd); pthread_mutex_unlock(&cerrojo); return -1; }
    size_t bytes = n_asientos * sizeof(int);
    if (write(fd, asientos, bytes) != (ssize_t)bytes) { close(fd); pthread_mutex_unlock(&cerrojo); return -1; }
    close(fd); pthread_mutex_unlock(&cerrojo); return 0;
}

int recupera_estado_sala(const char* ruta_fichero) {
    pthread_mutex_lock(&cerrojo);
    if (asientos == NULL) { pthread_mutex_unlock(&cerrojo); return -1; }
    int fd = open(ruta_fichero, O_RDONLY);
    if (fd == -1) { pthread_mutex_unlock(&cerrojo); return -1; }
    int cap_fichero;
    if (read(fd, &cap_fichero, sizeof(int)) != sizeof(int)) { close(fd); pthread_mutex_unlock(&cerrojo); return -1; }
    if (cap_fichero != n_asientos) { close(fd); pthread_mutex_unlock(&cerrojo); return -1; }
    size_t bytes = n_asientos * sizeof(int);
    if (read(fd, asientos, bytes) != (ssize_t)bytes) { close(fd); pthread_mutex_unlock(&cerrojo); return -1; }
    close(fd); pthread_mutex_unlock(&cerrojo); return 0;
}

int guarda_estado_parcial_sala(const char* ruta_fichero, size_t num_asientos, int* id_asientos) {
    pthread_mutex_lock(&cerrojo);
    if (asientos == NULL || id_asientos == NULL || num_asientos <= 0) { pthread_mutex_unlock(&cerrojo); return -1; }
    int fd = open(ruta_fichero, O_RDWR);
    if (fd == -1) { pthread_mutex_unlock(&cerrojo); return -1; }
    int cap_fichero;
    if (read(fd, &cap_fichero, sizeof(int)) != sizeof(int)) { close(fd); pthread_mutex_unlock(&cerrojo); return -1; }
    if (cap_fichero != n_asientos) { close(fd); pthread_mutex_unlock(&cerrojo); return -1; }
    for (size_t k = 0; k < num_asientos; k++) {
        int i = id_asientos[k];
        if (i < 1 || i > n_asientos) continue;
        off_t posicion = sizeof(int) + (i - 1) * sizeof(int);
        if (lseek(fd, posicion, SEEK_SET) == -1) { close(fd); pthread_mutex_unlock(&cerrojo); return -1; }
        if (write(fd, &asientos[i - 1], sizeof(int)) != sizeof(int)) { close(fd); pthread_mutex_unlock(&cerrojo); return -1; }
    }
    close(fd); pthread_mutex_unlock(&cerrojo); return 0;
}

int recupera_estado_parcial_sala(const char* ruta_fichero, size_t num_asientos, int* id_asientos) {
    pthread_mutex_lock(&cerrojo);
    if (asientos == NULL || id_asientos == NULL || num_asientos <= 0) { pthread_mutex_unlock(&cerrojo); return -1; }
    int fd = open(ruta_fichero, O_RDONLY);
    if (fd == -1) { pthread_mutex_unlock(&cerrojo); return -1; }
    int cap_fichero;
    if (read(fd, &cap_fichero, sizeof(int)) != sizeof(int)) { close(fd); pthread_mutex_unlock(&cerrojo); return -1; }
    if (cap_fichero != n_asientos) { close(fd); pthread_mutex_unlock(&cerrojo); return -1; }
    for (size_t k = 0; k < num_asientos; k++) {
        int i = id_asientos[k];
        if (i < 1 || i > n_asientos) continue;
        off_t posicion = sizeof(int) + (i - 1) * sizeof(int);
        if (lseek(fd, posicion, SEEK_SET) == -1) { close(fd); pthread_mutex_unlock(&cerrojo); return -1; }
        if (read(fd, &asientos[i - 1], sizeof(int)) != sizeof(int)) { close(fd); pthread_mutex_unlock(&cerrojo); return -1; }
    }
    close(fd); pthread_mutex_unlock(&cerrojo); return 0;
}