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

        char *ruta1 = argv[2];
        char *ruta2 =argv[3];
        

        if(strcmp(&ruta1[strlen(ruta1)-1],"/") == 0 || strcmp(&ruta2[strlen(ruta2)-1],"/") == 0)
        {
            printf("ERROR: ESPECIFICA FICHEROS\n");
        }
        else
        {
            mi_link(argv[1],argv[2]);
        }

        // Desmontamos el dispositivo
        bumount(argv[1]);
        return 0;
    }
}