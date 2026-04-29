#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sala.h"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Error: Argumentos insuficientes.\n");
        return -1;
    }

    char *ciudad = argv[1];
    int capacidad = atoi(argv[2]);

    if (crea_sala(capacidad) == -1) {
        printf("Error al crear la sala en %s.\n", ciudad);
        return -1;
    }

    char comando[100];
    int parametro;

    printf("  Sucursal teatro: %s\n", ciudad);
    printf("  Capacidad: %d asientos\n", capacidad);
    printf("Comandos disponibles:\n");
    printf(" - reserva <id_persona>\n");
    printf(" - libera_asiento <num_asiento>\n");
    printf(" - libera_persona <id_persona>\n");
    printf(" - estado_asiento <num_asiento>\n");
    printf(" - estado_sala\n");
    printf(" - cerrar_sala\n");

    while (1) {
        printf("\n%s> ", ciudad);
        fflush(stdout);

        if (scanf("%s", comando) <= 0) break;

        if (strcmp(comando, "reserva") == 0) {
            if (scanf("%d", &parametro) == 1) {
                int asiento = reserva_asiento(parametro);
                if (asiento == -1) printf("No hay asientos libres o ID invalido.\n");
                else printf("Asiento %d reservado para persona %d.\n", asiento, parametro);
            }
        }
        else if (strcmp(comando, "libera_asiento") == 0) {
            if (scanf("%d", &parametro) == 1) {
                int persona = libera_asiento(parametro);
                if (persona == -1) printf("Asiento %d ya estaba libre o no existe.\n", parametro);
                else printf("Asiento %d liberado (estaba la persona %d).\n", parametro, persona);
            }
        }
        else if (strcmp(comando, "libera_persona") == 0) {
            if (scanf("%d", &parametro) == 1) {
                int asiento = libera_persona(parametro);
                if (asiento == -1) printf("La persona %d no esta en la sala.\n", parametro);
                else printf("Persona %d localizada y liberada del asiento %d.\n", parametro, asiento);
            }
        }
        else if (strcmp(comando, "estado_asiento") == 0) {
            if (scanf("%d", &parametro) == 1) {
                int estado = estado_asiento(parametro);
                if (estado == -1) printf("Asiento %d: LIBRE.\n", parametro);
                else printf("Asiento %d: OCUPADO por persona %d.\n", parametro, estado);
            }
        }
        else if (strcmp(comando, "estado_sala") == 0) {
            printf("Capacidad: %d | Libres: %d | Ocupados: %d\n",
                   capacidad_sala(), asientos_libres(), asientos_ocupados());
        }
        else if (strcmp(comando, "cerrar_sala") == 0) {
            int libres = asientos_libres();
            printf("Cerrando sucursal %s. Reportando estado al maestro...\n", ciudad);
            elimina_sala();

            // Lógica del RETO: 0 si llena, 1 si quedan libres
            exit(libres == 0 ? 0 : 1);
        }
        else {
            printf("Comando desconocido: %s\n", comando);
            while (getchar() != '\n');
        }
    }
    return 0;
}