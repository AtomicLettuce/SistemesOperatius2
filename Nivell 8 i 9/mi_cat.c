#include "directorios.h"

// Sintaxis Sintaxis: ./mi_cat <disco> </ruta_fichero>
int main(int argc, char **argv)
{

    // Comprobamos sintaxis
    if (argc != 3)
    {
        printf("Sintaxis: mi_cat <disco> </ruta_fichero>\n");
        return -1;
    }
    else
    {
        // Montamos el dispositivo
        bmount(argv[1]);

        // comprobar que la ruta se corresponda a un fichero

        // Caso en el que se trata de un fichero
        if (argv[2][strlen(argv[2]) - 1] != '/')
        {
            int tambuffer = BLOCKSIZE * 4;
            int offset = 0;
            char buffer[tambuffer];
            unsigned int nbytesleidos = 0;
            unsigned int leidos;
            memset(buffer, 0, tambuffer);

            while ((leidos = mi_read(argv[2], buffer, offset, tambuffer)) > 0)
            {
                fwrite(buffer, sizeof(char), leidos, stdout);
                nbytesleidos += leidos;
                offset += leidos; 
                memset(buffer, 0, tambuffer);
            }
            if (nbytesleidos < 0)
            {
                nbytesleidos = 0;
            }
            printf("\nTotal leidos: %d\n", nbytesleidos);
            bumount();
            return 0;
        }
        // Caso en el que no es un fichero
        else
        {
            printf("ERROR: No se trata de un fichero\n");
        }
    }
}