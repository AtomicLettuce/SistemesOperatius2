#include "ficheros.h"

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        printf("Sintaxis: escribir <nombre_dispositivo><'$(cat fichero)'><diferentes_inodos>\n");
        printf("Offsets: 9000,209000,30725000,409605000,480000000\n");
        printf("Si diferentes_inodos = 0 se reserva un solo inodo para los offets\n");
        return -1;
    }
    struct STAT p_stat;
    char *camino = argv[1];
    int diferentes_inodos = atoi(argv[3]);
    long unsigned int offsets[] = {9000,209000,30725000,409605000,480000000};
    unsigned int ninodo, bytes_es;
    unsigned int nbytes = strlen(argv[2]);

    unsigned char buf[nbytes];
    memset(buf, 0, nbytes);
    memcpy(buf, argv[2], nbytes);

    if (bmount(camino) == -1)
    {
        perror("ERROR");
        return -1;
    }
    printf("longitud texto: %u\n", nbytes);
    if (diferentes_inodos == 0)
    {
        ninodo = reservar_inodo('f',6);
        for (int i = 0; i < 5; i++){
            printf("\nNº inodo reservado: %u\n", ninodo);
            printf("offset: %lu\n", offsets[i]);
            bytes_es = mi_write_f(ninodo, buf, offsets[i], nbytes);
            fprintf(stderr,"Bytes escritos: %u\n" ,bytes_es);
            mi_stat_f(ninodo, &p_stat);
            fprintf(stderr,"stat.tamEnBytesLog = %u\n",p_stat.tamEnBytesLog);
            fprintf(stderr,"stat.numBloquesOcupados = %u\n",p_stat.numBloquesOcupados);
        }
    } else {
        for (int i = 0; i < 5; i++){
            ninodo = reservar_inodo('f',6);
            printf("Nº inodo reservado: %u\n", ninodo);
            printf("offset: %lu\n", offsets[i]);
            bytes_es = mi_write_f(ninodo, buf, offsets[i], nbytes);
            printf("\nBytes escritos: %u\n" ,bytes_es);
            mi_stat_f(ninodo, &p_stat);
            printf("stat.tamEnBytesLog = %u\n",p_stat.tamEnBytesLog);
            printf("stat.numBloquesOcupados = %u\n",p_stat.numBloquesOcupados);
        }
    }
    if (bumount() == -1)
    {
        perror("ERROR");
        return -1;
    }
    return 0;
}
