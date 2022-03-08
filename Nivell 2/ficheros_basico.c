
#include "ficheros_basico.h"

int tamMB(unsigned int nbloques){
    int tam = (nbloques/8)/BLOCKSIZE; 
    if ((nbloques/8) % BLOCKSIZE != 0){
        return (tam + 1);
    }
    return tam;
}

int tamAI(unsigned int ninodos){
    int tam = (ninodos*INODOSIZE)/BLOCKSIZE;
    if ((ninodos*INODOSIZE) % BLOCKSIZE != 0){
        return (tam + 1);
    }
    return tam;
}

int initSB(unsigned int nbloques, unsigned int ninodos){
    struct superbloque SB;
    
    SB.posPrimerBloqueMB = posSB + tamSB;
    SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nbloques) - 1;
    SB.posPrimerBloqueAI = SB.posUltimoBloqueMB + 1;
    SB.posUltimoBloqueAI = SB.posPrimerBloqueAI + tamAI(ninodos) - 1;
    SB.posPrimerBloqueDatos = SB.posUltimoBloqueAI + 1;
    SB.posUltimoBloqueDatos = nbloques - 1;
    SB.posInodoRaiz = 0;
    SB.posPrimerInodoLibre = 0;
    SB.cantBloquesLibres = ninodos;
    SB.totBloques = nbloques;
    SB.totInodos = ninodos;

    bwrite(posSB,&SB);
    return 0;
}

