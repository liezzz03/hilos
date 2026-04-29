#ifndef SALA_H
#define SALA_H

int crea_sala(int capacidad);
int elimina_sala();
int reserva_asiento(int id_persona);
int libera_asiento(int id_asiento);
int libera_persona(int id_persona);
int estado_asiento(int id_asiento);
int asientos_libres();
int asientos_ocupados();
int capacidad_sala();
int reserva_asiento_especifico(int id_asiento, int id_persona);
int reserva_multiple(int npersonas, int* lista_id);
int guarda_estado_sala(const char* ruta_fichero);
int recupera_estado_sala(const char* ruta_fichero);
int guarda_estado_parcial_sala(const char* ruta_fichero, size_t num_asientos, int* id_asientos);
int recupera_estado_parcial_sala(const char* ruta_fichero, size_t num_asientos, int* id_asientos);

#endif