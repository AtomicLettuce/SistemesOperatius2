#include "ficheros_basico.h"
#include "debugging.h"

int main(int argc, char **argv)
{
    struct superbloque SB;
    struct inodo inodos[BLOCKSIZE / INODOSIZE];

    if (argc == 2)
    {
        char *camino = argv[1];
    }
    if (bmount(camino) == -1)
    {
        return -1;
    }
    if (bread(0, &SB) != -1)
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
        printf("sizeof struc superbloque: %lu\n", sizeof(struct superbloque));
        printf("sizeof struc inodo is: %lu\n", sizeof(struct inodo));

#if DEBUGN2
        for (int i = SB.posPrimerInodoLibre; i < SB.cantInodosLibres; i++)
        {
            if (i % (BLOCKSIZE / INODOSIZE) == 0)
            {
                if (bread(SB.posPrimerBloqueAI + i / (BLOCKSIZE / INODOSIZE), inodos) == -1)
                {
                    perror("ERROR: ");
                    return -1;
                }
            }
            printf("%d", inodos[i % (BLOCKSIZE / INODOSIZE)].punterosDirectos[0]);
        }
#endif

#if DEBUGN3
        printf("RESERVAMOS UN BLOQUE Y LUEGO LO LIBERAMOS\n");
        int nbloque = reservar_bloque();
        printf("Se ha reservado el bloque físico nº %i que era el %iº libre indicado por el MB\n", nbloque,
               ((SB.posPrimerBloqueDatos - nbloque) + 1));

        printf("SB.cantBloquesLibres = %i\n",SB.cantBloquesLibres);


        liberar_bloque(nbloque);
        printf("Liberamos ese bloque y después SB.cantBloquesLibres = %i\n",SB.cantBloquesLibres);
                       

        // mostrar el MB (y así comprobar el funcionamiento de escribir_bit() y leer_bit()).
        printf("MAPA DE BITS CON BLOQUES DE METADATOS OCUPADOS\n") for (int i = 0; i < SB.totBloques; i++)
        {
            printf("%u", leer_bit(i));
        }
    }
#endif

    else
    {
        perror("ERROR");
        return -1;
    }
    return 0;
}
