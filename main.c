#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include "sucursal.h"

typedef struct {
    pid_t pid;
    char ciudad[100];
} Sucursal;

Sucursal tabla[100];
int total_salas = 0;

void manejador_muerte_hijo(int senal) {
    int estado;
    pid_t pid;
    while ((pid = waitpid(-1, &estado, WNOHANG)) > 0) {
        if (WIFEXITED(estado)) {
            int codigo = WEXITSTATUS(estado);
            for (int i = 0; i < total_salas; i++) {
                if (tabla[i].pid == pid) {
                    printf("\nNotificación inmediata: Sala '%s' cerrada.\n", tabla[i].ciudad);
                    if (codigo == 0)
                        printf("Estado final: éxito total (Sala llena).\n");
                    else
                        printf("Estado final: cierre con asientos aún libres.\n");
                    printf("Introduce nombre de la ciudad (o 'salir'): ");
                    fflush(stdout);
                    break;
                }
            }
        }
    }
}

int main() {
    struct sigaction sa;
    sa.sa_handler = manejador_muerte_hijo;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &sa, NULL);

    char ciudad[100];
    int cap;

    while (1) {
        printf("Introduce nombre de la ciudad (o 'salir'): ");
        if (scanf("%s", ciudad) != 1) break;

        if (strcmp(ciudad, "salir") == 0) {
            printf("Cerrando maestro y esperando a las salas...\n");
            while (wait(NULL) > 0);
            break;
        } //
        // Created by paula on 23/03/2026.
        //

#include "sucursal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>


        pid_t crea_sucursal(const char *ciudad, int capacidad) {
            pid_t pid = fork();
            if (pid < 0) {
                perror("Error al crear el proceso hijo");
                return -1;
            }

            if (pid == 0) {
                // convertimos la capacidad a string para pasarla como argumento
                char cap_str[10];
                sprintf(cap_str, "%d", capacidad);
                // execlp busca el ejecutable en el path automáticamente
                execlp("gnome-terminal", "gnome-terminal", "--wait", "--", "./mini_shell", ciudad, cap_str, NULL);
                // si execlp falla, error
                perror("Error al ejecutar gnome-terminal");
                exit(1);
            }
            return pid;
        } //
        // Created by paula on 23/03/2026.
        //

#include "sucursal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>


        pid_t crea_sucursal(const char *ciudad, int capacidad) {
            pid_t pid = fork();
            if (pid < 0) {
                perror("Error al crear el proceso hijo");
                return -1;
            }

            if (pid == 0) {
                // convertimos la capacidad a string para pasarla como argumento
                char cap_str[10];
                sprintf(cap_str, "%d", capacidad);
                // execlp busca el ejecutable en el path automáticamente
                execlp("gnome-terminal", "gnome-terminal", "--wait", "--", "./mini_shell", ciudad, cap_str, NULL);
                // si execlp falla, error
                perror("Error al ejecutar gnome-terminal");
                exit(1);
            }
            return pid;
        } //
        // Created by paula on 23/03/2026.
        //

#include "sucursal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>


        pid_t crea_sucursal(const char *ciudad, int capacidad) {
            pid_t pid = fork();
            if (pid < 0) {
                perror("Error al crear el proceso hijo");
                return -1;
            }

            if (pid == 0) {
                // convertimos la capacidad a string para pasarla como argumento
                char cap_str[10];
                sprintf(cap_str, "%d", capacidad);
                // execlp busca el ejecutable en el path automáticamente
                execlp("gnome-terminal", "gnome-terminal", "--wait", "--", "./mini_shell", ciudad, cap_str, NULL);
                // si execlp falla, error
                perror("Error al ejecutar gnome-terminal");
                exit(1);
            }
            return pid;
        }

        printf("Introduce capacidad para %s: ", ciudad);
        if (scanf("%d", &cap) != 1) {
            printf("Capacidad no valida.\n");
            while (getchar() != '\n');
            continue;
        }

        pid_t p = crea_sucursal(ciudad, cap);
        if (p > 0) {
            tabla[total_salas].pid = p;
            strcpy(tabla[total_salas].ciudad, ciudad);
            total_salas++;
            printf("Sucursal '%s' lanzada con PID %d.\n", ciudad, p);
        }
    }
    return 0;
}
