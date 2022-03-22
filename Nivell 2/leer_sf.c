#include "ficheros_basico.h"

int main(int argc, char **argv)
{
    struct superbloque SB;
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    char *camino = argv[1];
    if (argc != 2)
    {
        printf("ERROR: ");
    }
    if (bmount(camino) == -1)
    {
        return -1;
    }
    if (bread(0, &SB) != -1)
    {
        printf("DATOS DEL SUPERBLOQUE\n");
        printf("posPrimerBloqueMB: %d\n", SB.posPrimerBloqueMB);
        printf("posUltimoBloqueMB: %d\n", SB.posUltimoBloqueMB);
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
        printf("sizeof struc superbloque: %lu\n", sizeof(struct superbloque));
        printf("sizeof struc inodo is: %lu\n", sizeof(struct inodo));
        printf("RECORRIDO LISTA ENLAZADA DE INODOS LIBRES\n");

        for (int i = SB.posPrimerInodoLibre; i < SB.cantInodosLibres; i++)
        {
            if (i % (BLOCKSIZE / INODOSIZE) == 0)
            {
                if (bread(SB.posPrimerBloqueAI + i/(BLOCKSIZE/INODOSIZE), inodos) == -1)
                {
                    perror("ERROR: ");
                    return -1;
                }
            }
            printf("%d", inodos[i % (BLOCKSIZE / INODOSIZE)].punterosDirectos[0]);
        }
    }
    else
    {
        perror("ERROR");
        return -1;
    }
    if (bumount() == -1)
    {
        return -1;
    }
    return 0;
}
