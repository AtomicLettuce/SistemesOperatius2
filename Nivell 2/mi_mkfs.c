#include "bloques.h"
#include "ficheros_basico.h"
int main(int argc, char **argv)
{
    if (argc == 3)
    {
        char *camino = argv[1];
        int nbloques = atoi(argv[2]);
        void *buf=malloc(BLOCKSIZE);
        int fd;

        memset(buf, 0, BLOCKSIZE);

    fd=(bmount(camino));
    if(fd!=-1){
            for (int i = 0; i < nbloques; i++)
            {
                bwrite(i,buf);
            }
            bumount(fd);
        }
    }else{
        perror("ERROR");
    }
}