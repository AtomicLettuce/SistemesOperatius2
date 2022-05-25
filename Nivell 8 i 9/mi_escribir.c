#include "directorios.h"

// Sintaxis : mi_escribir <disco> </ruta_fichero> <texto> <offset>
int main(int argc, char **argv)
{
    // Comprobamos cantidad de argumentos
    if (argc == 5)
    {

        // Montar el dispositivo
        bmount(argv[1]);

        int escritos=0;
        char *buffer_texto = argv[3];;
        int longitud=strlen(buffer_texto);
        char *camino = argv[2];
        unsigned int offset = atoi(argv[4]);

        // Caso en el que no se especifica un fichero

        if (argv[2][strlen(argv[2])-1]=='/'){
            fprintf(stderr, "Error: la ruta se corresponde a un directorio");
            exit(-1);
        }
        // Escribimos el texto
        fprintf(stderr, "longitud texto: %d\n", longitud);
        escritos = mi_write(camino,buffer_texto,offset,longitud); 
        fprintf(stderr, "Bytes escritos: %d\n", escritos);
        // Desmontamos el dispositivo
        bumount();

   }else{
        // Mala Sintaxis
        printf("Sintaxis : mi_escribir <disco> </ruta_fichero> <texto> <offset>\n");
	    exit(-1);

    }
  return 0;
}
