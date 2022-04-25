#include "debugging.h"
#include "ficheros_basico.h"
#define ERROR -1

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

    // Leemos el superbloque para obtener la localización del array de inodos.
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
    // Ponemos a 1 los bits pertenecientes a los metadatos (AI, SB y MB)
    actualizarBitsMetadatosMB();

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
    SB.cantBloquesLibres = nbloques;
    SB.cantInodosLibres = ninodos;
    SB.totBloques = nbloques;
    SB.totInodos = ninodos;

    if (bwrite(posSB, &SB) == ERROR)
    {
        perror("Error");
        return ERROR;
    }
    return 0;
}

int actualizarBitsMetadatosMB()
{
    struct superbloque SB;
    if (bread(posSB, &SB) == ERROR)
    {
        perror("ERROR: ");
        return ERROR;
    }

    // Número total de bloques que ocupan los metadatos
    int nBloquesTotalMetaDatos = tamAI(SB.totInodos) + tamMB(SB.totBloques) + tamSB;

    // Número total de bloques ENTEROS que hay que poner a 1
    int nBloquesMD = (nBloquesTotalMetaDatos / 8) / BLOCKSIZE;

    // Búfer que usaremos en seguida para escribir
    unsigned char bufferMB[BLOCKSIZE];

    // Escribimos los en el MB bloques enteros que correspondan a los metadatos
    if (nBloquesMD > 0)
    {
        memset(bufferMB, 255, sizeof(bufferMB));
        for (int i = SB.posPrimerBloqueMB; i < (SB.posPrimerBloqueMB + nBloquesMD); i++)
        {
            if (bwrite(i, bufferMB) == ERROR)
            {
                perror("ERROR: ");
                return ERROR;
            }
        }
    }
    // Por ahora hemos escrito todos los bloques enteros que deben estar a 1, falta poner a 1 los que han caído
    // entre y entre

    // búfer auxiliar
    unsigned char bufferAux[BLOCKSIZE];
    // Total de bytes ENTEROS que hay que poner a 1
    int nBytesMD = (nBloquesTotalMetaDatos / 8) % 1024;

    // Total de bits que hay que poner a 1 (será un número del 0-7)
    int nBitsMD = nBloquesTotalMetaDatos % 8;

    // Ponemos a 1 los bytes
    for (int i = 0; i < nBytesMD; i++)
    {
        bufferAux[i] = 255;
    }
    // Ponemos a 1 los bits
    if (nBitsMD != 0)
    {
        // Dependiendo de cuántos bits haya que poner a 1 se seleccionará una máscara u otra
        // 128 = 1000000
        // 192 = 1100000
        // 224 = 1110000
        // ...
        // 254 = 1111110

        // Nótese que no se tiene en cuenta 225 porque entonces tendríamos que poner 8 bits a 1
        // que es lo mismo que poner un byte a 1.
        unsigned char mascaras[] = {128, 192, 224, 240, 248, 252, 254};
        bufferAux[nBytesMD] = mascaras[nBitsMD - 1];
    }
    // Ponemos a 0 todos los otros bits
    for (int i = nBytesMD + 1; i < BLOCKSIZE; i++)
    {
        bufferAux[i] = 0;
    }
    // Escribimos este último bloque en el MB
    if (bwrite(SB.posPrimerBloqueMB + nBloquesMD, bufferAux) == ERROR)
    {
        perror("ERROR: ");
        return ERROR;
    }

    // Actualizamos SB
    SB.cantBloquesLibres = SB.cantBloquesLibres - nBloquesTotalMetaDatos;
    if (bwrite(0, &SB) == ERROR)
    {
        perror("ERROR: ");
        return ERROR;
    }

    return 0;
}

// Esta función se encargará de inicializar la lista de inodos libres.
int initAI()
{
    // Buffer para ir recorriendo el array de inodos
    struct inodo inodos[BLOCKSIZE / INODOSIZE];

    // Leemos el superbloque para obtener la localización del array de inodos.
    struct superbloque SB;
    if (bread(0, &SB) == ERROR)
    {
        perror("Error");
        return ERROR;
    }

    unsigned int contInodos = SB.posPrimerInodoLibre + 1;

    // Para cada bloque del array de inodos.
    for (int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++)
    {

        // Leemos el bloque de inodos
        // És necessari llegir es bloque de inodos?
        if (bread(i, inodos) == ERROR)
        {

            perror("Error");
            return ERROR;
        }

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
        if (bwrite(i, inodos) == ERROR)
        {
            perror("Error");
            return ERROR;
        }
    }
    return 0;
}

int escribir_bit(unsigned int nbloque, unsigned int bit)
{
    // Leemos el superbloque y nos lo guardamos paro usarlo más tarde
    struct superbloque SB;
    if (bread(0, &SB) == ERROR)
    {
        perror("Error");
        return ERROR;
    }

    int posbyte = nbloque / 8;
    int posbit = nbloque % 8;

    // Nbloque relativo dentro del mapa de bits
    int nbloqueMB = posbyte / BLOCKSIZE;

    // nbloque real dentro del disco
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;

    // Leemos el bloque
    unsigned char bufferMB[BLOCKSIZE];
    if (bread(nbloqueabs, bufferMB) == ERROR)
    {
        perror("Error");
        return ERROR;
    }

    // Calculamos el offset con respecto al bloque que hemos leído
    posbyte = posbyte % BLOCKSIZE;

    // Declaramos la máscara
    unsigned char mascara = 128; // 10000000
    // Desplazamos para que el 1 esté situado en la posición posbit-ésima
    mascara >>= posbit;

    // Escribimos el bit
    // Caso 1
    if (bit == 1)
    {
        bufferMB[posbyte] |= mascara;
    }
    // Caso 0
    else if (bit == 0)
    {
        bufferMB[posbyte] &= ~mascara;
    }
    // Caso Error
    else
    {
        perror("ERROR: ");
        return ERROR;
    }

    if (bwrite(nbloqueabs, bufferMB) == ERROR)
    {
        perror("ERROR: ");
        return ERROR;
    }
    return 0;
}

char leer_bit(unsigned int nbloque)
{
    // Leemos el superbloque y nos lo guardamos paro usarlo más tarde
    struct superbloque SB;
    if (bread(0, &SB) == ERROR)
    {
        perror("Error");
        return ERROR;
    }

    int posbyte = nbloque / 8;
    int posbit = nbloque % 8;

    // Nbloque relativo dentro del mapa de bits
    int nbloqueMB = posbyte / BLOCKSIZE;

    // nbloque real dentro del disco
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;

    // Leemos el bloque
    unsigned char bufferMB[BLOCKSIZE];
    if (bread(nbloqueabs, bufferMB) == ERROR)
    {
        perror("Error");
        return ERROR;
    }
#if DEBUGN3
    printf("[leer_bit(%i)→ posbyte:%i, posbit:%i, nbloqueMB:%i, nbloqueabs:%i)]\n", nbloque, posbyte, posbit, nbloqueMB, nbloqueabs);
#endif
    // Calculamos el offset con respecto al bloque que hemos leído
    posbyte = posbyte % BLOCKSIZE;

    // Declaramos la máscara
    unsigned char mascara = 128; // 10000000

    // Movemos el 1 de la máscara para que esté encima del mismo bit que queremos leer
    mascara >>= posbit;
    // Evaluamos el valor del bit
    mascara &= bufferMB[posbyte];
    // Desplazamos para obtener resultado
    mascara >>= (7 - posbit);

// Print para el DEBUGGING. Perteneciente al debugging del nivel 3
#if DEBUGN3
    printf("leer_bit(%i) = %i\n", nbloque, mascara);
#endif

    // Devolvemos el resultado
    return mascara;
}

/*
La función reservar_bloque ncuentra el primer bloque libre, consultando el MB (primer bit a 0),
lo ocupa y devuelve su posición.
*/
int reservar_bloque()
{
    // Obtenemos el superbloque
    struct superbloque SB;
    if (bread(posSB, &SB) == ERROR)
    {
        perror("ERROR: ");
        return ERROR;
    }
    // CASO EN EL QUE QUEDAN BLOQUES LIBRES
    if (SB.cantBloquesLibres > 0)
    {
        // Búferes que usaremos para comprobar cuál es el primer bloque libre
        unsigned char bufferMB[BLOCKSIZE];
        unsigned char bufferAux[BLOCKSIZE];
        memset(bufferAux, 255, sizeof(bufferAux));

        // Comprobamos todos los bloques pertenecientes al MB
        int posBloqueMB = SB.posPrimerBloqueMB;
        for (;; posBloqueMB++)
        {
            // Leemos un bloque del MB
            if (bread(posBloqueMB, bufferMB) == ERROR)
            {
                perror("ERROR: ");
                return ERROR;
            }
            // Comprobamos si hay algun bit a 0
            if (memcmp(bufferMB, bufferAux, sizeof(bufferAux)) < 0)
            {
                // Si ya hemos encontrado el primer bit a 0, forzamos la salida del bucle for
                break;
            }
        }

        // Busamos el primer byte que tenga un 0
        int posbyte = 0;
        for (;; posbyte++)
        {
            if (bufferMB[posbyte] < 255)
            {
                // Si ya lo hemos encontrado, fuerza la salida del bucle for
                break;
            }
        }

        // Ahora ya tenemos el byte que tiene un 0 en alguna de sus posiciones. Falta saber en cuál de ellas es
        unsigned char mascara = 128;
        int posbit = 0;

        // Encontramos en cuál de las posiciones del byte está un 0
        while (bufferMB[posbyte] & mascara)
        {
            bufferMB[posbyte] <<= 1;

            posbit++;
        }
        // Ya sabemos cuál es el primer bit a 0

        // Obtenemos el número de bloque que representa este bit
        unsigned int nbloque = ((posBloqueMB - SB.posPrimerBloqueMB) * BLOCKSIZE + posbyte) * 8 + posbit;

        // Lo marcamos como reservado
        escribir_bit(nbloque, 1);

        // Actualizamos el SB
        SB.cantBloquesLibres--;
        if (bwrite(posSB, &SB) == ERROR)
        {
            perror("ERROR: ");
            return ERROR;
        }

        // Limpiamos el bloque
        unsigned char bufBloque[BLOCKSIZE];
        memset(bufBloque, 0, BLOCKSIZE);
        if (bwrite(nbloque, &bufBloque) == ERROR)
        {
            perror("ERROR: ");
            return ERROR;
        }
        return nbloque;
    }

    // CASO EN EL QUE NO QUEDAN BLOQUES LIBRES
    else
    {
        return ERROR;
    }
}
// La función liberar_que libera un bloque determinado por nbloque. uep
int liberar_bloque(unsigned int nbloque)
{

    // Ponemos a 0 el bit del MB correspondiente al bloque nbloque
    escribir_bit(nbloque, 0);

    // Leemos el superbloque
    struct superbloque SB;
    if (bread(posSB, &SB) == ERROR)
    {
        perror("Error");
        return ERROR;
    }

    // Incrementamos la cantidad de bloques libres en el superbloque.
    SB.cantBloquesLibres = SB.cantBloquesLibres + 1;

    // Salvamos el SB
    if (bwrite(posSB, &SB) == ERROR)
    {
        perror("Error");
        return ERROR;
    }

    // Devolvemos el nº de bloque liberado, nbloque.
    return nbloque;
}
int escribir_inodo(unsigned int ninodo, struct inodo *inodo)
{
    // Leemos el sjuperbloque
    struct superbloque SB;
    if (bread(0, &SB) == ERROR)
    {
        perror("Error");
        return ERROR;
    }
    // Calculamos el numero de bloque absoluto que le corresponde al inodo que queremos escribir
    unsigned int bloqueabs = ((ninodo * INODOSIZE) / BLOCKSIZE) + SB.posPrimerBloqueAI;

    struct inodo inodos[BLOCKSIZE / INODOSIZE];

    // Leemos el bloque en el que está el inodo
    if (bread(bloqueabs, inodos) == ERROR)
    {
        perror("Error");
        return ERROR;
    }
    // Sustituimos el inodo antiguo por el que queremos escribir
    inodos[ninodo % (BLOCKSIZE / INODOSIZE)] = *inodo;
    // Los escribimos en el dispositivo
    if (bwrite(bloqueabs, &inodos) == ERROR)
    {
        perror("Error: ");
        return ERROR;
    }
    return 0;
}

int leer_inodo(unsigned int ninodo, struct inodo *inodo)
{
    struct superbloque SB;
    if (bread(0, &SB) == ERROR)
    {
        perror("Error");
        return ERROR;
    }

    unsigned int numBloque = ((ninodo * INODOSIZE) / BLOCKSIZE) + SB.posPrimerBloqueAI;

    struct inodo inodos[BLOCKSIZE / INODOSIZE];

    if (bread(numBloque, inodos) == ERROR)
    {
        // Error en la lectura.
        perror("ERROR: ");
        return ERROR;
    }
    // Ponemos el inodo solicitado en el puntero pasado por parámetro
    *inodo = inodos[ninodo % (BLOCKSIZE / INODOSIZE)];
    return 0;
}
int reservar_inodo(unsigned char tipo, unsigned char permisos)
{
    // Leemos el superbloque
    struct superbloque SB;
    if (bread(0, &SB) == ERROR)
    {
        perror("Error");
        return ERROR;
    }
    if (SB.cantInodosLibres > 0)
    {
        // Nos guardamos la posición del inodo que reservamos (tendrá su utilidad más adelante)
        int posInodoReservado = SB.posPrimerInodoLibre;

        // Leemos el primer inodo libre
        struct inodo *reservado = malloc(sizeof(struct inodo));
        if (leer_inodo(posInodoReservado, reservado) == ERROR)
        {
            perror("ERROR: ");
        }
        // Modificamos los datos del inodo
        reservado->tipo = tipo;
        reservado->permisos = permisos;
        reservado->nlinks = 1;
        reservado->tamEnBytesLog = 0;
        reservado->atime = time(NULL);
        reservado->ctime = time(NULL);
        reservado->mtime = time(NULL);
        reservado->numBloquesOcupados = 0;

        // Actualizamos el SB
        SB.posPrimerInodoLibre = reservado->punterosDirectos[0]; // Actualizamos para que el SB ahora apunte al NUEVO primer nodo libre
        SB.cantInodosLibres--;                                   // Puesto que reservamos uno

        // *****Lo escribiremos en el disco un poco más adelante *****

        // Ponemos a 0 todos los punteros
        for (int i = 0; i < 12; i++)
        {
            reservado->punterosDirectos[i] = 0;
        }
        for (int i = 0; i < 3; i++)
        {
            reservado->punterosIndirectos[i] = 0;
        }
        // Escribimos el inodo en el AI
        if (escribir_inodo(posInodoReservado, reservado) == ERROR)
        {
            perror("Error: ");
            return ERROR;
        }

        // Lo escribimos en el disco
        if (bwrite(0, &SB) == ERROR)
        {
            perror("Error: ");
            return ERROR;
        }
        // Retornamos la posición del inodo que acabamos de reservar
        return posInodoReservado;
    }

    // CASO EN EL QUE NO QUEDAN INODOS LIBRES
    else
    {
        printf("No quedan inodos libres\n");
        return ERROR;
    }
}

// NIVEL 4
int obtener_nRangoBL(struct inodo *inodo, unsigned int nblogico, unsigned int *ptr)
{
    if (nblogico < DIRECTOS)
    {
        *ptr = inodo->punterosDirectos[nblogico];
        return 0;
    }
    if (nblogico < INDIRECTOS0)
    {
        *ptr = inodo->punterosIndirectos[0];
        return 1;
    }
    if (nblogico < INDIRECTOS1)
    {
        *ptr = inodo->punterosIndirectos[1];
        return 2;
    }
    if (nblogico < INDIRECTOS2)
    {
        *ptr = inodo->punterosIndirectos[2];
        return 3;
    }
    *ptr = 0;
    perror("Error:");
    return ERROR;
}

int obtener_indice(unsigned int nblogico, int nivel_punteros)
{
    if (nblogico < DIRECTOS)
    {
        return nblogico;
    }
    if (nblogico < INDIRECTOS0)
    {
        return nblogico - DIRECTOS;
    }
    if (nblogico < INDIRECTOS1)
    {
        if (nivel_punteros == 2)
        {
            return (nblogico - INDIRECTOS0) / NPUNTEROS;
        }
        if (nivel_punteros == 1)
        {
            return (nblogico - INDIRECTOS0) % NPUNTEROS;
        }
    }
    if (nblogico < INDIRECTOS2)
    {
        if (nivel_punteros == 3)
        {
            return (nblogico - INDIRECTOS1) / (NPUNTEROS * NPUNTEROS);
        }
        if (nivel_punteros == 2)
        {
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS) / NPUNTEROS);
        }
        if (nivel_punteros == 1)
        {
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS) % NPUNTEROS);
        }
    }
    return ERROR;
}


int traducir_bloque_inodo(unsigned int ninodo, unsigned int nblogico, unsigned char reservar)
{
    unsigned int ptr = 0;
    // leemos el inodo solicitado.
    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) == ERROR)
    {
        perror("ERROR");
        return ERROR;
    }
    if (nblogico < DIRECTOS)
    { // el bloque logico es uno de los 12 primeros bloques logicos del inodo.
        switch (reservar)
        {
        case 0:                                        // modo consulta
            if (inodo.punterosDirectos[nblogico] == 0) // no tiene bloque físico asignado.
                return -1;
            else
            {
                ptr = inodo.punterosDirectos[nblogico];
            }
            break;
        case 1: // modo escritura
            if (inodo.punterosDirectos[nblogico] == 0)
            { // si no tiene bloque fisico le asignamos uno.
                inodo.punterosDirectos[nblogico] = reservar_bloque();
                ptr = inodo.punterosDirectos[nblogico];
                // aumentamos en uno el numero de bloques ocupados por el inodo en la zona de datos:
                inodo.numBloquesOcupados++;
                inodo.ctime = time(NULL);
                // escribimos el inodo con la info actualizada
                if (escribir_inodo(ninodo, &inodo) == ERROR)
                {
                    perror("ERROR: ");
                    return ERROR;
                }
            }
            else
            { // tiene bloque fisico asignado y lo devolvemos
                ptr = inodo.punterosDirectos[nblogico];
            }
            break;
        }
#if DEBUGN4
        printf("[traducir_bloque_inodo() → inodo.punterosDirectos[%d] =  %d (reservado BF %u para BL %u)]\n", nblogico, ptr, ptr, nblogico);
#endif
        // printf("nblogico= %d, ptr= %d\n", nblogico, ptr);
        return ptr;
    }
    // PUNTERO INDIRECTOS 0
    // El bloque logico lo encontramos en el rango de Indirectos 0, es decir,
    // está comprendido entre   el 0+12 y el 0+12+256-1: entre el 12 y el 267
    else if (nblogico < INDIRECTOS0)
    {
        unsigned int punteros_nivel1[NPUNTEROS];
        switch (reservar)
        {
        case 0:                                   // modo consulta
            if (inodo.punterosIndirectos[0] == 0) // no hay bloque fisico asignado a punteros_nivel1.
                return -1;
            else
            { // ya existe el bloque de punteros_nivel1 y lo leemos del dispositivo
                if (bread(inodo.punterosIndirectos[0], punteros_nivel1) == ERROR)
                {
                    perror("ERROR");
                    return ERROR;
                }
                if (punteros_nivel1[nblogico - DIRECTOS] == 0)
                    // no hay bloque físico asignado al bloque lógico de datos.
                    return -1;
                else
                {
                    ptr = punteros_nivel1[nblogico - DIRECTOS];
                }
            }
            break;
        case 1: // modo escritura
            if (inodo.punterosIndirectos[0] == 0)
            {                                                             // no hay bloque fisico asignado a punteros_nivel1
                inodo.punterosIndirectos[0] = reservar_bloque();          // para punteros_nivel1
                memset(punteros_nivel1, 0, BLOCKSIZE);                    // iniciamos a 0 los 256 punteros
                punteros_nivel1[nblogico - DIRECTOS] = reservar_bloque(); // para datos
                // aumentamos el numero de bloques ocupados por el inodo en la zona de datos
                inodo.numBloquesOcupados += 2;
                inodo.ctime = time(NULL);
                // salvamos el bloque de punteros_nivel1 en el dispositivo.
                if (bwrite(inodo.punterosIndirectos[0], punteros_nivel1) == ERROR)
                {
                    perror("ERROR");
                    return ERROR;
                }
                // devolvemos el bloque fisico de datos
                ptr = punteros_nivel1[nblogico - DIRECTOS];
            }
            else
            { // existe el bloque de punteros_nivel1 y lo leemos del dispositivo
                if (bread(inodo.punterosIndirectos[0], punteros_nivel1) == ERROR)
                {
                    perror("ERROR");
                    return ERROR;
                }
                if (punteros_nivel1[nblogico - DIRECTOS] == 0)
                {
                    // no hay bloque fisico de datos asignado, entonces lo reservamos
                    punteros_nivel1[nblogico - DIRECTOS] = reservar_bloque(); // para datos
                    // salvamos el bloque de punteros_nivel1 en el dispositivo.
                    if (bwrite(inodo.punterosIndirectos[0], punteros_nivel1) == ERROR)
                    {
                        perror("ERROR");
                        return ERROR;
                    }
                    ptr = punteros_nivel1[nblogico - DIRECTOS]; // devolvemos el bloque fisico de datos.
                    // aumentamos el numero de bloques ocupados por el inodo en la zona de datos.
                    inodo.numBloquesOcupados++;
                    inodo.ctime = time(NULL);
                }
                else
                { // si existe el bloque fisico de datos lo devolvemos
                    ptr = punteros_nivel1[nblogico - DIRECTOS];
                }
            }
            // escribimos en el dispositivo el inodo actualizado
            if (escribir_inodo(ninodo, &inodo) == ERROR)
            {
                perror("ERROR");
                return ERROR;
            }
#if DEBUGN4
            printf("[traducir_bloque_inodo() → punteros_nivel0[0] = %d (reservado BF %u para BL %u)]\n", ptr, ptr, nblogico);
#endif
            break;
        }
        // printf("nblogico= %d, ptr= %d\n", nblogico, ptr);
        return ptr;
    }
    // PUNTERO INDIRECTOS 1
    // El bloque logico lo encontramos en el rango de Indirectos 1, es decir,
    // los comprendidos entre el 0+12+256 y el 0+12+256+256^2-1: entre el 268 y el 65.803.
    else if (nblogico < INDIRECTOS1)
    {
        unsigned int punteros_nivel1[NPUNTEROS];
        unsigned int punteros_nivel2[NPUNTEROS];
        unsigned int indice_nivel1 = obtener_indice(nblogico, 1); // indice para punteros_nivel1
        unsigned int indice_nivel2 = obtener_indice(nblogico, 2); // indice para punteros_nivel2
        switch (reservar)
        {
        case 0:                                   // modo consulta
            if (inodo.punterosIndirectos[1] == 0) // no hay bloque fisico asignado a punteros_nivel2.
                return -1;
            else
            { // ya existe el bloque de punteros_nivel2 y lo leemos del dispositivo
                if (bread(inodo.punterosIndirectos[1], punteros_nivel2) == ERROR)
                {
                    perror("ERROR");
                    return ERROR;
                }
                if (punteros_nivel2[indice_nivel2] == 0)
                { // no hay bloque fisico asignado a punteros_nivel1.
                    return -1;
                }
                else
                { // ya existe el bloque de punteros_nivel1 y lo leemos del dispositivo
                    if (bread(punteros_nivel2[indice_nivel2], punteros_nivel1) == ERROR)
                    {
                        perror("ERROR");
                        return ERROR;
                    }
                    if (punteros_nivel1[indice_nivel1] == 0)
                        // no hay bloque físico asignado al bloque lógico de datos.
                        return -1;
                    else
                    {
                        ptr = punteros_nivel1[indice_nivel1]; // devolvemos el bloque fisico solicitado
                    }
                }
            }
            break;
        case 1: // modo escritura
            if (inodo.punterosIndirectos[1] == 0)
            {                                                       // no hay bloque fisico asignado a punteros_nivel2
                inodo.punterosIndirectos[1] = reservar_bloque();    // para punteros_nivel2
                memset(punteros_nivel2, 0, BLOCKSIZE);              // iniciamos a 0 los 256 punteros de nivel2
                punteros_nivel2[indice_nivel2] = reservar_bloque(); // para punteros_nivel1
                memset(punteros_nivel1, 0, BLOCKSIZE);              // iniciamos a 0 los 256 punteros de nivel1
                punteros_nivel1[indice_nivel1] = reservar_bloque(); // para datos
                // salvamos los buffers de los bloques de punteros en el dispositivo
                if (bwrite(inodo.punterosIndirectos[1], punteros_nivel2) == ERROR)
                {
                    perror("ERROR");
                    return ERROR;
                }
                if (bwrite(punteros_nivel2[indice_nivel2], punteros_nivel1) == ERROR)
                {
                    perror("ERROR");
                    return ERROR;
                }
                // devolvemos el bloque fisico de datos
                ptr = punteros_nivel1[indice_nivel1];
                // aumentamos el numero de bloques ocupados por el inodo en la zona de datos.
                inodo.numBloquesOcupados += 3;
                inodo.ctime = time(NULL);
            }
            else
            { // existe el bloque de punteros_nivel2 y lo leemos del dispositivo
                if (bread(inodo.punterosIndirectos[1], punteros_nivel2) == ERROR)
                {
                    perror("ERROR");
                    return ERROR;
                }
                if (punteros_nivel2[indice_nivel2] == 0)
                {                                                       // no hay bloque fisico asignado a punteros_nivel1
                    punteros_nivel2[indice_nivel2] = reservar_bloque(); // para punteros_nivel1
                    memset(punteros_nivel1, 0, BLOCKSIZE);              // iniciamos a 0 los 256 punteros de nivel1
                    punteros_nivel1[indice_nivel1] = reservar_bloque(); // para datos
                    // salvamos los buffers de los bloques de punteros en el dispositivo
                    if (bwrite(inodo.punterosIndirectos[1], punteros_nivel2) == ERROR)
                    {
                        perror("ERROR");
                        return ERROR;
                    }
                    if (bwrite(punteros_nivel2[indice_nivel2], punteros_nivel1) == ERROR)
                    {
                        perror("ERROR");
                        return ERROR;
                    }
                    // devolvemos el bloque fisico de datos
                    ptr = punteros_nivel1[indice_nivel1];
                    // aumentamos el numero de bloques ocupados por el inodo en la zona de datos.
                    inodo.numBloquesOcupados += 2;
                    inodo.ctime = time(NULL);
                }
                else
                { // existe el bloque de punteros_nivel1 y lo leemos del dispositivo
                    if (bread(punteros_nivel2[indice_nivel2], punteros_nivel1) == ERROR)
                    {
                        perror("ERROR");
                        return ERROR;
                    }
                    if (punteros_nivel1[indice_nivel1] == 0)
                    {                                                       // no hay bloque físico asignado al bloque de datos
                        punteros_nivel1[indice_nivel1] = reservar_bloque(); // para datos
                        // salvamos el bloque de punteros_nivel1 en el dispositivo
                        if (bwrite(punteros_nivel2[indice_nivel2], punteros_nivel1) == ERROR)
                        {
                            perror("ERROR");
                            return ERROR;
                        }
                        // devolvemos el bloque físico asignado a los datos
                        ptr = punteros_nivel1[indice_nivel1];
                        // aumentamos en uno el numero de bloques ocupados por el inodo en la zona de datos.
                        inodo.numBloquesOcupados++;
                        inodo.ctime = time(NULL);
                    }
                    else
                    {
                        ptr = punteros_nivel1[indice_nivel1];
                    }
                }
            }
            // escribimos en el dispositivo el inodo actualizado
            escribir_inodo(ninodo, &inodo);
#if DEBUGN4
            printf("[traducir_bloque_inodo() → punteros_nivel1[1] = %d (reservado BF %u para BL %u)]\n", ptr, ptr, nblogico);
#endif
            break;
        }
        // printf("nblogico= %d, ptr= %d\n", nblogico, ptr);
        return ptr;
    }
    // PUNTERO INDIRECTOS 2
    // El bloque logico lo encontramos en el rango de Indirectos 2, es decir, los comprendidos entre
    // el 0+12+256+256^2 y el 0+12+256+256^2+256^3-1: entre el 65.804 y el 16.843.019.
    else if (nblogico < INDIRECTOS2)
    {
        unsigned int punteros_nivel1[NPUNTEROS];
        unsigned int punteros_nivel2[NPUNTEROS];
        unsigned int punteros_nivel3[NPUNTEROS];
        unsigned int indice_nivel1 = obtener_indice(nblogico, 1); // indice para punteros_nivel1
        unsigned int indice_nivel2 = obtener_indice(nblogico, 2); // indice para punteros_nivel2
        unsigned int indice_nivel3 = obtener_indice(nblogico, 3); // indice para punteros_nivel3
        switch (reservar)
        {
        case 0:                                   // modo consulta
            if (inodo.punterosIndirectos[2] == 0) // no hay bloque fisico asignado a punteros_nivel3.
                return -1;
            else
            { // ya existe el bloque de punteros_nivel3 y lo leemos del dispositivo
                if (bread(inodo.punterosIndirectos[2], punteros_nivel3) == ERROR)
                {
                    perror("ERROR");
                    return ERROR;
                }
                if (punteros_nivel3[indice_nivel3] == 0) // no hay bloque fisico asignado a punteros_nivel2.
                    return -1;
                else
                { // ya existe el bloque de punteros_nivel2 y lo leemos del dispositivo
                    if (bread(punteros_nivel3[indice_nivel3], punteros_nivel2) == ERROR)
                    {
                        perror("ERROR");
                        return ERROR;
                    }
                    if (punteros_nivel2[indice_nivel2] == 0) // no hay bloque fisico asignado a punteros_nivel1.
                        return -1;
                    else
                    { // ya existe el bloque de punteros_nivel1 y lo leemos del dispositivo
                        if (bread(punteros_nivel2[indice_nivel2], punteros_nivel1) == ERROR)
                        {
                            perror("ERROR");
                            return ERROR;
                        }
                        if (punteros_nivel1[indice_nivel1] == 0)
                            // no hay bloque físico asignado al bloque lógico de datos.
                            return -1;
                        else
                        {
                            ptr = punteros_nivel1[indice_nivel1]; // devolvemos el bloque fisico solicitado
                        }
                    }
                }
            }
            break;
        case 1: // modo escritura
            if (inodo.punterosIndirectos[2] == 0)
            {                                                       // no hay bloque fisico asignado a punteros_nivel3
                inodo.punterosIndirectos[2] = reservar_bloque();    // para punteros_nivel3
                memset(punteros_nivel3, 0, BLOCKSIZE);              // iniciamos a 0 los 256 punteros de nivel3
                punteros_nivel3[indice_nivel3] = reservar_bloque(); // para punteros_nivel2
                memset(punteros_nivel2, 0, BLOCKSIZE);              // iniciamos a 0 los 256 punteros de nivel2
                punteros_nivel2[indice_nivel2] = reservar_bloque(); // para punteros_nivel1
                memset(punteros_nivel1, 0, BLOCKSIZE);              // iniciamos a 0 los 256 punteros de nivel1
                punteros_nivel1[indice_nivel1] = reservar_bloque(); // para datos
                // salvamos los buffers de los bloques de punteros en el dispositivo
                if (bwrite(inodo.punterosIndirectos[2], punteros_nivel3) == ERROR)
                {
                    perror("ERROR");
                    return ERROR;
                }
                if (bwrite(punteros_nivel3[indice_nivel3], punteros_nivel2) == ERROR)
                {
                    perror("ERROR");
                    return ERROR;
                }
                if (bwrite(punteros_nivel2[indice_nivel2], punteros_nivel1) == ERROR)
                {
                    perror("ERROR");
                    return ERROR;
                }
                // devolvemos el bloque fisico de datos
                ptr = punteros_nivel1[indice_nivel1];
                // aumentamos en uno el numero de bloques ocupados por el inodo en la zona de datos.
                inodo.numBloquesOcupados += 4;
                inodo.ctime = time(NULL);
            }
            else
            { // existe el bloque de punteros_nivel3 y lo leemos del dispositivo
                if (bread(inodo.punterosIndirectos[2], punteros_nivel3) == ERROR)
                {
                    perror("ERROR");
                    return ERROR;
                }
                if (punteros_nivel3[indice_nivel3] == 0)
                {                                                       // no hay bloque fisico asignado a punteros_nivel2
                    punteros_nivel3[indice_nivel3] = reservar_bloque(); // para punteros_nivel2
                    memset(punteros_nivel2, 0, BLOCKSIZE);              // iniciamos a 0 los 256 punteros de nivel2
                    punteros_nivel2[indice_nivel2] = reservar_bloque(); // para punteros_nivel1
                    memset(punteros_nivel1, 0, BLOCKSIZE);              // iniciamos a 0 los 256 punteros de nivel1
                    punteros_nivel1[indice_nivel1] = reservar_bloque(); // para datos
                    // salvamos los buffers de los bloques de punteros en el dispositivo
                    if (bwrite(inodo.punterosIndirectos[2], punteros_nivel3) == ERROR)
                    {
                        perror("ERROR");
                        return ERROR;
                    }
                    if (bwrite(punteros_nivel3[indice_nivel3], punteros_nivel2) == ERROR)
                    {
                        perror("ERROR");
                        return ERROR;
                    }
                    if (bwrite(punteros_nivel2[indice_nivel2], punteros_nivel1) == ERROR)
                    {
                        perror("ERROR");
                        return ERROR;
                    }
                    // devolvemos el bloque fisico de datos
                    ptr = punteros_nivel1[indice_nivel1];
                    // aumentamos el numero de bloques ocupados por el inodo en la zona de datos.
                    inodo.numBloquesOcupados += 3;
                    inodo.ctime = time(NULL);
                }
                else
                { // existe el bloque de punteros_nivel2 y lo leemos del dispositivo
                    if (bread(punteros_nivel3[indice_nivel3], punteros_nivel2) == ERROR)
                    {
                        perror("ERROR");
                        return ERROR;
                    }
                    if (punteros_nivel2[indice_nivel2] == 0)
                    {                                                       // no hay bloque fisico asignado a punteros_nivel1
                        punteros_nivel2[indice_nivel2] = reservar_bloque(); // para punteros_nivel1
                        memset(punteros_nivel1, 0, BLOCKSIZE);              // iniciamos a 0 los 256 punteros de nivel1
                        punteros_nivel1[indice_nivel1] = reservar_bloque(); // para datos
                        // salvamos los buffers de los bloques de punteros en el dispositivo
                        if (bwrite(punteros_nivel3[indice_nivel3], punteros_nivel2) == ERROR)
                        {
                            perror("ERROR");
                            return ERROR;
                        }
                        if (bwrite(punteros_nivel2[indice_nivel2], punteros_nivel1) == ERROR)
                        {
                            perror("ERROR");
                            return ERROR;
                        }
                        // devolvemos el bloque fisico de datos
                        ptr = punteros_nivel1[indice_nivel1];
                        // aumentamos el numero de bloques ocupados por el inodo en la zona de datos.
                        inodo.numBloquesOcupados += 2;
                        inodo.ctime = time(NULL);
                    }
                    else
                    { // existe el bloque de punteros_nivel1 y lo leemos del dispositivo
                        bread(punteros_nivel2[indice_nivel2], punteros_nivel1);
                        if (punteros_nivel1[indice_nivel1] == 0)
                        {                                                       // no hay bloque físico asignado al bloque de datos
                            punteros_nivel1[indice_nivel1] = reservar_bloque(); // para datos
                            // salvamos el bloque de punteros_nivel1 en el dispositivo
                            if (bwrite(punteros_nivel2[indice_nivel2], punteros_nivel1) == ERROR)
                            {
                                perror("ERROR");
                                return ERROR;
                            }
                            // devolvemos el bloque físico asignado a los datos
                            ptr = punteros_nivel1[indice_nivel1];
                            // aumentamos el numero de bloques ocupados por el inodo en la zona de datos.
                            inodo.numBloquesOcupados++;
                            inodo.ctime = time(NULL);
                        }
                        else
                        {
                            ptr = punteros_nivel1[indice_nivel1];
                        }
                    }
                }
            }
            // escribimos en el dispositivo el inodo actualizado
            if (escribir_inodo(ninodo, &inodo) == ERROR)
            {
                perror("ERROR");
                return ERROR;
            }
#if DEBUGN4
            printf("[traducir_bloque_inodo() → punteros_nivel2[2] = %d (reservado BF %u para BL %u)]\n", ptr, ptr, nblogico);
#endif
            break;
        }
        // printf("nblogico= %d, ptr= %d\n", nblogico, ptr);
        return ptr;
    }
    return ptr;
}
// Nivel 6

int liberar_inodo(unsigned int ninodo)
{
    struct inodo inodo;
    struct superbloque SB;
    // Leemos el inodo
    if (leer_inodo(ninodo, &inodo) == ERROR)
    {
        perror("ERROR: ");
        return ERROR;
    }
    // Liberamos sus bloques
    int bliberados = liberar_bloques_inodos(0, &inodo);
    // Control de errores
    if (bliberados == ERROR)
    {
        perror("ERROR: ");
        return ERROR;
    }
    // Actualizamos inodo.numBloquesOcupados
    inodo.numBloquesOcupados = inodo.numBloquesOcupados - bliberados;

    if (inodo.numBloquesOcupados == 0)
    {
        // Lo marcamos como vacío y libre
        inodo.tamEnBytesLog = 0;
        inodo.tipo = 'l';

        if (bread(0, &SB) == ERROR)
        {
            perror("ERROR: ");
            return ERROR;
        }
        // Ponemos el inodo que hemos liberado en la lista de libres
        inodo.punterosDirectos[0] = SB.posPrimerInodoLibre;
        SB.posPrimerInodoLibre = ninodo;
        SB.cantInodosLibres++;

        // Lo escribimos en el AI
        if (escribir_inodo(ninodo, &inodo) == ERROR)
        {
            perror("ERROR: ");
            return ERROR;
        }
        // Escribims el SB actualizado
        if (bwrite(0, &SB) == ERROR)
        {
            perror("ERROR: ");
            return ERROR;
        }
        return ninodo;
    }
    else
    {
        printf("[liberar_inodo()-> ERROR! inodo.numBloquesOcupados != 0. inodo.numBloquesOcupados = %u]", inodo.numBloquesOcupados);
        return ERROR;
    }
}

int liberar_bloques_inodos(unsigned int primerBL, struct inodo *inodo)
{
    unsigned int nivel_punteros, nblog, ultimoBL;
    unsigned char bufAux_punteros[BLOCKSIZE];
    unsigned int bloques_punteros[3][NPUNTEROS];
    int indices_primerBL[3]; // indices del primerBL para cuando se llama desde mi_truncar_f()
    int liberados = 0;
    int i, j, k;                          // para iterar en cada nivel de punteros
    int eof = 0;                          // para determinar si hemos llegado al último BL
    int contador_breads = 0;              // para comprobar optimización eficiencia
    int contador_bwrites = 0;             // para comprobar optimización eficiencia
    int bloque_modificado[3] = {0, 0, 0}; // para saber si se ha modificado un bloque de punteros de algún nivel

#if DEBUGN6
    int BLliberado = 0; // utilizado para imprimir el nº de bloque lógico que se ha liberado
#endif

    // Si el inodo está vacío
    if (inodo->tamEnBytesLog == 0)
    {

        return 0;
    }

    // Calculamos la posición del último BL
    if (inodo->tamEnBytesLog % BLOCKSIZE == 0)
    {

        ultimoBL = inodo->tamEnBytesLog / BLOCKSIZE - 1;
    }
    else
    {

        ultimoBL = inodo->tamEnBytesLog / BLOCKSIZE;
    }

#if DEBUGN6
    fprintf(stderr, "[liberar_bloques_inodo()→ primer BL: %d, último BL: %d]\n", primerBL, ultimoBL);
#endif

    memset(bufAux_punteros, 0, BLOCKSIZE);

    // liberamos los bloques de datos de punteros directos
    if (primerBL < DIRECTOS)
    {
        nivel_punteros = 0;
        i = obtener_indice(primerBL, nivel_punteros);
        while (!eof && i < DIRECTOS)
        {
            nblog = i;
            if (nblog == ultimoBL)
                eof = 1;
            if (inodo->punterosDirectos[i])
            {

                liberar_bloque(inodo->punterosDirectos[i]);
#if DEBUGN6
                fprintf(stderr, "[liberar_bloques_inodo()→ liberado BF %d de datos para BL %d]\n", inodo->punterosDirectos[i], nblog);
                // BLliberado=nblog;
#endif
                liberados++;
                inodo->punterosDirectos[i] = 0;
            }
            i++;
        }
    }

    // liberamos los bloques de datos e índice de Indirectos[0]
    if (primerBL < INDIRECTOS0 && !eof)
    {
        nivel_punteros = 1;
        if (inodo->punterosIndirectos[0])
        {
            bread(inodo->punterosIndirectos[0], bloques_punteros[nivel_punteros - 1]);
            bloque_modificado[nivel_punteros - 1] = 0;
            contador_breads++;
            if (primerBL >= DIRECTOS)
            {
                i = obtener_indice(primerBL, nivel_punteros);
            }
            else
            {
                i = 0;
            }
            while (!eof && i < NPUNTEROS)
            {
                nblog = DIRECTOS + i;
                if (nblog == ultimoBL)
                    eof = 1;
                if (bloques_punteros[nivel_punteros - 1][i])
                {
                    liberar_bloque(bloques_punteros[nivel_punteros - 1][i]);
#if DEBUGN6
                    fprintf(stderr, "[liberar_bloques_inodo()→ liberado BF %d de datos para BL %d]\n", bloques_punteros[nivel_punteros - 1][i], nblog);
                    BLliberado = nblog;
#endif
                    liberados++;
                    bloques_punteros[nivel_punteros - 1][i] = 0;
                    bloque_modificado[nivel_punteros - 1] = 1;
                }
                i++;
            }
            if (memcmp(bloques_punteros[nivel_punteros - 1], bufAux_punteros, BLOCKSIZE) == 0)
            {
                liberar_bloque(inodo->punterosIndirectos[0]); // de punteros
#if DEBUGN6
                fprintf(stderr, "[liberar_bloques_inodo()→ liberado BF %d de punteros_nivel%d correspondiente al BL %d]\n", inodo->punterosIndirectos[0], nivel_punteros, BLliberado);
#endif
                liberados++;
                inodo->punterosIndirectos[0] = 0;
            }
            else
            { // escribimos en el dispositivo el bloque de punteros, si ha sido modificado
                if (bloque_modificado[nivel_punteros - 1])
                {
                    if (bwrite(inodo->punterosIndirectos[0], bloques_punteros[nivel_punteros - 1]) < 0)
                        return -1;
                    contador_bwrites++;
                }
            }
        }
    }

    // liberamos los bloques de datos e índice de Indirectos[1]
    if (primerBL < INDIRECTOS1 && !eof)
    {
        nivel_punteros = 2;
        indices_primerBL[0] = 0;
        indices_primerBL[1] = 0;
        if (inodo->punterosIndirectos[1])
        {
            bread(inodo->punterosIndirectos[1], bloques_punteros[nivel_punteros - 1]);
            bloque_modificado[nivel_punteros - 1] = 0;
            contador_breads++;
            if (primerBL >= INDIRECTOS0)
            {
                i = obtener_indice(primerBL, nivel_punteros);
            }
            else
                i = 0;
            indices_primerBL[nivel_punteros - 1] = i;
            while (!eof && i < NPUNTEROS)
            {
                if (bloques_punteros[nivel_punteros - 1][i])
                {
                    bread(bloques_punteros[nivel_punteros - 1][i], bloques_punteros[nivel_punteros - 2]);
                    bloque_modificado[nivel_punteros - 2] = 0;
                    contador_breads++;
                    if (i == indices_primerBL[nivel_punteros - 1])
                    {
                        j = obtener_indice(primerBL, nivel_punteros - 1);
                        indices_primerBL[nivel_punteros - 2] = j;
                    }
                    else
                        j = 0;

                    while (!eof && j < NPUNTEROS)
                    {
                        nblog = INDIRECTOS0 + i * NPUNTEROS + j;
                        if (nblog == ultimoBL)
                            eof = 1;
                        if (bloques_punteros[nivel_punteros - 2][j])
                        {
                            liberar_bloque(bloques_punteros[nivel_punteros - 2][j]);
#if DEBUGN6
                            fprintf(stderr, "[liberar_bloques_inodo()→ liberado BF %d de datos para BL %d]\n", bloques_punteros[nivel_punteros - 2][j], nblog);
                            BLliberado = nblog;
#endif
                            liberados++;
                            bloques_punteros[nivel_punteros - 2][j] = 0;
                            bloque_modificado[nivel_punteros - 2] = 1;
                        }
                        j++;
                    }
                    if (memcmp(bloques_punteros[nivel_punteros - 2], bufAux_punteros, BLOCKSIZE) == 0)
                    {
                        liberar_bloque(bloques_punteros[nivel_punteros - 1][i]); // de punteros
#if DEBUGN6
                        fprintf(stderr, "[liberar_bloques_inodo()→ liberado BF %d de punteros_nivel%d correspondiente al BL %d]\n", bloques_punteros[nivel_punteros - 1][i], nivel_punteros - 1, BLliberado);
#endif
                        liberados++;
                        bloques_punteros[nivel_punteros - 1][i] = 0;
                        bloque_modificado[nivel_punteros - 1] = 1;
                    }
                    else
                    { // escribimos en el dispositivo el bloque de punteros, si ha sido modificado
                        if (bloque_modificado[nivel_punteros - 2])
                        {
                            if (bwrite(bloques_punteros[nivel_punteros - 1][i], bloques_punteros[nivel_punteros - 2]) < 0)
                                return -1;
                            contador_bwrites++;
                        }
                    }
                }
                i++;
            }
            if (memcmp(bloques_punteros[nivel_punteros - 1], bufAux_punteros, BLOCKSIZE) == 0)
            {
                liberar_bloque(inodo->punterosIndirectos[1]); // de punteros
#if DEBUGN6
                fprintf(stderr, "[liberar_bloques_inodo()→ liberado BF %d de punteros_nivel%d correspondiente al BL %d]\n", inodo->punterosIndirectos[1], nivel_punteros, BLliberado);
#endif
                liberados++;
                inodo->punterosIndirectos[1] = 0;
            }
            else
            { // escribimos en el dispositivo el bloque de punteros, si ha sido modificado
                if (bloque_modificado[nivel_punteros - 1])
                {
                    if (bwrite(inodo->punterosIndirectos[1], bloques_punteros[nivel_punteros - 1]) < 0)
                        return -1;
                    contador_bwrites++;
                }
            }
        }
    }

    // liberamos los bloques de datos e índice de Indirectos[2]
    if (primerBL < INDIRECTOS2 && !eof)
    {
        nivel_punteros = 3;
        indices_primerBL[0] = 0;
        indices_primerBL[1] = 0;
        indices_primerBL[2] = 0;
        if (inodo->punterosIndirectos[2])
        {
            bread(inodo->punterosIndirectos[2], bloques_punteros[nivel_punteros - 1]);
            bloque_modificado[nivel_punteros - 1] = 0;
            contador_breads++;
            if (primerBL >= INDIRECTOS1)
            {
                i = obtener_indice(primerBL, nivel_punteros);
                indices_primerBL[nivel_punteros - 1] = i;
            }
            else
                i = 0;
            while (!eof && i < NPUNTEROS)
            {
                if (bloques_punteros[nivel_punteros - 1][i])
                {
                    bread(bloques_punteros[nivel_punteros - 1][i], bloques_punteros[nivel_punteros - 2]);
                    contador_breads++;
                    if (i == indices_primerBL[nivel_punteros - 1])
                    {
                        j = obtener_indice(primerBL, nivel_punteros - 1);
                        indices_primerBL[nivel_punteros - 2] = j;
                    }
                    else
                        j = 0;
                    while (!eof && j < NPUNTEROS)
                    {
                        if (bloques_punteros[nivel_punteros - 2][j])
                        {
                            bread(bloques_punteros[nivel_punteros - 2][j], bloques_punteros[nivel_punteros - 3]);
                            contador_breads++;
                            if (i == indices_primerBL[nivel_punteros - 1] && j == indices_primerBL[nivel_punteros - 2])
                            {
                                k = obtener_indice(primerBL, nivel_punteros - 2);
                                indices_primerBL[nivel_punteros - 3] = k;
                            }
                            else
                                k = 0;
                            while (!eof && k < NPUNTEROS)
                            {
                                nblog = INDIRECTOS1 + i * NPUNTEROS2 + j * NPUNTEROS + k;
                                if (nblog == ultimoBL)
                                    eof = 1;
                                if (bloques_punteros[nivel_punteros - 3][k])
                                {
                                    liberar_bloque(bloques_punteros[nivel_punteros - 3][k]);
#if DEBUGN6
                                    fprintf(stderr, "[liberar_bloques_inodo()→ liberado BF %d de datos para BL %d]\n", bloques_punteros[nivel_punteros - 3][k], nblog);
                                    BLliberado = nblog;
#endif
                                    liberados++;
                                    bloques_punteros[nivel_punteros - 3][k] = 0;
                                    bloque_modificado[nivel_punteros - 3] = 1;
                                }
                                k++;
                            }
                            if (memcmp(bloques_punteros[nivel_punteros - 3], bufAux_punteros, BLOCKSIZE) == 0)
                            {
                                liberar_bloque(bloques_punteros[nivel_punteros - 2][j]); // de punteros
#if DEBUGN6
                                fprintf(stderr, "[liberar_bloques_inodo()→ liberado BF %d de punteros_nivel%d correspondiente al BL %d]\n", bloques_punteros[nivel_punteros - 2][j], nivel_punteros - 2, BLliberado);
#endif
                                liberados++;
                                bloques_punteros[nivel_punteros - 2][j] = 0;
                                bloque_modificado[nivel_punteros - 2] = 1;
                            }
                            else
                            { // escribimos en el dispositivo el bloque de punteros, si ha sido modificado
                                if (bloque_modificado[nivel_punteros - 3])
                                {
                                    if (bwrite(bloques_punteros[nivel_punteros - 2][j], bloques_punteros[nivel_punteros - 3]) < 0)
                                        return -1;
                                    contador_bwrites++;
                                }
                            }
                        }
                        j++;
                    }
                    if (memcmp(bloques_punteros[nivel_punteros - 2], bufAux_punteros, BLOCKSIZE) == 0)
                    {
                        liberar_bloque(bloques_punteros[nivel_punteros - 1][i]); // de punteros
#if DEBUGN6
                        fprintf(stderr, "[liberar_bloques_inodo()→ liberado BF %d de punteros_nivel%d correspondiente al BL %d]\n", bloques_punteros[nivel_punteros - 1][i], nivel_punteros - 1, BLliberado);
#endif
                        liberados++;
                        bloques_punteros[nivel_punteros - 1][i] = 0;
                        bloque_modificado[nivel_punteros - 1] = 1;
                    }
                    else
                    { // escribimos en el dispositivo el bloque de punteros, si ha sido modificado
                        if (bloque_modificado[nivel_punteros - 2])
                        {
                            if (bwrite(bloques_punteros[nivel_punteros - 1][i], bloques_punteros[nivel_punteros - 2]) < 0)
                                return -1;
                            contador_bwrites++;
                        }
                    }
                }
                i++;
            }
            if (memcmp(bloques_punteros[nivel_punteros - 1], bufAux_punteros, BLOCKSIZE) == 0)
            {
                liberar_bloque(inodo->punterosIndirectos[2]); // de punteros
#if DEBUGN6
                fprintf(stderr, "[liberar_bloques_inodo()→ liberado BF %d de punteros_nivel%d correspondiente al BL %d]\n", inodo->punterosIndirectos[2], nivel_punteros, BLliberado);
#endif
                liberados++;
                inodo->punterosIndirectos[2] = 0;
            }
            else
            { // escribimos en el dispositivo el bloque de punteros, si ha sido modificado
                if (bloque_modificado[nivel_punteros - 1])
                {
                    if (bwrite(inodo->punterosIndirectos[2], bloques_punteros[nivel_punteros - 1]) < 0)
                        return  -1;
                    contador_bwrites++;
                }
            }
        }
    }

#if DEBUGN6
    fprintf(stderr, "[liberar_bloques_inodo()→ total bloques liberados: %d, total breads: %d, total_bwrites:%d]\n", liberados, contador_breads, contador_bwrites);
#endif
    return liberados;
}