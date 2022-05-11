#include "directorios.h"

// Sintaxis : mi_escribir <disco> </ruta_fichero> <texto> <offset>
int main(int argc, char **argv)
{
    // Comprobamos cantidad de argumentos
    if(argc==4){

        // Montar el dispositivo
        bmount(argv[1]);

        char camino[strlen(argv[2])];
        strcpy(camino, argv[2]);

        // Caso en el que no se especifica un fichero
        if(camino[strlen(camino)-1]=='/'){
            printf("ERROR DE RUTA: La ruta no acaba en un fichero");
        }
        // Escribimos el texto
        else{
            char texto[strlen(argv[3])];
            strcpy(texto,argv[3]);
            printf("Longitud del texto: %li", strlen(texto));
            
            printf("Bytes escritos: %i",mi_write(camino, camino, atoi(argv[4]), strlen(argv[3])));
        }
        // Desmontamos el dispositivo
        bumount(argv[1]);




    }else{
        // Mala Sintaxis
        printf("Sintaxis : mi_escribir <disco> </ruta_fichero> <texto> <offset>\n");
	    return -1;
    }



}