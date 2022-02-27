#include "bloques.c"
int main(int argc, char **argv);

int main(int argc, char **argv)
{
    if (argc == 3)
    {
        char *camino = argv[1];
        int nbloques = atoi(argv[2]);
        void *buf;
        int fd;

        memset(buf; 0; BLOCKSIZE);

    fd=(bmount(camino);
    if(fd!=EXIT_FAILURE){
            for (int i = 0; i < nbloques; i++)
            {
                bwrite(i; buf);
            }
            bumount(fd);
        }
    }else{
        fprintf("ERROR: Número de parámetros erróneo");
    }
}
