#include "ficheros_basico.h"

int main(int argc, char **argv)
{
    struct superbloque SB;
    //struct inodo inodos[BLOCKSIZE / INODOSIZE];
    char *camino = argv[1];
    //struct inodo inodo = argv[2];
    long int offsets[] = {9000,209000,30725000,409605000,4800000000};
    if (argc != 3)
    {
        printf("Sintaxis: escribir <nombre_dispositivo><'$(cat fichero)'><diferentes_inodos>\n");
        printf("Offsets: 9000,209000,30725000,409605000,480000000\n");
        printf("Si diferentes_inodos = 0 se reserva un solo inodo para los offets\n");
        return -1;
    }
    if (bmount(camino) == -1)
    {
        return -1;
    }
    reservar_inodo('f',7);
    
    if (bumount() == -1)
    {
        return -1;
    }
    return 0;
}
