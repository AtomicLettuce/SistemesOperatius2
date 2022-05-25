#include "directorios.h"

// Sintaxis: ./mi_link disco /ruta_fichero_original /ruta_enlace

int main(int argv, char **argv)
{
    if (argv != 4)
    {
        printf("Sintaxis: ./mi_link disco /ruta_fichero_original /ruta_enlace");
        return -1;
    }
    else
    {
        // Montamos dispositivo
        bmount(argv[1]);

        if (argv[2][strlen(argv[2] - 1)] != '/' && argv[2][strlen(argv[2] - 1)] != '/')
        {
            mi_link(argv[1],argv[2]);
        }
        else
        {
            printf("ESPECIFICA FICHEROS");
        }

        // Desmontamos el dispositivo
        bumount(argv[1]);
        return 0;
    }
}