#include "ficheros_basico.h"

int main(int argc, char **argv){
    
    if (argc == 2)
    {
        char *camino = argv[1];

    if(bmount(camino) == -1){
        return -1;
    }
    if(bread(0,camino) != -1){
        printf("DATOS DEL SUPERBLOQUE\n");
        printf("posPrimerBloqueMB: %lu\n",camino.posPrimerBloqueMB);
        printf("posUltimoBloqueMB: %lu\n",posUltimoBloqueMB);
        printf("posPrimerBloqueAI: %lu\n",posPrimerBloqueAI);
        printf("posUltimoBloqueMB: %lu\n",posUltimoBloqueAI);
        printf("posPrimerBloqueDatos: %lu\n",posPrimerBloqueDatos);
        printf("posUltimoBloqueDatos: %lu\n",posUltimoBloqueDatos);
        printf("posInodoRaiz: %lu\n",posInodoRaiz);
        printf("posPrimerInodoLibre: %lu\n",posPrimerInodoLibre);
        printf("cantBloquesLibres: %lu\n",cantBloquesLibres);
        printf("cantInodosLibres: %lu\n",cantInodosLibres);
        printf("totBloques: %lu\n",totBloques);
        printf("totInodos: %lu\n\n",totInodos);
        printf("sizeof struc superbloque: %lu\n",sizeof(struc superbloque));
        printf("sizeof struc inodo is: %lu\n",sizeof(struc inodo));
        printf("RECORRIDO LISTA ENLAZADA DE INODOS LIBRES")
        for(int i = posPrimerInodoLibre; i < cantInodosLibres; i++){
            printf("%lu",i.punterosDirectos[0]);
        }   
    }else{
        perror("ERROR");
        return -1;
    }
    return 0;
}
