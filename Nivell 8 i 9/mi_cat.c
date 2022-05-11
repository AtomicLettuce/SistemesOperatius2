#include "directorios.h"
#define BUFFERSIZE 1420

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
        char camino[strlen(argv[2])];
        strcpy(camino, argv[2]);

        // Caso en el que se trata de un fichero
        if (camino[strlen(camino) - 1] != '/')
        {
            unsigned char buffer_texto[BUFFERSIZE];
            unsigned char buffer_aux[BUFFERSIZE];
            // Posam a 0 totes les posicions del buffer.
            memset(buffer_texto, 0, BUFFERSIZE);
            memset(buffer_aux, 0, BUFFERSIZE);
            unsigned int leidos;
            unsigned int total_leidos = 0;
            unsigned int offset = 0;
            char string[128];
            leidos = mi_read(argv[2], buffer_texto, offset, BUFFERSIZE);
            offset += BUFFERSIZE;

            while (leidos > 0)
            {
                write(1, buffer_texto, leidos);
                memset(buffer_texto, 0, BUFFERSIZE);
                total_leidos += leidos;
                leidos = mi_read(argv[2], buffer_texto, offset, BUFFERSIZE);
                offset += BUFFERSIZE;
            }
            sprintf(string, "\n");
            write(2, string, strlen(string));
            sprintf(string, "Total_leidos: %d\n", total_leidos);
            write(2, string, strlen(string));
        }
        // Caso en el que no es un fichero
        else
        {
            printf("ERROR: No se trata de un fichero\n");
        }
    }
}