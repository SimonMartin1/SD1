// cliente.cpp
#include <iostream>
#include <cstdlib>
using namespace std;
// Envolvemos el header generado por rpcgen para compatibilidad con C++
extern "C" {
    #include "calculadora.h"
}

int main(int argc, char *argv[]) {
    CLIENT *clnt;
    int *resultado;
    operandos args;

    if (argc != 4) {
        std::cerr << "Uso: " << argv[0] << " <host> <num1> <num2>" << std::endl;
        return 1;
    }

    char* host = argv[1];
    args.a = std::atoi(argv[2]);
    args.b = std::atoi(argv[3]);

    // 1. Crear el manejador del cliente (usando UDP en este caso)
    clnt = clnt_create(host, CALCULADORA_PROG, CALCULADORA_VERS, "udp");
    if (clnt == nullptr) {
        clnt_pcreateerror(host);
        return 1;
    }

    // 2. Llamar a la función SUMA remota
    // rpcgen añade el sufijo _1 (por la versión 1)
    resultado = suma_1(&args, clnt);
    if (resultado == nullptr) {
        clnt_perror(clnt, "Fallo en la llamada a SUMA");
    } else {
        std::cout << "Resultado SUMA: " << *resultado << std::endl;
    }

    // 3. Llamar a la función RESTA remota
    resultado = resta_1(&args, clnt);
    if (resultado == nullptr) {
        clnt_perror(clnt, "Fallo en la llamada a RESTA");
    } else {
        std::cout << "Resultado RESTA: " << *resultado << std::endl;
    }

    // 4. Limpiar recursos
    clnt_destroy(clnt);
    return 0;
}

// docker build -t rpc-dev-env .
// docker run -it --rm -v ${PWD}:/app rpc-dev-env
// rpcgen -C calculadora.x
// g++ -I/usr/include/tirpc servidor.cpp calculadora_svc.c calculadora_xdr.c -o servidor -ltirpc
// g++ -I/usr/include/tirpc cliente.cpp calculadora_clnt.c calculadora_xdr.c -o cliente -ltirpc
// ./servidor &
// ./cliente localhost 15 7