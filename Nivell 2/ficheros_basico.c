
#include "ficheros_basico.h"


int tamMB(unsigned int nbloques)
{
    int tam = (nbloques / 8) / BLOCKSIZE;
    if ((nbloques / 8) % BLOCKSIZE != 0)
    {
        return (tam + 1);
    }
    return tam;
}

int tamAI(unsigned int ninodos)
{
    int tam = (ninodos * INODOSIZE) / BLOCKSIZE;
    if ((ninodos * INODOSIZE) % BLOCKSIZE != 0)
    {
        return (tam + 1);
    }
    return tam;
}

int initMB()
{
    struct superbloque SB;
    // Declaramos un búfer tan grande como un bloque
    unsigned char *buf = malloc(BLOCKSIZE);
    // Contenido de buffer = todo 0s
    memset(buf, 0, BLOCKSIZE);

    // Leemos el superbloque
    struct superbloque SB;
    if (bread(0, &SB) == -1)
    {
        perror("Error");
        return -1;
    }

    // Ponemos todo el MB a 0s
    for (int i = SB.posPrimerBloqueMB; i < SB.posUltimoBloqueMB; i++)
    {
        if (bwrite(i, buf) == -1)
        {
            perror("Error");
            return -1;
        }
    }

    // DEMANAR QUE HEM DE RETORNAR !!!!!!!!!!!!
    // I SI AIXO ES CORRECTE
    return 0;
}

int initSB(unsigned int nbloques, unsigned int ninodos)
{
    // Inicializamos el superbloque

    struct superbloque SB;
    

    SB.posPrimerBloqueMB = posSB + tamSB;
    SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nbloques) - 1;
    SB.posPrimerBloqueAI = SB.posUltimoBloqueMB + 1;
    SB.posUltimoBloqueAI = SB.posPrimerBloqueAI + tamAI(ninodos) - 1;
    SB.posPrimerBloqueDatos = SB.posUltimoBloqueAI + 1;
    SB.posUltimoBloqueDatos = nbloques - 1;
    SB.posInodoRaiz = 0;
    SB.posPrimerInodoLibre = 0;
    SB.cantBloquesLibres = ninodos;
    SB.totBloques = nbloques;
    SB.totInodos = ninodos;

    if (bwrite(posSB, &SB) == -1)
    {
        perror("Error");
        return -1;
    }
    return 0;
}

// Esta función se encargará de inicializar la lista de inodos libres.
int initAI()
{
    struct superbloque SB;
    // Buffer para ir recorriendo el array de inodos
    struct inodo inodos[BLOCKSIZE / INODOSIZE];

    // Leemos el superbloque para obtener la localización del array de inodos.
    struct superbloque SB;
    if (bread(0, &SB) == -1)
    {
        perror("Error");
        return -1;
    }

    unsigned int contInodos = SB.posPrimerInodoLibre + 1;

    // Para cada bloque del array de inodos.
    for (int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++)
    {


        // Para cada inodo del array de inodos
        for (int j = 0; j < BLOCKSIZE / INODOSIZE; j++)
        {

            inodos[j].tipo = 'l'; // Indicamos que el tipo de inodo es libre ('l')

            // Si no hemos llegado al último inodo.
            if (contInodos < SB.totInodos)
            {
                // Enlazamos el inodo con el siguiente.
                inodos[j].punterosDirectos[0] = contInodos;
                contInodos++;
            }
            else
            {
                inodos[j].punterosDirectos[0] = UINT_MAX;
                break;
            }
        }

        // Escribimos el bloque de inodos i en el dispositivo virtual
        if (bwrite(i, inodos) == -1)
        {
            perror("Error");
            return -1;
        }
    }

    // Tornar 0?
    return 0;
}