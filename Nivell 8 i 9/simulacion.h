#include <sys/wait.h>
#include <signal.h>
#include "directorios.h"

#define NUMPROCESOS 100
#define NUMESCRITURAS 50
#define REGMAX  (((12+256+256*256+256*256*256)-1)*BLOCKSIZE)

struct REGISTRO{

    time_t fecha;
    pid_t pid;
    int nEscritura;
    int nRegistro;
};