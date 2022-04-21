#include "directorios.h"
#include "debugging.h"
#include <string.h>

int extraer_camino(const char *camino, char *inicial, char *final, char *tipo){


    // Nos aseguramos de que empieze correctamente
    if(camino[0]!='/'|| camino==NULL){
        return ERROR;
    }
    // Caso en el que empieza correctamente
    else{
        // Copiamos el contenido del camino en un string auxiliar para poder modificarlo
        char straux[strlen(camino)];
        strcpy(straux,camino);
        
        // Dividimos el string auxiliar en dos tókens divididos por el carácter "/".
        // La primera parte irá al puntero inicial y straux contendrá el resto del string
        strcpy(inicial,strtok(straux,"/"));

        // Comprobamos si se trata de un fichero o de un directorio
        // Caso directorio
        if(camino[strlen(inicial)+1]=='/'){
            // Ponemos el valor correspondiente a tipo
            *tipo='d';
            //Copiamos el resto de la ruta en el puntero final
            strcpy(final,camino+strlen(inicial)+1);

        }
        // Caso fichero
        else{
            // Ponemos el valor correspondiente a tipo
            *tipo='f';
            // Ponemos "" en final pues no le podemos asignar valor NULL pues luego quedaría apuntando a NULL
            strcpy(final,"");
            // Si es un fichero lo ponemos sin el "/" inicial
            strcpy(inicial,camino+1);
        }

    }
}
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada,
                     char reservar, unsigned char permisos){



}