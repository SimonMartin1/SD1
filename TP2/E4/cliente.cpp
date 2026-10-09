// cliente.cpp
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio> 
using namespace std;

extern "C" {
    #include "calculadora.h"
}

int main(int argc, char *argv[]) {
    CLIENT *clnt;
    int *resultado;

    if (argc != 3) {
        printf("Uso: %s <host> \"<oracion>\"\n", argv[0]);
        return 1;
    }

    char* host = argv[1];
    char* texto_arg = argv[2]; 

    clnt = clnt_create(host, CALCULADORA_PROG, CALCULADORA_VERS, "udp");
    if (clnt == nullptr) {
        clnt_pcreateerror(host);
        return 1;
    }

    resultado = cant_carac_e_1(&texto_arg, clnt);
    if (resultado == nullptr) {
        clnt_perror(clnt, "Fallo en la llamada");
    } else {
        printf("Caracteres con espacios: %d\n", *resultado);
    }

    resultado = cant_carac_se_1(&texto_arg, clnt);
    if (resultado == nullptr) {
        clnt_perror(clnt, "Fallo en la llamada");
    } else {
        printf("Caracteres sin espacios: %d\n", *resultado);
    }

    resultado = cant_palabras_1(&texto_arg, clnt);
    if (resultado == nullptr) {
        clnt_perror(clnt, "Fallo en la llamada");
    } else {
        printf("Cantidad de palabras: %d\n", *resultado);
    }

    clnt_destroy(clnt);
    return 0;
}