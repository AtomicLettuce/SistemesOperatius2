
#include "ficheros_basico.h"

// DEMANAR SI AIXÒ HAURIA D'ESTAR AQUI !!!!!!!!!
struct superbloque SB;

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

int initMB(){
    // Declaramos un búfer tan grande como un bloque
    unsigned char* buf=malloc(BLOCKSIZE);
    // Contenido de buffer = todo 0s
    memset(buf,0,BLOCKSIZE);

    // Ponemos todo el MB a 0s
    for(int i=SB.posPrimerBloqueMB;i<=SB.posUltimoBloqueMB;i++){
        bwrite(i,buf);
    }

    // DEMANAR QUE HEM DE RETORNAR !!!!!!!!!!!!
    // I SI AIXO ES CORRECTE
    return 0;
}

int initSB(unsigned int nbloques, unsigned int ninodos){    
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

