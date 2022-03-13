
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
    return 0;
}

/*
La función reservar_bloque ncuentra el primer bloque libre, consultando el MB (primer bit a 0),
lo ocupa y devuelve su posición.
*/
int reservar_bloque()
{

   // Leemos el superbloque 
    struct superbloque SB;
    if (bread(posSB, &SB) == -1)
    {
        perror("Error");
        return -1;
    }

    // Si quedan bloques libres en el dispositivo
    if (SB.cantBloquesLibres>0)
    {

        unsigned char bufferMB[BLOCKSIZE]; // Buffer para leer el MB
        memset(bufferMB, 0, BLOCKSIZE);

        unsigned char bufferAux[BLOCKSIZE]; // Buffer auxiliar
        memset(bufferAux, 255, BLOCKSIZE); 

        // Variable para detectar el primer bit a 0;
        int primerlibre =0;
        
        unsigned int nbloqueabs = SB.posPrimerBloqueMB;

        // Recorremos el MB hasta encontrar localizar el 1er bloque libre iterando con nbloqueabs
        for (nbloqueabs; nbloqueabs<=SB.posUltimoBloqueMB && primerlibre==0; nbloqueabs++){

            // Cargamos el bloque en el bufferMB
            if (bread(nbloqueabs,bufferMB) == -1)
            {
                perror("Error");
                return -1;
            }

            // Si el bloque del bufferMB tiene uno o más bits a 0 --> primerlibre != 0;
            primerlibre = memcmp(bufferMB,bufferAux, BLOCKSIZE);

        }

        unsigned char mascara = 128; // 10000000
        int posbit = 0;

        // Recorremos los bytes del bloque para localizar el byte con el bit a 0
        for (int posbyte =0; posbyte < BLOCKSIZE; posbyte++){

            // Detectamos el byte con el bit a 0
            if(bufferMB[posbyte]<255){

                // Buscamos la posición del bit a 0
                while (bufferMB[posbyte] & mascara) { // operador AND para bits
                bufferMB[posbyte] <<= 1;      // desplazamiento de bits a la izquierda
                posbit++;
                }

                int nbloque = ((nbloqueabs - SB.posPrimerBloqueMB) * BLOCKSIZE + posbyte) * 8 + posbit;

                // Indicamos que el bloque està reservado
                escribir_bit(nbloque, 1);  

                // Decrementamos la cantidad de bloques libres del SB
                SB.cantBloquesLibres = SB.cantBloquesLibres - 1;    

                // Actualizamos el SB
                if (bwrite(posSB, &SB) == -1) {
                    perror("Error");
                }

                // Limpiamos el bloque
                unsigned char bufferVacio[BLOCKSIZE]; 
                memset(bufferVacio,0,BLOCKSIZE);
                if (bwrite(nbloque, &bufferVacio) == -1) {

                return -1;
                }

                // Devolvemos el nº de bloque que hemos reservado
                return nbloque;
            }

        }

    }else{

        // No quedan bloques libres en el dispositivo
        return -1;
    }

    
}

// La función liberar_que libera un bloque determinado por nbloque
int liberar_bloque(unsigned int nbloque)
{

    // Ponemos a 0 el bit del MB correspondiente al bloque nbloque
    escribir_bit(nbloque, 0);

    // Leemos el superbloque 
    struct superbloque SB;
    if (bread(posSB, &SB) == -1)
    {
        perror("Error");
        return -1;
    }

    // Incrementamos la cantidad de bloques libres en el superbloque.
    SB.cantBloquesLibres = SB.cantBloquesLibres+1;

    // Salvamos el SB
    if (bwrite(posSB, &SB) == -1) {
        perror("Error");
    }

    // Devolvemos el nº de bloque liberado, nbloque.
    return nbloque;

}