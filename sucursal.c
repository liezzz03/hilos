#include "sucursal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>


pid_t crea_sucursal(const char* ciudad, int capacidad) {
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