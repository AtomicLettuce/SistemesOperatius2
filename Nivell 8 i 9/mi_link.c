#include "directorios.h"

// Sintaxis: ./mi_link disco /ruta_fichero_original /ruta_enlace

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        printf("Sintaxis: ./mi_link disco /ruta_fichero_original /ruta_enlace");
        return -1;
    }
    else
    {
        // Montamos dispositivo
        bmount(argv[1]);
        

        if(strcmp(&argv[2][strlen(argv[2])-1],"/") == 0 || strcmp(&argv[3][strlen(argv[3])-1],"/") == 0)
        {
            printf("ERROR: ESPECIFICA FICHEROS\n");
        }
        else
        {
            mi_link(argv[2],argv[3]);
        }

        // Desmontamos el dispositivo
        bumount();
        return 0;
    }
}