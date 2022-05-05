#include "bloques.h"
#include "debugging.h"
int descriptor;

int bmount(const char *camino){
    umask(000);
    if((descriptor=open(camino,O_RDWR|O_CREAT,0666)) == -1){
        perror("ERROR: ");
        return ERROR;
    }
    return descriptor;
}
int bumount(){
    if(close(descriptor) == -1){
        perror("ERROR: ");
        return ERROR;
    }
    return 0;
}
int bwrite(unsigned int nbloque, const void *buf){
    int rw;
    if(lseek(descriptor,nbloque*BLOCKSIZE,SEEK_SET) == -1){
        perror("ERROR: ");
        return ERROR;
    }
    if((rw=write(descriptor,buf,BLOCKSIZE)) == -1){
        perror("ERROR: ");
        return ERROR;
    }
    return rw;
}
int bread(unsigned int nbloque, void *buf){
    int wr;
    if(lseek(descriptor,nbloque*BLOCKSIZE,SEEK_SET) == -1){
        perror("ERROR: ");
        return ERROR;
    }
    if((wr=read(descriptor,buf,BLOCKSIZE)) == -1){
        perror("ERROR: ");
        return ERROR;
    }
    return wr;
}
