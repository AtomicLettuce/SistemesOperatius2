#include "ficheros_basico.h"
#include "debugging.h"
#define ERROR -1
int main(int argc, char **argv)
{
    struct superbloque SB;
    char *camino = argv[1];

    if (argc != 2)
    {
        printf("ERROR: ");
    }
    if (bmount(camino) == ERROR)
    {
        return ERROR;
    }
    if (bread(0, &SB) != ERROR)
    {

        // mostrar el superbloque

        printf("DATOS DEL SUPERBLOQUE\n");
        printf("posPrimerBloqueMB: %u\n", SB.posPrimerBloqueMB);
        printf("posUltimoBloqueMB: %u\n", SB.posUltimoBloqueMB);
        printf("posPrimerBloqueAI: %u\n", SB.posPrimerBloqueAI);
        printf("posUltimoBloqueMB: %u\n", SB.posUltimoBloqueAI);
        printf("posPrimerBloqueDatos: %u\n", SB.posPrimerBloqueDatos);
        printf("posUltimoBloqueDatos: %u\n", SB.posUltimoBloqueDatos);
        printf("posInodoRaiz: %u\n", SB.posInodoRaiz);
        printf("posPrimerInodoLibre: %u\n", SB.posPrimerInodoLibre);
        printf("cantBloquesLibres: %u\n", SB.cantBloquesLibres);
        printf("cantInodosLibres: %u\n", SB.cantInodosLibres);
        printf("totBloques: %u\n", SB.totBloques);
        printf("totInodos: %u\n\n", SB.totInodos);

#if DEBUGN2
        printf("sizeof struc superbloque: %lu\n", sizeof(struct superbloque));
        printf("sizeof struc inodo is: %lu\n", sizeof(struct inodo));
        struct inodo inodos[BLOCKSIZE / INODOSIZE];
        for (int i = SB.posPrimerInodoLibre; i < SB.cantInodosLibres; i++)
        {
            if (i % (BLOCKSIZE / INODOSIZE) == 0)
            {
                if (bread(SB.posPrimerBloqueAI + i / (BLOCKSIZE / INODOSIZE), inodos) == ERROR)
                {
                    perror("ERROR: ");
                    return ERROR;
                }
            }
            printf("%d ", inodos[i % (BLOCKSIZE / INODOSIZE)].punterosDirectos[0]);
        }
#endif

#if DEBUGN3
        printf("RESERVAMOS UN BLOQUE Y LUEGO LO LIBERAMOS\n");
        int nbloque = reservar_bloque();
        // Actualizamos datos del SB local
        if (bread(0, &SB) == ERROR)
        {
            perror("ERROR: ");
            return ERROR;
        }

        printf("Se ha reservado el bloque físico nº %i que era el %iº libre indicado por el MB\n", nbloque,
               ((nbloque - SB.posPrimerBloqueDatos)));

        printf("SB.cantBloquesLibres = %i\n",SB.cantBloquesLibres);
        if (liberar_bloque(nbloque) == ERROR)
        {
            perror("ERROR: ");
            return ERROR;
        }
        // Actualizamos los datos del SB local
        if (bread(0, &SB) == ERROR)
        {
            perror("ERROR: ");
            return ERROR;
        }
        printf("Liberamos ese bloque y después SB.cantBloquesLibres = %i\n", SB.cantBloquesLibres);
        // mostrar el MB (y así comprobar el funcionamiento de escribir_bit() y leer_bit()).
        printf("MAPA DE BITS CON BLOQUES DE METADATOS OCUPADOS\n");
        for (int i = 0; i < SB.totBloques; i++)
        {
            leer_bit(i);
        }

#endif
    }
    else
    {
        perror("ERROR");
        return ERROR;
    }
    if (bumount() == ERROR)
    {
        return ERROR;
    }

    return 0;
}
