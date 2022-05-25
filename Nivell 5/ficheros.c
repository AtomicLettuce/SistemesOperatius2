#include "ficheros.h"
#define ERROR -1

int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes)
{
    // Variable de retorno
    int nbytesEscritos = 0;
    // Este búfer lo usaremos más adelante
    char buf_bloque[BLOCKSIZE];

    // Leemos el inodo apuntado por ninodo
    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) == ERROR)
    {
        perror("ERROR: ");
        return ERROR;
    }

    // Comprobamos si tiene permisos de escritura
    if ((inodo.permisos & 2) != 2)
    {
        fprintf(stderr,"No hay permisos de lectura\n");
        return ERROR;
    }
    else
    {
        // Averiguamos el rango de bloques lógicos que habrá que escribir
        int primerBL = offset / BLOCKSIZE;
        int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;

        // Averiguamos los depslazamientos dentro del mismo bloque. Es decir, un offset puede ser (por ejemplo) 300. Por tanto
        // habrá que empezar a escribir a partir del byte 300 del primer BL.
        // Lo mismo pasa para el último BL que escribamos. No necesariamente habrá que escribir un bloque entero
        int desp1 = offset % BLOCKSIZE;
        int desp2 = (offset + nbytes - 1) % BLOCKSIZE;

        // Caso en el que primerBL==ultimoBL
        if (primerBL == ultimoBL)
        {
            // Obtenemos el num de bloque físico
            int nbfisico = traducir_bloque_inodo(ninodo, primerBL, 1);

            // Leemos el bloque
            if (bread(nbfisico, buf_bloque) == ERROR)
            {
                perror("ERROR: ");
                return ERROR;
            }

            // Copiamos el contenido que había que escribir
            memcpy(buf_bloque + desp1, buf_original, nbytes);

            // Lo escribimos en el dispositivo
            if (bwrite(nbfisico, buf_bloque) == ERROR)
            {
                perror("ERROR: ");
                return ERROR;
            }
            nbytesEscritos = nbytes;
        }
        // Caso en el que la operación de escritura afecta a más de un bloque
        else
        {
            // Leemos el primer bloque lógico
            int nbfisico = traducir_bloque_inodo(ninodo, primerBL, 1);
            if (bread(nbfisico, buf_bloque) == ERROR)
            {
                perror("ERROR: ");
                return ERROR;
            }
            // Copiamos lo que faltaba del primer BL
            memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE - desp1);
            if (bwrite(nbfisico, buf_bloque) == ERROR)
            {
                perror("ERROR: ");
                return ERROR;
            }
            nbytesEscritos = BLOCKSIZE - desp1;

            // Escribimos los bloques intermedios
            for (int i = primerBL + 1; i < ultimoBL; i++)
            {
                // Obtenemos nbfisico apuntado por en i-ésimo bloque lógico
                nbfisico = traducir_bloque_inodo(ninodo, i, 1);

                bwrite(nbfisico, buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE);
                // Actualizamos nbytesEscritos
                nbytesEscritos += BLOCKSIZE;
            }

            // Último bloque lógico
            nbfisico = traducir_bloque_inodo(ninodo, ultimoBL, 1);
            // Leemos lo que había antes
            if (bread(nbfisico, buf_bloque) == ERROR)
            {
                perror("ERROR: ");
                return ERROR;
            }
            // Sobreescribimos la parte que queremos
            memcpy(buf_bloque, buf_original + (nbytes - desp2 - 1), desp2 + 1);

            // Lo escribimos en el dispositivo
            if (bwrite(nbfisico, &buf_bloque) == ERROR)
            {
                perror("ERROR: ");
                return ERROR;
            }
            nbytesEscritos += desp2 + 1;
        }
    }
    // Actualizamos los atributos del inodo
    if (leer_inodo(ninodo, &inodo) == ERROR)
    {
        perror("ERROR: ");
        return ERROR;
    }
    // Si hemos escrito al menos 1 byte, actualizamos el mtime
    if (nbytesEscritos > 0)
    {
        inodo.mtime = time(NULL);
    }

    // Actualizamos tamEnBytesLog y ctime si hemos escrito más allá del final de fichero
    if (offset + nbytesEscritos > inodo.tamEnBytesLog)
    {
        inodo.tamEnBytesLog = offset + nbytesEscritos;
        inodo.ctime = time(NULL);
    }

    // Escribimos el inodo ahora actualizado
    if (escribir_inodo(ninodo, &inodo) == ERROR)
    {
        perror("ERROR: ");
        return ERROR;
    }

    // Devolvemos la cantidad de bytes que hemos escrito
    return nbytesEscritos;
}

int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes)
{
    struct inodo inodo;
    int nbytesLeidos = 0;
    char buf_bloque[BLOCKSIZE];
    int nbfisico;

    // Leemos el inodo
    if (leer_inodo(ninodo, &inodo) == ERROR)
    {
        perror("ERROR: ");
        return ERROR;
    }
    // Puesto que hemos accedido al inodo, actualizamos el atime
    inodo.atime = time(NULL);
    if ((escribir_inodo(ninodo, &inodo)) == ERROR)
    {
        perror("ERROR: ");
        return ERROR;
    }

    // Comprobamos que tenga permisos de lectura
    if ((inodo.permisos & 4) != 4)
    {
        fprintf(stderr,"No hay permisos de lectura\n");
        return nbytesLeidos;
    }
    else
    {
        // Comprobamos que no queramos leer más allá del EOF
        if (offset >= inodo.tamEnBytesLog)
        {
            return nbytesLeidos;
        }
        // Comprobamos  si pretende leer más allá del EOF
	else if ((offset + nbytes) >= inodo.tamEnBytesLog)
        {
            nbytes = inodo.tamEnBytesLog - offset;
        }

        // Limpiamos el buf_original
        memset(buf_original, 0, nbytes);

        // Averiguamos el rango de bloques lógicos que habrá que leer
        int primerBL = offset / BLOCKSIZE;
        int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;

        // Averiguamos los depslazamientos dentro del mismo bloque. Es decir, un offset puede ser (por ejemplo) 300. Por tanto
        // habrá que empezar a leere a partir del byte 300 del primer BL.
        // Lo mismo pasa para el último BL que escribamos. No necesariamente habrá que leer un bloque entero
        int desp1 = offset % BLOCKSIZE;
        int desp2 = (offset + nbytes - 1) % BLOCKSIZE;

        // Caso en el que solo tenemos que leer un bloque o parte de un bloque
        if (primerBL == ultimoBL)
        {
            // Obtenemos el num de bloque físico
            nbfisico = traducir_bloque_inodo(ninodo, primerBL, 0);

            // Nos aseguramos de que el inodo tenga un BF para dicho BL
            if (nbfisico != ERROR)
            {
                // Leemos el contenido
                if (bread(nbfisico, buf_bloque) == ERROR)
                {
                    perror("ERROR: ");
                    return ERROR;
                }
                // Copiamos lo que nos interesa
                memcpy(buf_original, buf_bloque + desp1, nbytes);
            }
            nbytesLeidos = nbytes;
            return nbytesLeidos;
        }
        // Caso en el que toca leer más de un bloque
        else
        {
            nbfisico = traducir_bloque_inodo(ninodo, primerBL, 0);

            // Comprobamos si existe un BF para dicho BL
            if (nbfisico != ERROR)
            {
                // Leemos el bloque
                if (bread(nbfisico, buf_bloque) == ERROR)
                {
                    perror("ERROR: ");
                    return ERROR;
                }
                // Copiamos lo que nos interesa
                memcpy(buf_original, buf_bloque + desp1, BLOCKSIZE - desp1);
            }
            nbytesLeidos += BLOCKSIZE - desp1;

            // Para los bloques intermedios entre primerBL y ultimoBL
            for (int i = primerBL + 1; i < ultimoBL; i++)
            {
                // Obtenemos direccion de nbfisco para dicho BL
                nbfisico = traducir_bloque_inodo(ninodo, i, 0);

                // Comprobamos si tiene asignado dicho BF
                if (nbfisico != ERROR)
                {
                    // Leemos dicho bloque
                    if (bread(nbfisico, buf_bloque) == ERROR)
                    {
                        perror("ERROR: ");
                        return ERROR;
                    }
                    memcpy(buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE, buf_bloque, BLOCKSIZE);
                }
                nbytesLeidos += BLOCKSIZE;
            }
            // Para el último bloque o fragmento de bloque que tengamos que leer
            nbfisico = traducir_bloque_inodo(ninodo, ultimoBL, 0);

            // Comprobamos si existe un BF asignado para tal BL
            if (nbfisico != ERROR)
            {
                // Leemos el bloque
                if (bread(nbfisico, buf_bloque) == ERROR)
                {
                    perror("ERROR: ");
                    return ERROR;
                }

                memcpy(buf_original + (nbytes - desp2 - 1), buf_bloque, desp2 + 1);
            }
            // Actualizamos el número de bytes leídos
            nbytesLeidos += desp2 + 1;
            return nbytesLeidos;
        }
    }
    // Por si acaso
    return ERROR;
}

int mi_stat_f(unsigned int ninodo, struct STAT *p_stat)
{

    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) < 0)
    {
        perror("ERROR: ");
        return -1;
    }

    p_stat->tipo = inodo.tipo;
    p_stat->permisos = inodo.permisos;

    p_stat->atime = inodo.atime;
    p_stat->mtime = inodo.mtime;
    p_stat->ctime = inodo.ctime;

    p_stat->nlinks = inodo.nlinks;
    p_stat->tamEnBytesLog = inodo.tamEnBytesLog;
    p_stat->numBloquesOcupados = inodo.numBloquesOcupados;

    return 0;
}

int mi_chmod_f(unsigned int ninodo, unsigned char permisos)
{

    struct inodo inodo;

    // Leemos el inodo del correspondiente nº de inodo
    if (leer_inodo(ninodo, &inodo) < 0)
    {
        perror("ERROR: ");
        return -1;
    }

    // Cambiamos los permisos
    inodo.permisos = permisos;

    // Actualizamos ctime
    inodo.ctime = time(NULL);

    if (escribir_inodo(ninodo, &inodo) < 0)
    {
        perror("ERROR: ");
        return -1;
    }

    return 0;
}

