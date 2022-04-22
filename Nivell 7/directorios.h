#include "ficheros_basico.h"
#include "debugging.h"
#define TAMNOMBRE 60

int extraer_camino(const char *camino, char *inicial, char *final, char *tipo);
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos);

struct entrada{
    char nombre[TAMNOMBRE];
    unsigned int ninodo;
};