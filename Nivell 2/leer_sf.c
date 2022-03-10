#include "ficheros_basico.h"

int main(int argc, char **argv){
    struc superbloque SB;
    if (argc == 2)
    {
        char *camino = argv[1];

    if(bmount(camino) == -1){
        return -1;
    }
    if(bread(0,&SB) != -1){
        printf("DATOS DEL SUPERBLOQUE\n");
        printf("posPrimerBloqueMB: %lu\n",SB.posPrimerBloqueMB);
        printf("posUltimoBloqueMB: %lu\n",SB.posUltimoBloqueMB);
        printf("posPrimerBloqueAI: %lu\n",SB.posPrimerBloqueAI);
        printf("posUltimoBloqueMB: %lu\n",SB.posUltimoBloqueAI);
        printf("posPrimerBloqueDatos: %lu\n",SB.posPrimerBloqueDatos);
        printf("posUltimoBloqueDatos: %lu\n",SB.posUltimoBloqueDatos);
        printf("posInodoRaiz: %lu\n",SB.posInodoRaiz);
        printf("posPrimerInodoLibre: %lu\n",SB.posPrimerInodoLibre);
        printf("cantBloquesLibres: %lu\n",SB.cantBloquesLibres);
        printf("cantInodosLibres: %lu\n",SB.cantInodosLibres);
        printf("totBloques: %lu\n",SB.totBloques);
        printf("totInodos: %lu\n\n",SB.totInodos);
        printf("sizeof struc superbloque: %lu\n",sizeof(struc superbloque));
        printf("sizeof struc inodo is: %lu\n",sizeof(struc inodo));
        printf("RECORRIDO LISTA ENLAZADA DE INODOS LIBRES")
        for(int i = SB.posPrimerInodoLibre; i < SB.cantInodosLibres; i++){
            printf("%lu",i.punterosDirectos[0]);
        }   
    }else{
        perror("ERROR");
        return -1;
    }
    return 0;
}
