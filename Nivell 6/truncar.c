#include "ficheros_basico.h"
#include "ficheros.h"
int main(int argc, char **argv)
{
    // Comprobamos que se haya introducido el comando correctamente
    if (argc == 4)
    {
        char *camino = argv[1];
        int ninodo = atoi(argv[2]);
        int nbytes = atoi(argv[3]);

    fd=(bmount(camino));
    if(fd!=-1){
        // Inicializamos los bloques del dispositivo todo a 0
            if(nbytes == 0) {
                liberar_inodo(ninodo);
            } else {
                mi_truncar_f(ninodo,nbytes);
            }
            if(bumount(fd) == -1){
                perror("ERROR");
            }
        }
    }else{
        perror("ERROR");
    }
}
