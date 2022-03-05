#include "bloques.h"

int descriptor;

int bmount(const char *camino){
    if((descriptor=open(camino,O_RDWR|O_CREAT)) == -1){
        perror("Errorsito");
        return -1;
    }
    return descriptor;
}
int bumount(){
    if(close(descriptor) == -1){
        perror("Error");
        return -1;
    }
    return 0;
}
int bwrite(unsigned int nbloque, const void *buf){
    int rw;
    if(lseek(descriptor,nbloque*BLOCKSIZE,SEEK_SET) == -1){
        perror("Error");
        return -1;
    }
    if((rw=write(descriptor,buf,BLOCKSIZE)) == -1){
        perror("Error");
        return -1;
    }
    return rw;
}
int bread(unsigned int nbloque, void *buf){
    int wr;
    if(lseek(descriptor,nbloque*BLOCKSIZE,SEEK_SET) == -1){
        perror("Error");
        return -1;
    }
    if((wr=read(descriptor,buf,BLOCKSIZE)) == -1){
        perror("Error");
        return -1;
    }
    return wr;
}
