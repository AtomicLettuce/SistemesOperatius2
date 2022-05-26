#include "directorios.h"
// Sintaxis: :/mi_rm disco /ruta
int main(int argc, char **argv)
{

    // Comprobamos sintaxis
    if (argc == 3)
    {
        // Montar dispositivo
        bmount(argv[1]);        

        if (strcmp(argv[2],"/")==0)
        {
            printf("ERROR: No se puede eliminar el inodo raiz\n");
            return EXIT_FAILURE;
        }else

        // Llamada a unlink
        mi_unlink(argv[2]);

        // Desmontamos el dispositivo
        bumount();
        return EXIT_SUCCESS;
    }
    else
    {
        printf("Sintaxis: :/mi_rm disco /ruta");
        return -1;
    }
}