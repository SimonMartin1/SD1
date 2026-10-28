#include <iostream>
#include <fstream>
#include <filesystem>
#include <unistd.h>
#include <string>
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <cstring>

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

bool parse_seconds(const char* value, unsigned int& seconds) {
    char* end = nullptr;
    errno = 0;
    long parsed = strtol(value, &end, 10);
    if (errno != 0 || end == value || *end != '\0' || parsed < 0 || parsed > UINT_MAX) {
        return false;
    }
    seconds = static_cast<unsigned int>(parsed);
    return true;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        print_usage();
        return 2;
    }

    const char* ip_destino = argv[1];
    string client_id = argv[2];
    string operacion = argv[3];
    bool is_status = operacion == "status";
    bool is_get = operacion == "get";
    bool is_test_lock = operacion == "test_lock";
    if ((!is_status && !is_get && !is_test_lock) ||
        (is_status && argc != 4) ||
        (is_get && argc != 6) ||
        (is_test_lock && argc != 6)) {
        print_usage();
        return 2;
    }

    if (client_id.empty() || client_id.size() > MYFS_MAX_NAME_LENGTH) {
        fprintf(stderr, "Error: el ID del cliente debe tener entre 1 y %d caracteres.\n",
                MYFS_MAX_NAME_LENGTH);
        return 2;
    }

    if (!is_status && (string(argv[4]).empty() || strlen(argv[4]) > MYFS_MAX_NAME_LENGTH)) {
        fprintf(stderr, "Error: el nombre del recurso debe tener entre 1 y %d caracteres.\n",
                MYFS_MAX_NAME_LENGTH);
        return 2;
    }

    unsigned int wait_seconds = 0;
    if (is_test_lock && !parse_seconds(argv[5], wait_seconds)) {
        fprintf(stderr, "Error: los segundos deben ser un entero no negativo.\n");
        return 2;
    }

    // Conexión RPC
    CLIENT *clnt = clnt_create(ip_destino, MYFS_PROG, MYFS_VERS, "tcp");
    if (clnt == nullptr) {
        clnt_pcreateerror(ip_destino);
        return 1;
    }

    int exit_code = 0;
    if (is_status) {
        char *result = nullptr;
        enum clnt_stat stat = get_server_status_1(nullptr, &result, clnt);
        if (stat == RPC_SUCCESS && result != nullptr) {
            printf("%s\n", result);
            clnt_freeres(clnt, (xdrproc_t)xdr_wrapstring, (caddr_t)&result);
        } else if (stat == RPC_SUCCESS) {
            fprintf(stderr, "Error: el servidor devolvio un estado vacio.\n");
            exit_code = 1;
        } else {
            clnt_perror(clnt, "Fallo GET_SERVER_STATUS");
            exit_code = 1;
        }
    } 
    else {
        string recurso = argv[4];
        
        LockRequest req;
        req.lock_name = const_cast<char*>(recurso.c_str());
        req.client_id = const_cast<char*>(client_id.c_str());
        int lock_res = 0;

        printf("[Cliente %s] Solicitando lock para '%s'...\n", client_id.c_str(), recurso.c_str());
        
        if (acquire_lock_1(&req, &lock_res, clnt) != RPC_SUCCESS) {
            clnt_perror(clnt, "Fallo ACQUIRE_LOCK");
            exit_code = 1;
        }
        bool lock_acquired = exit_code == 0 && lock_res == LOCK_GRANTED;
        if (lock_acquired) {
            printf("[Cliente %s] Lock concedido inmediatamente.\n", client_id.c_str());
        } else if (exit_code == 0) {
            printf("[Cliente %s] Lock ocupado. En lista de espera...\n", client_id.c_str());
            while (!lock_acquired) {
                sleep(1); // Polling hasta que sea nuestro turno
                if (check_lock_1(&req, &lock_res, clnt) != RPC_SUCCESS) {
                    clnt_perror(clnt, "Fallo CHECK_LOCK");
                    exit_code = 1;
                    break;
                }
                if (lock_res == LOCK_GRANTED) {
                    lock_acquired = true;
                    printf("[Cliente %s] ¡Turno alcanzado! Lock concedido.\n", client_id.c_str());
                }
            }
        }

        if (lock_acquired && is_test_lock) {
            printf("[Cliente %s] Trabajando... Reteniendo el lock por %u segundos...\n",
                   client_id.c_str(), wait_seconds);
            sleep(wait_seconds);
        } else if (lock_acquired && is_get) {
            string directorio_destino = argv[5];
            FileRequest freq;
            freq.file_name = const_cast<char*>(recurso.c_str());
            FileResponse fres;
            memset(&fres, 0, sizeof(fres));

            printf("[Cliente %s] Descargando archivo...\n", client_id.c_str());
            if (get_file_1(&freq, &fres, clnt) == RPC_SUCCESS) {
                if (fres.status == FILE_OK) {
                    fs::path p(directorio_destino);
                    std::error_code fs_error;
                    bool destination_is_directory = fs::is_directory(p, fs_error);
                    if (fs_error) {
                        fprintf(stderr, "Error al consultar el destino: %s\n",
                                fs_error.message().c_str());
                        exit_code = 1;
                    } else if (destination_is_directory ||
                               (!directorio_destino.empty() &&
                                (directorio_destino.back() == '/' ||
                                 directorio_destino.back() == '\\'))) {
                        p /= fs::path(recurso).filename();
                    }
                    if (exit_code == 0 && p.has_parent_path()) {
                        fs::create_directories(p.parent_path(), fs_error);
                    }

                    if (exit_code == 0 && fs_error) {
                        fprintf(stderr, "Error al crear el directorio de destino: %s\n",
                                fs_error.message().c_str());
                        exit_code = 1;
                    }
                    if (exit_code == 0) {
                        ofstream out(p, ios::binary);
                        if (!out.is_open()) {
                            fprintf(stderr, "Error: no se pudo abrir el archivo de destino.\n");
                            exit_code = 1;
                        } else {
                            if (fres.data.data_len > 0) {
                                out.write(fres.data.data_val,
                                          static_cast<streamsize>(fres.data.data_len));
                            }
                            if (!out) {
                                fprintf(stderr, "Error al escribir el archivo de destino.\n");
                                exit_code = 1;
                            } else {
                                out.close();
                                if (!out) {
                                    fprintf(stderr, "Error al cerrar el archivo de destino.\n");
                                    exit_code = 1;
                                }
                            }
                            if (exit_code == 0) {
                                printf("[Cliente %s] Archivo guardado (%u bytes)\n",
                                       client_id.c_str(), fres.data.data_len);
                            }
                        }
                    }
                } else {
                    fprintf(stderr, "Error: el archivo no existe o no se pudo leer en el servidor.\n");
                    exit_code = 1;
                }
                clnt_freeres(clnt, (xdrproc_t)xdr_FileResponse, (caddr_t)&fres);
            } else {
                clnt_perror(clnt, "Fallo GET_FILE");
                exit_code = 1;
            }
        }

        if (lock_acquired) {
            printf("[Cliente %s] Liberando lock '%s'...\n", client_id.c_str(), recurso.c_str());
            if (release_lock_1(&req, &lock_res, clnt) != RPC_SUCCESS) {
                clnt_perror(clnt, "Fallo RELEASE_LOCK");
                exit_code = 1;
            } else if (lock_res != LOCK_GRANTED) {
                fprintf(stderr, "Error: el servidor no libero el lock.\n");
                exit_code = 1;
            }
        } else if (exit_code != 0 && lock_res == LOCK_WAITING) {
            if (cancel_lock_1(&req, &lock_res, clnt) != RPC_SUCCESS) {
                clnt_perror(clnt, "Fallo CANCEL_LOCK");
            }
        }
    }

    clnt_destroy(clnt);
    return exit_code;
}