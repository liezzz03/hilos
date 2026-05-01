#include <stdlib.h>
#include "sala.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> // para open() y las constantes O_RDONLY, ...
#include <unistd.h> // para read, write, close
#include "retardo.h"
#include <pthread.h> // Necesario para el mutex

static int* asientos = NULL; // Puntero al arreglo de asientos (NULL si no hay sala creada)
static int n_asientos = 0; //  número total de asientos en la sala

static pthread_mutex_t cerrojo = PTHREAD_MUTEX_INITIALIZER;

//Crea una nueva sala con la capacidad especificada
//Retorna el número de asientos si se crea correctamente, -1 en caso de error
int crea_sala(int capacidad) {
    pthread_mutex_lock(&cerrojo);
    // No permitir crear una sala si ya existe una o si la capacidad es inválida
    if (asientos != NULL || capacidad < 1) {
        pthread_mutex_unlock(&cerrojo);
        return -1;
    }
    asientos = (int*) malloc(capacidad * sizeof(int));
    if (asientos == NULL) {
        pthread_mutex_unlock(&cerrojo);
        return -1; // Error de memoria
    }

    n_asientos = capacidad;
    // Inicializar todos los asientos como libres (-1 indica libre)
    for (int i = 0; i < n_asientos; i++) {
        asientos[i] = -1;
    }
    pthread_mutex_unlock(&cerrojo);
    return n_asientos;
}

// Retorna la capacidad total de la sala, o -1 si no existe
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

// Retorna el número de asientos ocupados, o -1 si no existe sala
int asientos_ocupados() {
    pthread_mutex_lock(&cerrojo);
    if (asientos == NULL) {
        pthread_mutex_unlock(&cerrojo);
        return -1;
    }
    int ocupados = 0;
    for (int i = 0; i < n_asientos; i++) {
        // Asiento ocupado si no es -1
        if (asientos[i] != -1) {
            ocupados++;
        }
    }
    pthread_mutex_unlock(&cerrojo);
    return ocupados;
}

// Retorna el número de asientos libres, o -1 si no existe la sala
int asientos_libres() {
    // No bloqueamos aquí porque llamamos a funciones que ya bloquean (asientos_ocupados)
    // Pero como asientos_ocupados bloquea y libera, es más seguro bloquear aquí
    // y usar lógica interna para evitar "deadlocks".
    // Sin embargo, para mantener tu estructura, bloquearemos y consultaremos directamente.
    pthread_mutex_lock(&cerrojo);
    if (asientos == NULL) {
        pthread_mutex_unlock(&cerrojo);
        return -1;
    }

    int ocupados = 0;
    for (int i = 0; i < n_asientos; i++) {
        if (asientos[i] != -1) ocupados++;
    }

    int libres = n_asientos - ocupados;
    pthread_mutex_unlock(&cerrojo);
    return libres;
}

//Reserva el primer asiento libre para la persona con id_persona
//Retorna el id del asiento reservado, o -1 si falla
int reserva_asiento(int id_persona) {
    pthread_mutex_lock(&cerrojo);
    if (asientos == NULL || id_persona <= 0) {
        pthread_mutex_unlock(&cerrojo);
        return -1;
    }

    for (int i = 0; i < n_asientos; i++) {
        if (asientos[i] == -1) { // Encontrar primer asiento libre
            // Aunque quites el comentario de la pausa, el mutex evitará el fallo
            // pausa_aleatoria(0.1);
            asientos[i] = id_persona;
            pthread_mutex_unlock(&cerrojo);
            return (i + 1); // Retornar id en rango [1, n_asientos]
        }
    }
    pthread_mutex_unlock(&cerrojo);
    return -1; // Sala llena
}

// Librea el asiento especificado por id_asiento
// Retorna el id de la persona que lo ocupaba, o -1 si falla
int libera_asiento(int id_asiento) {
    pthread_mutex_lock(&cerrojo);
    if (asientos == NULL || id_asiento < 1 || id_asiento > n_asientos) {
        pthread_mutex_unlock(&cerrojo);
        return -1;
    }

    int index = id_asiento - 1;
    if (asientos[index] == -1) {
        pthread_mutex_unlock(&cerrojo);
        return -1; // Asiento ya libre
    }

    int id_persona = asientos[index];
    asientos[index] = -1; // Liberar asiento
    pthread_mutex_unlock(&cerrojo);
    return id_persona;
}

// Libera el asiento dependiendo del id de la persona
int libera_persona(int id_persona) {
    pthread_mutex_lock(&cerrojo);
    if (asientos == NULL || id_persona <= 0) {
        pthread_mutex_unlock(&cerrojo);
        return -1;
    }
    for (int i = 0; i < n_asientos; i++) {
        if (asientos[i] == id_persona) {
            asientos[i] = -1; // Liberamos el asiento
            pthread_mutex_unlock(&cerrojo);
            return (i + 1);    // Devolvemos el número de asiento que ocupaba (1..N)
        }
    }
    pthread_mutex_unlock(&cerrojo);
    return -1; // No se encontró a esa persona en la sala
}

// Elimina la sala y libera la memoria
// Retorna 0 si se elimina correctamente, -1 si no existe la sala
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

// Retorna el estado del asiento: 0 si libre, id de persona si ocupado, -1 si error
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

// Reserva un asiento específico para la persona con id_persona
// Retorna el id del asiento si se reserva, -1 si falla
int reserva_asiento_especifico(int id_asiento, int id_persona) {
    pthread_mutex_lock(&cerrojo);
    if (asientos == NULL || id_persona <= 0) {
        pthread_mutex_unlock(&cerrojo);
        return -1;
    }
    if (id_asiento < 1 || id_asiento > n_asientos) {
        pthread_mutex_unlock(&cerrojo);
        return -1;
    }

    if (asientos[id_asiento - 1] != -1) {
        pthread_mutex_unlock(&cerrojo);
        return -1; // Asiento ya ocupado
    }

    asientos[id_asiento - 1] = id_persona;
    pthread_mutex_unlock(&cerrojo);
    return id_asiento;
}

// Implementación de la prueba de reserva múltiple (Todo o nada)
int reserva_multiple(int npersonas, int* lista_id) {
    // Al ser una operación que usa otras funciones de la API,
    // bloqueamos aquí y usamos lógica interna para no causar interbloqueo.
    pthread_mutex_lock(&cerrojo);

    if (npersonas <= 0 || lista_id == NULL || asientos == NULL) {
        pthread_mutex_unlock(&cerrojo);
        return -1;
    }

    int ocupados = 0;
    for (int i = 0; i < n_asientos; i++) if (asientos[i] != -1) ocupados++;
    int libres = n_asientos - ocupados;

    if (libres < npersonas) {
        pthread_mutex_unlock(&cerrojo);
        return -1;
    }

    int exitos = 0;
    for (int i = 0; i < npersonas; i++) {
        // Lógica de reserva directa para evitar llamar a reserva_asiento (que ya tiene lock)
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
    if (asientos == NULL) {
        pthread_mutex_unlock(&cerrojo);
        return -1;
    }
    // abrimos el fichero. 0664 permisos de lectura/escrityra (rw-rw-r--)
    int fd = open(ruta_fichero, O_WRONLY | O_CREAT | O_TRUNC, 0664);
    if (fd == -1) {
        pthread_mutex_unlock(&cerrojo);
        return -1;
    }
    if (write(fd, &n_asientos, sizeof(int)) != sizeof(int)) {
       close(fd);
       pthread_mutex_unlock(&cerrojo);
       return -1;
    }
    size_t bytes_a_escribir = n_asientos * sizeof(int);
    if (write(fd, asientos, bytes_a_escribir) != (ssize_t)bytes_a_escribir) {
       close(fd);
       pthread_mutex_unlock(&cerrojo);
       return -1;
    }
    close(fd);
    pthread_mutex_unlock(&cerrojo);
    return 0;
}

int recupera_estado_sala(const char* ruta_fichero) {
    pthread_mutex_lock(&cerrojo);
    if (asientos == NULL) {
        pthread_mutex_unlock(&cerrojo);
        return -1;
    }
    int fd = open(ruta_fichero, O_RDONLY);
    if (fd == -1) {
        pthread_mutex_unlock(&cerrojo);
        return -1;
    }
    int capacidad_fichero;
    //leemos la capacidad guardada en el fichero
    if (read(fd, &capacidad_fichero, sizeof(int)) != sizeof(int)) {
        close(fd);
        pthread_mutex_unlock(&cerrojo);
        return -1;
    }
    //la capacidad debe coincidir con la de la sala actual
    if (capacidad_fichero != n_asientos) {
        close(fd);
        pthread_mutex_unlock(&cerrojo);
        return -1;
    }
    //leemos el estado de los asientos y sobrescribimos el array actual
    size_t bytes_a_leer = n_asientos * sizeof(int);
    if (read(fd, asientos, bytes_a_leer) != (ssize_t)bytes_a_leer) {
        close(fd);
        pthread_mutex_unlock(&cerrojo);
        return -1;
    }
    close(fd);
    pthread_mutex_unlock(&cerrojo);
    return 0;
}

int guarda_estado_parcial_sala(const char* ruta_fichero, size_t num_asientos, int* id_asientos) {
    pthread_mutex_lock(&cerrojo);
    if (asientos == NULL || id_asientos == NULL || num_asientos <= 0) {
        pthread_mutex_unlock(&cerrojo);
        return -1;
    }
    // Abrimos en modo Lectura/Escritura (O_RDWR) porque el fichero debe existir
    int fd = open(ruta_fichero, O_RDWR);
    if (fd == -1) {
        pthread_mutex_unlock(&cerrojo);
        return -1;
    }
    int capacidad_fichero;
    if (read(fd, &capacidad_fichero, sizeof(int)) != sizeof(int)) {
        close(fd);
        pthread_mutex_unlock(&cerrojo);
        return -1;
    }
    if (capacidad_fichero != n_asientos) {
        close(fd);
        pthread_mutex_unlock(&cerrojo);
        return -1;
    }
    for (size_t k = 0; k < num_asientos; k++) {
        int i = id_asientos[k]; //número de asiento (ej: 1, 5, 10...)
        if (i < 1 || i > n_asientos) continue;

        off_t posicion = sizeof(int) + (i - 1) * sizeof(int);
        if (lseek(fd, posicion, SEEK_SET) == -1) {
            close(fd);
            pthread_mutex_unlock(&cerrojo);
            return -1;
        }
        if (write(fd, &asientos[i - 1], sizeof(int)) != sizeof(int)) {
            close(fd);
            pthread_mutex_unlock(&cerrojo);
            return -1;
        }
    }
    close(fd);
    pthread_mutex_unlock(&cerrojo);
    return 0;
}

int recupera_estado_parcial_sala(const char* ruta_fichero, size_t num_asientos, int* id_asientos) {
    pthread_mutex_lock(&cerrojo);
    if (asientos == NULL || id_asientos == NULL || num_asientos <= 0) {
        pthread_mutex_unlock(&cerrojo);
        return -1;
    }
    int fd = open(ruta_fichero, O_RDONLY);
    if (fd == -1) {
        pthread_mutex_unlock(&cerrojo);
        return -1;
    }
    int capacidad_fichero;
    if (read(fd, &capacidad_fichero, sizeof(int)) != sizeof(int)) {
        close(fd);
        pthread_mutex_unlock(&cerrojo);
        return -1;
    }
    if (capacidad_fichero != n_asientos) {
        close(fd);
        pthread_mutex_unlock(&cerrojo);
        return -1;
    }
    for (size_t k = 0; k < num_asientos; k++) {
        int i = id_asientos[k];
        if (i < 1 || i > n_asientos) continue;

        off_t posicion = sizeof(int) + (i - 1) * sizeof(int);
        if (lseek(fd, posicion, SEEK_SET) == -1) {
            close(fd);
            pthread_mutex_unlock(&cerrojo);
            return -1;
        }
        if (read(fd, &asientos[i - 1], sizeof(int)) != sizeof(int)) {
            close(fd);
            pthread_mutex_unlock(&cerrojo);
            return -1;
        }
    }
    close(fd);
    pthread_mutex_unlock(&cerrojo);
    return 0;
}