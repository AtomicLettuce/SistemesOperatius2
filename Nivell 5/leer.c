#include "ficheros.h"
int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Sintaxis: leer <nombre_dispositivo><ninodos>\n");
        return -1;
    } 
    struct superbloque SB;
    char string[128];
    char *camino = argv[1];
    int ninodo = atoi(argv[2]);
    int leidos, offset = 0;
    void *buffer_texto = malloc(BLOCKSIZE);
    memset(buffer_texto, 0, BLOCKSIZE);
    if (bmount(camino) == -1)
    {
        return -1;
    }
    if (bread(0, &SB) != -1)
    {
        leidos = mi_read_f(ninodo, buffer_texto, offset, BLOCKSIZE);
        printf("\n%d\n",leidos);
        //while (leidos > 0)
        //{
        //    write(1,buffer_texto,leidos);
        //    offset = offset + BLOCKSIZE;
        //    memset(buffer_texto, 0, BLOCKSIZE);
        //    leidos = mi_read_f(ninodo, buffer_texto, offset, BLOCKSIZE);
        //}
        sprintf(string, "bytes leídos %d\n", leidos);
        write(2, string, strlen(string));
    }else
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