
#include <iostream>
using namespace std;
// Envolvemos el header generado por rpcgen para compatibilidad con C++
extern "C" {
    #include "./calculadora.h"
}

// Las variables de retorno deben ser estáticas o asignadas dinámicamente
static int resultado;


// rpcgen añade el sufijo _1_svc (por la versión 1 y svc de service)
extern "C" int * suma_1_svc(operandos *argp, struct svc_req *rqstp) {
    printf("Petición de Suma: %s + %s ",argp->a,argp->b);
    resultado = argp->a + argp->b;
    return &resultado;
}

extern "C" int * resta_1_svc(operandos *argp, struct svc_req *rqstp) {
    printf("Petición de Resta: %s + %s ",argp->a,argp->b);
    resultado = argp->a - argp->b;
    return &resultado;
}