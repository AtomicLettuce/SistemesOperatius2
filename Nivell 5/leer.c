#include "ficheros_basico.h"

int main(int argc, char **argv)
{
    //struct superbloque SB;
    //struct inodo inodos[BLOCKSIZE / INODOSIZE];
    char *camino = argv[1];
    //struct inodo inodo = argv[2];
    if (argc != 3)
    {
        printf("Sintaxis: leer <nombre_dispositivo><ninodos>\n");
    }
    if (bmount(camino) == -1)
    {
        return -1;
    }

    if (bumount() == -1)
    {
        return -1;
    }
    return 0;
}