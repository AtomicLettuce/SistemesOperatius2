#include "ficheros.h"
#define TAMNOMBRE 60
#define TAMENTRADA 60 + 4
#define PROFUNDIDAD 32

int extraer_camino(const char *camino, char *inicial, char *final, char *tipo);
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos);
void mostrar_error_buscar_entrada(int error);
int mi_creat(const char *camino, unsigned char permisos);
int mi_dir(const char *camino, char *buffer);
int mi_chmod(const char *camino, unsigned char permisos);
int mi_stat(const char *camino, struct STAT *p_stat);
int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes);

struct entrada
{
    char nombre[TAMNOMBRE];
    unsigned int ninodo;
};

struct UltimaEntrada{
  char camino [TAMNOMBRE*PROFUNDIDAD];
  int p_inodo;
};