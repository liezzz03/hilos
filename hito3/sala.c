#include <stdlib.h>
#include <stdio.h>
#include "../sala.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> // para open() y las constantes O_RDONLY, ...
#include <unistd.h> // para read, write, close
#include "../retardo.h"
#include <pthread.h> // Necesario para el mutex

static int* asientos = NULL; // Puntero al arreglo de asientos (NULL si no hay sala creada)
static int n_asientos = 0; //  número total de asientos en la sala

static pthread_mutex_t cerrojo = PTHREAD_MUTEX_INITIALIZER;

static pthread_cond_t cond_hay_sitio = PTHREAD_COND_INITIALIZER;
static pthread_cond_t cond_hay_reservas = PTHREAD_COND_INITIALIZER;

// Función externa definida en multihilos.c para consultar liberadores activos
extern int quedan_liberadores();

// Función auxiliar interna para contar sin bloquear (evita interbloqueos)
int asientos_libres_interna() {
    int ocupados = 0;
    for (int i = 0; i < n_asientos; i++) {
        if (asientos[i] != -1) ocupados++;
    }
    return n_asientos - ocupados;
}

// Crea una nueva sala con la capacidad especificada
// Retorna el número de asientos si se crea correctamente, -1 en caso de error
int crea_sala(int capacidad) {
    // SIN CERROJO: Función de inicialización
    if (asientos != NULL || capacidad < 1) return -1;

    asientos = (int*) malloc(capacidad * sizeof(int));
    if (asientos == NULL) return -1; // Error de memoria

    n_asientos = capacidad;
    // Inicializar todos los asientos como libres (-1 indica libre)
    for (int i = 0; i < n_asientos; i++) {
        asientos[i] = -1;
    }
    return n_asientos;
}

// Retorna la capacidad total de la sala, o -1 si no existe
int capacidad_sala() {
    // SIN CERROJO: Solo lectura
    if (asientos == NULL) return -1;
    return n_asientos;
}

// Retorna el número de asientos ocupados, o -1 si no existe sala
int asientos_ocupados() {
    // SIN CERROJO: Solo lectura
    if (asientos == NULL) return -1;
    int ocupados = 0;
    for (int i = 0; i < n_asientos; i++) {
        // Asiento ocupado si no es -1
        if (asientos[i] != -1) {
            ocupados++;
        }
    }
    return ocupados;
}

// Retorna el número de asientos libres, o -1 si no existe la sala
int asientos_libres() {
    // SIN CERROJO: Solo lectura
    if (asientos == NULL) return -1;

    int ocupados = 0;
    for (int i = 0; i < n_asientos; i++) {
        if (asientos[i] != -1) ocupados++;
    }

    return n_asientos - ocupados;
}

// Reserva el primer asiento libre para la persona con id_persona
// Cancela la ejecución si la sala está llena y ya no quedan liberadores activos
int reserva_asiento(int id_persona) {
    // 1. Validaciones previas sin cerrojo para mayor eficiencia
    if (asientos == NULL || id_persona <= 0) return -1;

    // 2. SECCIÓN CRÍTICA: Bloqueamos porque usamos pthread_cond_wait y modificamos el array
    pthread_mutex_lock(&cerrojo);

    // El hilo espera MIENTRAS la sala esté llena Y ADEMÁS sigan quedando liberadores vivos trabajando
    while (asientos_libres_interna() == 0 && quedan_liberadores() > 0) {
        pthread_cond_wait(&cond_hay_sitio, &cerrojo);
    }

    // Si salimos del bucle porque la sala está llena pero ya NO quedan liberadores activos...
    if (asientos_libres_interna() == 0) {
        printf("[SALA] Hilo %d: Sala llena y no quedan liberadores activos. Cancelando y terminando.\n", id_persona);
        pthread_mutex_unlock(&cerrojo);
        return -1; // Retorna error controlado para abortar el interbloqueo
    }

    // Si salimos del bucle porque realmente hay sitio libre, reservamos normalmente:
    for (int i = 0; i < n_asientos; i++) {
        if (asientos[i] == -1) {
            asientos[i] = id_persona;
            pthread_cond_broadcast(&cond_hay_reservas);
            pthread_mutex_unlock(&cerrojo);
            return (i + 1);
        }
    }
    pthread_mutex_unlock(&cerrojo);
    return -1;
}

// Libera el asiento especificado por id_asiento
// Retorna el id de la persona que lo ocupaba, o -1 si falla
int libera_asiento(int id_asiento) {
    // 1. Validaciones previas sin cerrojo
    if (asientos == NULL || id_asiento < 1 || id_asiento > n_asientos) return -1;

    // 2. SECCIÓN CRÍTICA
    pthread_mutex_lock(&cerrojo);

    // Si la sala está entera libre, esperamos a que alguien lo reserve
    while (asientos_libres_interna() == n_asientos) {
        pthread_cond_wait(&cond_hay_reservas, &cerrojo);
    }

    // Comprobamos si el asiento concreto que queríamos liberar está vacío
    if (asientos[id_asiento-1] == -1) {
        pthread_mutex_unlock(&cerrojo);
        return -1;
    }

    int id_persona = asientos[id_asiento-1];
    asientos[id_asiento-1] = -1; // Liberar asiento

    // AVISAMOS de que hemos liberado un sitio
    pthread_cond_broadcast(&cond_hay_sitio);
    pthread_mutex_unlock(&cerrojo);
    return id_persona;
}

// Libera el asiento dependiendo del id de la persona
int libera_persona(int id_persona) {
    if (asientos == NULL || id_persona <= 0) return -1;

    pthread_mutex_lock(&cerrojo);
    for (int i = 0; i < n_asientos; i++) {
        if (asientos[i] == id_persona) {
            asientos[i] = -1; // Liberamos el asiento
            pthread_cond_broadcast(&cond_hay_sitio); // Avisamos que hay sitio libre
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
    // SIN CERROJO: Operación de destrucción (Se asume que ningún hilo trabaja ya)
    if (asientos == NULL) return -1;
    free(asientos);
    asientos = NULL;
    n_asientos = 0;
    return 0;
}

// Retorna el estado del asiento: 0 si libre, id de persona si ocupado, -1 si error
int estado_asiento(int id_asiento) {
    // SIN CERROJO: Solo lectura
    if (asientos == NULL || id_asiento < 1 || id_asiento > n_asientos) return -1;
    int val = asientos[id_asiento - 1];
    return (val == -1) ? 0 : val;
}

// Reserva un asiento específico para la persona con id_persona
// Retorna el id del asiento si se reserva, -1 si falla
int reserva_asiento_especifico(int id_asiento, int id_persona) {
    if (asientos == NULL || id_persona <= 0 || id_asiento < 1 || id_asiento > n_asientos) return -1;

    pthread_mutex_lock(&cerrojo);
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
    if (npersonas <= 0 || lista_id == NULL || asientos == NULL) return -1;

    pthread_mutex_lock(&cerrojo);

    int ocupados = 0;
    for (int i = 0; i < n_asientos; i++) if (asientos[i] != -1) ocupados++;
    int libres = n_asientos - ocupados;

    if (libres < npersonas) {
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

// --- FUNCIONES DE PERSISTENCIA (SIN CERROJOS - SE ASUME USO SECUENCIAL) ---

int guarda_estado_sala(const char* ruta_fichero){
    if (asientos == NULL) return -1;
    int fd = open(ruta_fichero, O_WRONLY | O_CREAT | O_TRUNC, 0664);
    if (fd == -1) return -1;
    if (write(fd, &n_asientos, sizeof(int)) != sizeof(int)) {
       close(fd);
       return -1;
    }
    size_t bytes_a_escribir = n_asientos * sizeof(int);
    if (write(fd, asientos, bytes_a_escribir) != (ssize_t)bytes_a_escribir) {
       close(fd);
       return -1;
    }
    close(fd);
    return 0;
}

int recupera_estado_sala(const char* ruta_fichero) {
    if (asientos == NULL) return -1;
    int fd = open(ruta_fichero, O_RDONLY);
    if (fd == -1) return -1;
    int capacidad_fichero;
    if (read(fd, &capacidad_fichero, sizeof(int)) != sizeof(int)) {
        close(fd);
        return -1;
    }
    if (capacidad_fichero != n_asientos) {
        close(fd);
        return -1;
    }
    size_t bytes_a_leer = n_asientos * sizeof(int);
    if (read(fd, asientos, bytes_a_leer) != (ssize_t)bytes_a_leer) {
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

int guarda_estado_parcial_sala(const char* ruta_fichero, size_t num_asientos, int* id_asientos) {
    if (asientos == NULL || id_asientos == NULL || num_asientos <= 0) return -1;
    int fd = open(ruta_fichero, O_RDWR);
    if (fd == -1) return -1;
    int capacidad_fichero;
    if (read(fd, &capacidad_fichero, sizeof(int)) != sizeof(int)) {
        close(fd);
        return -1;
    }
    if (capacidad_fichero != n_asientos) {
        close(fd);
        return -1;
    }
    for (size_t k = 0; k < num_asientos; k++) {
        int i = id_asientos[k];
        if (i < 1 || i > n_asientos) continue;

        off_t posicion = sizeof(int) + (i - 1) * sizeof(int);
        if (lseek(fd, posicion, SEEK_SET) == -1) {
            close(fd);
            return -1;
        }
        if (write(fd, &asientos[i - 1], sizeof(int)) != sizeof(int)) {
            close(fd);
            return -1;
        }
    }
    close(fd);
    return 0;
}

int recupera_estado_parcial_sala(const char* ruta_fichero, size_t num_asientos, int* id_asientos) {
    if (asientos == NULL || id_asientos == NULL || num_asientos <= 0) return -1;
    int fd = open(ruta_fichero, O_RDONLY);
    if (fd == -1) return -1;
    int capacidad_fichero;
    if (read(fd, &capacidad_fichero, sizeof(int)) != sizeof(int)) {
        close(fd);
        return -1;
    }
    if (capacidad_fichero != n_asientos) {
        close(fd);
        return -1;
    }
    for (size_t k = 0; k < num_asientos; k++) {
        int i = id_asientos[k];
        if (i < 1 || i > n_asientos) continue;

        off_t posicion = sizeof(int) + (i - 1) * sizeof(int);
        if (lseek(fd, posicion, SEEK_SET) == -1) {
            close(fd);
            return -1;
        }
        if (read(fd, &asientos[i - 1], sizeof(int)) != sizeof(int)) {
            close(fd);
            return -1;
        }
    }
    close(fd);
    return 0;
}

// Función para forzar el despertar de todos los reservas cuando los liberadores mueren
void despierta_hilos_bloqueados() {
    // ESTA SÍ LLEVA CERROJO: Interactúa con pthread_cond_broadcast
    pthread_mutex_lock(&cerrojo);
    pthread_cond_broadcast(&cond_hay_sitio);
    pthread_mutex_unlock(&cerrojo);
}