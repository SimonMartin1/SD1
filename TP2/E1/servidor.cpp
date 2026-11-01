// servidor.cpp - Implementacion de los procedimientos remotos
#include <cstdio>
extern "C" {
    #include "calculadora.h"
}

static respuesta res;   // el valor devuelto debe ser estatico

extern "C" respuesta *suma_1_svc(operandos *argp, struct svc_req *rqstp) {
    res.error = 0;
    res.valor = argp->a + argp->b;
    printf("[SERVIDOR] Solicitud SUMA: %g + %g = %g\n", argp->a, argp->b, res.valor);
    fflush(stdout);
    return &res;
}

extern "C" respuesta *resta_1_svc(operandos *argp, struct svc_req *rqstp) {
    res.error = 0;
    res.valor = argp->a - argp->b;
    printf("[SERVIDOR] Solicitud RESTA: %g - %g = %g\n", argp->a, argp->b, res.valor);
    fflush(stdout);
    return &res;
}

extern "C" respuesta *multiplicacion_1_svc(operandos *argp, struct svc_req *rqstp) {
    res.error = 0;
    res.valor = argp->a * argp->b;
    printf("[SERVIDOR] Solicitud MULTIPLICACION: %g * %g = %g\n", argp->a, argp->b, res.valor);
    fflush(stdout);
    return &res;
}

extern "C" respuesta *division_1_svc(operandos *argp, struct svc_req *rqstp) {
    if (argp->b == 0) {
        res.error = 1;
        res.valor = 0;
        printf("[SERVIDOR] Solicitud DIVISION: %g / %g -> ERROR: division por cero\n", argp->a, argp->b);
    } else {
        res.error = 0;
        res.valor = argp->a / argp->b;
        printf("[SERVIDOR] Solicitud DIVISION: %g / %g = %g\n", argp->a, argp->b, res.valor);
    }
    fflush(stdout);
    return &res;
}
