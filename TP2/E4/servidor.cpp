// servidor.cpp
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm> 
using namespace std;

extern "C" {
    #include "./calculadora.h"
}

static int resultado;

extern "C" int * cant_carac_e_1_svc(cadena *argp, struct svc_req *rqstp) {
    printf("Petición recibida: %s\n", *argp); 
    
    string str(*argp); 
    resultado = str.length();
    return &resultado;
}

extern "C" int * cant_carac_se_1_svc(cadena *argp, struct svc_req *rqstp) {
    printf("Petición recibida: %s\n", *argp);
    string str(*argp);
    
    resultado = count_if(str.begin(), str.end(), [](unsigned char c) {return !isspace(c);});
    return &resultado;
}

extern "C" int * cant_palabras_1_svc(cadena *argp, struct svc_req *rqstp) {
    printf("Petición recibida: %s\n", *argp);
    string str(*argp);
    stringstream stream(str);
    string palabra;
    
    resultado = 0;
    while (stream >> palabra) {
        resultado++;
    }
    return &resultado;
}