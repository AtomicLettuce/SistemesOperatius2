#include "ficheros.h"

int mi_stat_f(unsigned int ninodo, struct STAT *p_stat){

    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) < 0) {
        perror("Error");
        return -1;
    }

    p_stat->tipo = inodo.tipo;
    p_stat->permisos = inodo.permisos;

    p_stat->atime = inodo.atime;
    p_stat->mtime = inodo.mtime;
    p_stat->ctime = inodo.ctime;

    p_stat->nlinks = inodo.nlinks;
    p_stat->tamEnBytesLog = inodo.tamEnBytesLog;
    p_stat->numBloquesOcupados = inodo.numBloque;

    return 0;
}


int mi_chmod_f(unsigned int ninodo, unsigned char permisos){


    struct inodo inodo;

    // Leemos el inodo del correspondiente nº de inodo
    if (leer_inodo(ninodo, &inodo) < 0) {
        perror("Error");
        return -1;
    }

    // Cambiamos los permisos
    inodo.permisos = permisos;

    // Actualizamos ctime
    inodo.ctime  = time(NULL);

    if (escribir_inodo(ninodo, inodo)<0){
        perror("Error");
        return -1;
    }

    return 0;

}