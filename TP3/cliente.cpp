#include <iostream>
#include <fstream>
#include <filesystem>
#include <unistd.h>
#include <string>

extern "C" {
    #include "myfs.h" // Cabecera generada por rpcgen
}

using namespace std;
namespace fs = filesystem;

void print_usage() {
    printf("Uso: ./cliente <ip_servidor> <mi_id_cliente> <operacion> [args...]\n");
    printf("Operaciones:\n");
    printf("  get <archivo> <directorio_destino>  - Descarga archivo protegiendo con lock\n");
    printf("  status                              - Consulta el estado de los locks en el servidor\n");
    printf("  test_lock <nombre_lock> <segundos>  - Adquiere un lock, espera N segundos y lo libera\n");
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        print_usage();
        return -1;
    }

    const char* ip_destino = argv[1];
    string client_id = argv[2];
    string operacion = argv[3];

    // Conexión RPC
    CLIENT *clnt = clnt_create(ip_destino, MYFS_PROG, MYFS_VERS, "tcp");
    if (clnt == nullptr) {
        clnt_pcreateerror(ip_destino);
        return -1;
    }

    if (operacion == "status") {
        char *result = nullptr;
        enum clnt_stat stat = get_server_status_1(nullptr, &result, clnt);
        if (stat == RPC_SUCCESS && result != nullptr) {
            printf("%s\n", result);
            xdr_free((xdrproc_t)xdr_wrapstring, (char *)&result);
        } else {
            clnt_perror(clnt, "Fallo GET_SERVER_STATUS");
        }
    } 
    else if (operacion == "get" || operacion == "test_lock") {
        string recurso = argv[4]; 
        
        LockRequest req;
        req.lock_name = (char*)recurso.c_str();
        req.client_id = (char*)client_id.c_str();
        int lock_res = 0;

        printf("[Cliente %s] Solicitando lock para '%s'...\n", client_id.c_str(), recurso.c_str());
        
        if (acquire_lock_1(&req, &lock_res, clnt) != RPC_SUCCESS) {
            clnt_perror(clnt, "Fallo ACQUIRE_LOCK");
            clnt_destroy(clnt);
            return -1;
        }

        if (lock_res == 1) {
            printf("[Cliente %s] Lock concedido inmediatamente.\n", client_id.c_str());
        } else {
            printf("[Cliente %s] Lock ocupado. En lista de espera...\n", client_id.c_str());
            while (true) {
                sleep(1); // Polling hasta que sea nuestro turno
                if (check_lock_1(&req, &lock_res, clnt) != RPC_SUCCESS) break;
                if (lock_res == 1) {
                    printf("[Cliente %s] ¡Turno alcanzado! Lock concedido.\n", client_id.c_str());
                    break;
                }
            }
        }

        if (operacion == "test_lock") {
            int espera = atoi(argv[5]);
            printf("[Cliente %s] Trabajando... Reteniendo el lock por %d segundos...\n", client_id.c_str(), espera);
            sleep(espera);
        } 
        else if (operacion == "get") {
            string directorio_destino = argv[5];
            FileRequest freq;
            freq.file_name = (char*)recurso.c_str();
            FileResponse fres;
            memset(&fres, 0, sizeof(fres));

            printf("[Cliente %s] Descargando archivo...\n", client_id.c_str());
            if (get_file_1(&freq, &fres, clnt) == RPC_SUCCESS) {
                if (fres.status == 1) {
                    fs::path p(directorio_destino);
                    if (fs::is_directory(p) || (!directorio_destino.empty() && (directorio_destino.back() == '/' || directorio_destino.back() == '\\'))) {
                        p /= fs::path(recurso).filename();
                    }
                    if (p.has_parent_path()) fs::create_directories(p.parent_path());

                    ofstream out(p, ios::binary);
                    if (out.is_open()) {
                        out.write(fres.data.data_val, fres.data.data_len);
                        out.close();
                        printf("[Cliente %s] Archivo guardado (%d bytes)\n", client_id.c_str(), fres.data.data_len);
                    }
                } else {
                    printf("[Cliente %s] Error: El archivo no existe en el servidor.\n", client_id.c_str());
                }
                clnt_freeres(clnt, (xdrproc_t)xdr_FileResponse, (caddr_t)&fres);
            }
        }

        // Liberación obligatoria del lock
        printf("[Cliente %s] Liberando lock '%s'...\n", client_id.c_str(), recurso.c_str());
        release_lock_1(&req, &lock_res, clnt);
    }

    clnt_destroy(clnt);
    return 0;
}