// Uso: ./cliente <host>
#include <iostream>
extern "C" {
    #include "calculadora.h"
}
using namespace std;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cerr << "Uso: " << argv[0] << " <host>" << endl;
        return 1;
    }

    CLIENT *clnt = clnt_create(argv[1], CALCULADORA_PROG, CALCULADORA_VERS, "tcp");
    if (clnt == nullptr) {
        clnt_pcreateerror(argv[1]);
        return 1;
    }

    int opcion;
    do {
        cout << "\n=== Calculadora RPC (servidor: " << argv[1] << ") ===\n"
             << "1) Suma\n2) Resta\n3) Multiplicacion\n4) Division\n0) Salir\n"
             << "Opcion: ";
        if (!(cin >> opcion)) break;
        if (opcion == 0) break;
        if (opcion < 1 || opcion > 4) {
            cout << "Opcion invalida\n";
            continue;
        }

        operandos args;
        cout << "Primer operando: ";
        if (!(cin >> args.a)) break;
        cout << "Segundo operando: ";
        if (!(cin >> args.b)) break;

        respuesta *r = nullptr;
        const char *simbolo = "";
        switch (opcion) {
            case 1: r = suma_1(&args, clnt);           simbolo = "+"; break;
            case 2: r = resta_1(&args, clnt);          simbolo = "-"; break;
            case 3: r = multiplicacion_1(&args, clnt); simbolo = "*"; break;
            case 4: r = division_1(&args, clnt);       simbolo = "/"; break;
        }

        if (r == nullptr) {
            clnt_perror(clnt, "Fallo la llamada RPC");
        } else if (r->error) {
            cout << "[CLIENTE] Error: division por cero" << endl;
        } else {
            cout << "[CLIENTE] " << args.a << " " << simbolo << " " << args.b
                 << " = " << r->valor << endl;
        }
    } while (true);

    clnt_destroy(clnt);
    return 0;
}
