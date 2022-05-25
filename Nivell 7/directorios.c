#include "directorios.h"
#include "debugging.h"
#include <string.h>

int extraer_camino(const char *camino, char *inicial, char *final, char *tipo)
{

    // Nos aseguramos de que empieze correctamente
    if (camino[0] != '/' || camino == NULL)
    {
        return ERROR;
    }
    // Caso en el que empieza correctamente
    else
    {
        // Copiamos el contenido del camino en un string auxiliar para poder modificarlo
        char straux[strlen(camino)];
        strcpy(straux, camino);

        // Dividimos el string auxiliar en dos tókens divididos por el carácter "/".
        // La primera parte irá al puntero inicial y straux contendrá el resto del string
        strcpy(inicial, strtok(straux, "/"));

        // Comprobamos si se trata de un fichero o de un directorio
        // Caso directorio
        if (camino[strlen(inicial) + 1] == '/')
        {
            // Ponemos el valor correspondiente a tipo
            *tipo = 'd';
            // Copiamos el resto de la ruta en el puntero final
            strcpy(final, camino + strlen(inicial) + 1);
        }
        // Caso fichero
        else
        {
            // Ponemos el valor correspondiente a tipo
            *tipo = 'f';
            // Ponemos "" en final pues no le podemos asignar valor NULL pues luego quedaría apuntando a NULL
            strcpy(final, "");
            // Si es un fichero lo ponemos sin el "/" inicial
            strcpy(inicial, camino + 1);
        }
    }
    return 0;
}
<<<<<<< HEAD
=======


>>>>>>> 74381f3f199b5c9b7a92837d77ddf84c0562cf2f
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada,
                   char reservar, unsigned char permisos)
{
    struct superbloque SB;
    struct entrada entrada;
    struct inodo inodo_dir;
    char inicial[sizeof(entrada.nombre)];
    char final[strlen(camino_parcial)];
    char tipo;
    int cant_entradas_inodo;
    int num_entrada_inodo;
    int reservado;

    // Leemos superbloque
    if (bread(0, &SB) == ERROR)
    {
        perror("ERROR");
        return ERROR;
    }

    if (strcmp(camino_parcial, "/") == 0)
    {
        *p_inodo = SB.posInodoRaiz;
        *p_entrada = 0;
        return 0;
    }

    if (extraer_camino(camino_parcial, inicial, final, &tipo) == ERROR)
    {
        return ERROR_CAMINO_INCORRECTO;
    }
#if DEBUGN7
    fprintf(stdout,"[buscar_entrada()-> inicial:%s final:%s,reservar: %i]\n", inicial, final, reservar);
#endif
    // buscamos la entrada cuyo nombre se encuentra en inicial
    if (leer_inodo(*p_inodo_dir, &inodo_dir) == ERROR)
    {
        perror("ERROR: ");
        return ERROR;
    }
    // Comprobamos si tiene permisos de lectura
    if ((inodo_dir.permisos & 4) != 4)
    {
        return ERROR_PERMISO_LECTURA;
    }
    memset(&entrada, 0, sizeof(entrada));
    // Calculamos cuantas entradas tiene el inodo_dir
    cant_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof(entrada);
    // Num de entrada inicial
    num_entrada_inodo = 0;
    // Variable que usaremos para iterar
    int offset = 0;

    if (cant_entradas_inodo > 0)
    {
        if (mi_read_f(*p_inodo_dir, &entrada, offset, sizeof(entrada)) == ERROR)
        {
            perror("ERROR: ");
            return ERROR;
        }
        while ((num_entrada_inodo < cant_entradas_inodo) && (strcmp(inicial, entrada.nombre) != 0))
        {
            num_entrada_inodo++;
            offset +=sizeof(entrada);
            memset(&entrada, 0, sizeof(entrada));
            if (mi_read_f(*p_inodo_dir, &entrada, offset, sizeof(entrada)) == ERROR)
                {
                    perror("ERROR: ");
                    return ERROR;
                }
            }
        }
    if (strcmp(inicial, entrada.nombre) != 0)
    {
        switch (reservar)
        {
        case 0:
            return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
            break;

        case 1:
            if (inodo_dir.tipo == 'f')
            {
                return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
            }
            if ((inodo_dir.permisos & 2) != 2)
            {
                return ERROR_PERMISO_ESCRITURA;
            }
            else
            {
                strcpy(entrada.nombre, inicial);
                if (tipo == 'd')
                {
                    if (strcmp(final, "/") == 0)
                    {
                        reservado = reservar_inodo('d', permisos);
                        if (reservado == ERROR)
                        {
                            perror("ERROR: ");
                            return ERROR;
                        }
#if DEBUGN7
                        fprintf(stdout, "[buscar_entrada()-> reservado inodo %i tipo %c con permisos %i para %s]\n", reservado, tipo, permisos, inicial);
#endif
                        entrada.ninodo = reservado;
                    }
                    else
                    {
                        return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                    }
                }
                else
                {
                    reservado = reservar_inodo('f', permisos);
                    if (reservado == ERROR)
                    {
                        perror("ERROR: ");
                        return ERROR;
                    }
#if DEBUGN7
                    fprintf(stdout,"[buscar_entrada()-> reservado inodo %i tipo %c con permisos %i para %s]\n", reservado, tipo, permisos, inicial);
#endif
                    entrada.ninodo = reservado;
                }
                if (mi_write_f(*p_inodo_dir, &entrada, offset, sizeof(entrada)) == ERROR)
                {
                    if (entrada.ninodo != -1)
                    {
                        if (liberar_inodo(entrada.ninodo) == ERROR)
                        {
                            perror("ERROR: ");
                            return ERROR;
                        }
                    }
                    return EXIT_FAILURE;
                }
#if DEBUGN7
                fprintf(stdout, "[buscar_entrada()-> creada entrada %s, %i]\n", entrada.nombre, entrada.ninodo);
#endif
            }
            //break;
        }
    }

    if ((strcmp(final, "/") == 0) || (strcmp(final, "") == 0))
    {
        if ((num_entrada_inodo < cant_entradas_inodo) && (reservar == 1))
        {
            return ERROR_ENTRADA_YA_EXISTENTE;
        }
        *p_inodo = entrada.ninodo;
        *p_entrada = num_entrada_inodo;
        return EXIT_SUCCESS;
    }
    else
    {
        *p_inodo_dir = entrada.ninodo;
        return (buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos));
    }
    return EXIT_SUCCESS;
}

void mostrar_error_buscar_entrada(int error)
{
    // fprintf(stderr, "Error: %d\n", error);
    switch (error)
    {
    case -1:
        fprintf(stderr, "Error: Camino incorrecto.\n");
        break;
    case -2:
        fprintf(stderr, "Error: Permiso denegado de lectura.\n");
        break;
    case -3:
        fprintf(stderr, "Error: No existe el archivo o el directorio.\n");
        break;
    case -4:
        fprintf(stderr, "Error: No existe algún directorio intermedio.\n");
        break;
    case -5:
        fprintf(stderr, "Error: Permiso denegado de escritura.\n");
        break;
    case -6:
        fprintf(stderr, "Error: El archivo ya existe.\n");
        break;
    case -7:
        fprintf(stderr, "Error: No es un directorio.\n");
        break;
    }
}