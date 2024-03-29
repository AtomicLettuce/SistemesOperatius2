<<<<<<< HEAD
#include "ficheros.h"
int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Sintaxis: leer <nombre_dispositivo><ninodos>\n");
        return -1;
    } 
    struct STAT p_stat;
    char string[128];
    char *camino = argv[1];
    unsigned int ninodo = atoi(argv[2]);
    unsigned int leidos, offset = 0;
    unsigned int t_leidos = 0;
    int tambuffer = 1500;
    void *buffer_texto = malloc(tambuffer);
    memset(buffer_texto, 0, tambuffer);
    if (bmount(camino) == -1)
    {
        return -1;
    }
    leidos = mi_read_f(ninodo, buffer_texto, offset, tambuffer);
    while (leidos > 0)
    {
        write(1, buffer_texto ,leidos);
        offset += tambuffer;
        memset(buffer_texto, 0, tambuffer);
        t_leidos  += leidos;
        leidos = mi_read_f(ninodo, buffer_texto, offset, tambuffer);
    }
    sprintf(string, "\ntotal_leidos %d\n", t_leidos);
    write(2, string, strlen(string));
    mi_stat_f(ninodo, &p_stat);
    sprintf(string, "tamEnBytesLog %d\n", p_stat.tamEnBytesLog);
    write(2, string, strlen(string));    
    if (bumount() == -1)
    {
        return -1;
    }
=======
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
               ((nbloque - SB.posPrimerBloqueDatos) + 1));

        printf("SB.cantBloquesLibres = %i\n", SB.cantBloquesLibres);
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

        // Imprimir los datos del directorio raíz
        struct inodo dirRaiz;
        leer_inodo(SB.posInodoRaiz, &dirRaiz);

        printf("\n\nDATOS DEL DIRECTORIO RAIZ\n");
        printf("tipo: %c\n", dirRaiz.tipo);
        printf("permisos: %u\n", dirRaiz.permisos);
        ts = localtime(&dirRaiz.atime);
        strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
        ts = localtime(&dirRaiz.mtime);
        strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
        ts = localtime(&dirRaiz.ctime);
        strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
        printf("ID: %d ATIME: %s MTIME: %s CTIME: %s\n", SB.posInodoRaiz, atime, mtime, ctime);
        printf("nlinks: %u\n", dirRaiz.nlinks);
        printf("tamEnBytesLog: %u\n", dirRaiz.tamEnBytesLog);
        printf("numBloquesOcupados: %u\n", dirRaiz.numBloquesOcupados);
#endif

#if DEBUGN41
        struct tm *ts;
        char atime[80];
        char mtime[80];
        char ctime[80];
        struct inodo inodoN4;
        unsigned int ninodoN4 = reservar_inodo('f', 6);

        if (ninodoN4 < 0)
        {
            perror("ERROR: ");
            return ERROR;
        }
        if (leer_inodo(ninodoN4, &inodoN4) == ERROR)
        {
            perror("ERROR: ");
            return ERROR;
        }

        printf("INODO %u. TRADUCCION DE LOS BLOQUES LOGICOS 8, 204, 30.004, 400.004 Y 468.750\n", ninodoN4);
        traducir_bloque_inodo(ninodoN4, 8, 1);
        traducir_bloque_inodo(ninodoN4, 204, 1);
        traducir_bloque_inodo(ninodoN4, 30004, 1);
        traducir_bloque_inodo(ninodoN4, 400004, 1);
        traducir_bloque_inodo(ninodoN4, 468750, 1);
        if (leer_inodo(ninodoN4, &inodoN4) == ERROR)
        {
            perror("ERROR: ");
            return ERROR;
        }
        printf("\n\nDATOS DEL INODO RESERVADO %u\n", ninodoN4);
        printf("tipo: %c\n", inodoN4.tipo);
        printf("permisos: %u\n", inodoN4.permisos);
        ts = localtime(&inodoN4.atime);
        strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
        ts = localtime(&inodoN4.mtime);
        strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
        ts = localtime(&inodoN4.ctime);
        strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
        printf("atime: %s\nmtime: %s\nctime: %s\n", atime, mtime, ctime);
        printf("nlinks: %u\n", inodoN4.nlinks);
        printf("tamEnBytesLog: %u\n", inodoN4.tamEnBytesLog);
        printf("numBloquesOcupados: %u\n", inodoN4.numBloquesOcupados);

        if (bread(0, &SB) == ERROR)
        {
            perror("ERROR: ");
            return ERROR;
        }
        printf("\n\nSB.posPrimerInodoLibre = %u\n",SB.posPrimerInodoLibre);
        

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

>>>>>>> 74381f3f199b5c9b7a92837d77ddf84c0562cf2f
    return 0;
}
