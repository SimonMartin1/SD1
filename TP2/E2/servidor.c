/* servidor.c - Implementacion de los procedimientos remotos (Ejercicio 2) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include "proceso.h"

static respuesta res;           /* resultado estatico devuelto a rpcgen */
static char msg[MAX_MSG];

/* Un proceso zombie (terminado, pendiente de wait del padre) cuenta como finalizado */
static int esta_vivo(pid_t pid)
{
    char ruta[64], linea[512];
    FILE *f;
    char *p;

    snprintf(ruta, sizeof ruta, "/proc/%d/stat", (int)pid);
    f = fopen(ruta, "r");
    if (!f) return 0;
    if (!fgets(linea, sizeof linea, f)) { fclose(f); return 0; }
    fclose(f);
    p = strrchr(linea, ')');            /* el estado va despues del nombre "(comm)" */
    return !(p && p[2] == 'Z');
}

static int existe(pid_t pid)
{
    return kill(pid, 0) == 0 || errno == EPERM;
}

/* Envia SIGTERM y espera hasta ~2 s a que el proceso termine. Devuelve un 'estado'. */
static estado terminar(pid_t pid, char *detalle, size_t n)
{
    struct timespec t = {0, 100 * 1000 * 1000};
    int i;

    if (pid <= 1 || pid == getpid()) {
        snprintf(detalle, n, "PID %d protegido: no se permite finalizarlo", (int)pid);
        return PROHIBIDO;
    }
    if (!existe(pid)) {
        snprintf(detalle, n, "El proceso %d no existe", (int)pid);
        return NO_EXISTE;
    }
    if (kill(pid, SIGTERM) != 0) {
        if (errno == EPERM) {
            snprintf(detalle, n, "Permiso denegado para finalizar el proceso %d", (int)pid);
            return SIN_PERMISO;
        }
        if (errno == ESRCH) {
            snprintf(detalle, n, "El proceso %d no existe", (int)pid);
            return NO_EXISTE;
        }
        snprintf(detalle, n, "Error al finalizar %d: %s", (int)pid, strerror(errno));
        return ERROR;
    }
    for (i = 0; i < 20; i++) {
        if (!esta_vivo(pid)) {
            snprintf(detalle, n, "Proceso %d finalizado correctamente", (int)pid);
            return OK;
        }
        nanosleep(&t, NULL);
    }
    snprintf(detalle, n, "Se envio SIGTERM al proceso %d pero sigue en ejecucion", (int)pid);
    return NO_FINALIZO;
}

static void preparar(void)
{
    free(res.mensaje);
    res.mensaje = NULL;
    res.codigo = ERROR;
    res.cantidad = 0;
}

static void responder(estado e, int cant, const char *texto)
{
    res.codigo = e;
    res.cantidad = cant;
    res.mensaje = strdup(texto);
    printf("[SERVIDOR] Resultado: %s\n", texto);
    fflush(stdout);
}

respuesta *terminar_por_pid_1_svc(int *pid, struct svc_req *rqstp)
{
    preparar();
    printf("[SERVIDOR] Solicitud: finalizar PID %d\n", *pid);
    estado e = terminar((pid_t)*pid, msg, sizeof msg);
    responder(e, e == OK ? 1 : 0, msg);
    return &res;
}

respuesta *terminar_por_nombre_1_svc(nombre_proc *nombre, struct svc_req *rqstp)
{
    DIR *d;
    struct dirent *de;
    int encontrados = 0, finalizados = 0;
    estado ultimo = NO_EXISTE;
    char ruta[64], comm[256], detalle[MAX_MSG];

    preparar();
    printf("[SERVIDOR] Solicitud: finalizar proceso de nombre '%s'\n", *nombre);

    d = opendir("/proc");
    if (!d) {
        responder(ERROR, 0, "No se pudo leer /proc");
        return &res;
    }
    while ((de = readdir(d)) != NULL) {
        FILE *f;
        pid_t pid;
        if (!isdigit((unsigned char)de->d_name[0])) continue;
        pid = (pid_t)atoi(de->d_name);
        if (pid <= 1 || pid == getpid()) continue;
        snprintf(ruta, sizeof ruta, "/proc/%d/comm", (int)pid);
        f = fopen(ruta, "r");
        if (!f) continue;
        if (fgets(comm, sizeof comm, f)) {
            comm[strcspn(comm, "\n")] = '\0';
            if (strcmp(comm, *nombre) == 0 && esta_vivo(pid)) {
                estado e;
                encontrados++;
                e = terminar(pid, detalle, sizeof detalle);
                printf("[SERVIDOR]   %s\n", detalle);
                if (e == OK) finalizados++; else ultimo = e;
            }
        }
        fclose(f);
    }
    closedir(d);

    if (encontrados == 0)
        responder(NO_EXISTE, 0, "No existe ningun proceso con ese nombre");
    else if (finalizados == encontrados) {
        snprintf(msg, sizeof msg, "%d proceso(s) '%s' finalizado(s) correctamente",
                 finalizados, *nombre);
        responder(OK, finalizados, msg);
    } else {
        snprintf(msg, sizeof msg, "Se finalizaron %d de %d proceso(s) '%s' (ultimo error: codigo %d)",
                 finalizados, encontrados, *nombre, ultimo);
        responder(ultimo, finalizados, msg);
    }
    return &res;
}
