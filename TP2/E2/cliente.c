/* cliente.c - Cliente del Ejercicio 2
 * Uso: ./cliente <host> pid <PID>
 *      ./cliente <host> nombre <nombre_proceso>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rpc/rpc.h>
#include "proceso.h"

int main(int argc, char *argv[])
{
    CLIENT *clnt;
    respuesta *r;

    if (argc != 4 || (strcmp(argv[2], "pid") && strcmp(argv[2], "nombre"))) {
        fprintf(stderr, "Uso: %s <host> pid <PID>\n     %s <host> nombre <nombre>\n",
                argv[0], argv[0]);
        return 2;
    }

    clnt = clnt_create(argv[1], TERMINAR_PROG, TERMINAR_VERS, "tcp");
    if (clnt == NULL) {
        clnt_pcreateerror(argv[1]);
        return 1;
    }

    if (strcmp(argv[2], "pid") == 0) {
        char *fin;
        int pid = (int)strtol(argv[3], &fin, 10);
        if (*fin != '\0') {
            fprintf(stderr, "PID invalido: %s\n", argv[3]);
            return 2;
        }
        printf("[CLIENTE] Solicitando finalizar PID %d en %s\n", pid, argv[1]);
        r = terminar_por_pid_1(&pid, clnt);
    } else {
        nombre_proc nombre = argv[3];
        printf("[CLIENTE] Solicitando finalizar proceso '%s' en %s\n", nombre, argv[1]);
        r = terminar_por_nombre_1(&nombre, clnt);
    }

    if (r == NULL) {
        clnt_perror(clnt, "Fallo la llamada RPC");
        clnt_destroy(clnt);
        return 1;
    }
    printf("[CLIENTE] Codigo: %d | Finalizados: %d | %s\n", r->codigo, r->cantidad, r->mensaje);
    clnt_destroy(clnt);
    return r->codigo == OK ? 0 : 3;
}
