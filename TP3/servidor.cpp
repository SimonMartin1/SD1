#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <list>
#include <mutex>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <limits>
#include <system_error>

extern "C" {
    #include "myfs.h"
}

using namespace std;
namespace fs = filesystem;


// Estructura de LockManager para proteger datos concurrentes
struct LockState {
    bool is_locked = false;
    string owner_client_id = "";
    list<string> waitlist; // Lista de clientes en espera
};

class LockManager {
    unordered_map<string, LockState> locks;
    mutex mtx; // Protege de accesos concurrentes
public:
    int acquire(const string& lock, const string& client) {
        lock_guard<mutex> lk(mtx);
        auto& state = locks[lock];
        if (!state.is_locked) {
            state.is_locked = true;
            state.owner_client_id = client;
            return LOCK_GRANTED;
        } else {
            bool found = false;
            for(auto& c : state.waitlist) if(c == client) found = true;
            if(!found && state.owner_client_id != client) {
                state.waitlist.push_back(client);
            }
            return LOCK_WAITING;
        }
    }

    int check(const string& lock, const string& client) {
        lock_guard<mutex> lk(mtx);
        auto it = locks.find(lock);
        if (it != locks.end() && it->second.is_locked &&
            it->second.owner_client_id == client) {
            return LOCK_GRANTED;
        }
        return LOCK_WAITING;
    }

    int release(const string& lock, const string& client) {
        lock_guard<mutex> lk(mtx);
        auto it = locks.find(lock);
        if (it != locks.end() && it->second.is_locked &&
            it->second.owner_client_id == client) {
            release_owner(it->second);
            return LOCK_GRANTED;
        }
        return LOCK_WAITING;
    }

    int cancel(const string& lock, const string& client) {
        lock_guard<mutex> lk(mtx);
        auto it = locks.find(lock);
        if (it == locks.end()) return LOCK_WAITING;

        auto& state = it->second;
        if (state.is_locked && state.owner_client_id == client) {
            release_owner(state);
            return LOCK_GRANTED;
        }
        auto waiting = find(state.waitlist.begin(), state.waitlist.end(), client);
        if (waiting == state.waitlist.end()) return LOCK_WAITING;
        state.waitlist.erase(waiting);
        return LOCK_GRANTED;
    }

    string get_status() {
        lock_guard<mutex> lk(mtx);
        string status = "\n--- Estado de Locks en el Servidor ---\n";
        for (const auto& pair : locks) {
            status += "Lock: [" + pair.first + "] | Estado: ";
            if (pair.second.is_locked) {
                status += "CONCEDIDO a [" + pair.second.owner_client_id + "]\n";
                if (!pair.second.waitlist.empty()) {
                    status += "   -> En espera: ";
                    for (const auto& w : pair.second.waitlist) status += "[" + w + "] ";
                    status += "\n";
                }
            } else {
                status += "LIBRE\n";
            }
        }
        return status;
    }

private:
    static void release_owner(LockState& state) {
        if (!state.waitlist.empty()) {
            state.owner_client_id = state.waitlist.front();
            state.waitlist.pop_front();
        } else {
            state.is_locked = false;
            state.owner_client_id.clear();
        }
    }
};


class FileCache {
private:
    struct CacheNode {
        string filename;     
        string content;      
        size_t total_node_bytes;  
    };
    list<CacheNode> usage_list;
    unordered_map<string, list<CacheNode>::iterator> cache_map;
    const size_t max_bytes;
    size_t current_bytes;
    mutable mutex cache_mutex;

    size_t estimate_string_mem(const string& str) const {
        return sizeof(str) + str.capacity();
    }
    size_t calcularEspacioEnMemoria(const string& filename, const string& content) {
        return estimate_string_mem(filename) + estimate_string_mem(content) + 48;
    }

public:
    explicit FileCache(size_t max_bytes_capacity) 
        : max_bytes(max_bytes_capacity), current_bytes(0) {}

    bool obtenerArchivo(const string& filename, string& out_content) {
        lock_guard<mutex> lock(cache_mutex);
        auto it = cache_map.find(filename);
        if (it == cache_map.end()) return false; 
        usage_list.splice(usage_list.begin(), usage_list, it->second);
        out_content = it->second->content;
        return true; 
    }

    void insertarArchivo(const string& filename, const string& content) {
        lock_guard<mutex> lock(cache_mutex);
        size_t new_node_bytes = calcularEspacioEnMemoria(filename, content);
        if (new_node_bytes > max_bytes) return; 

        auto it = cache_map.find(filename);
        if (it != cache_map.end()) {
            current_bytes -= it->second->total_node_bytes;
            it->second->content = content;
            it->second->total_node_bytes = new_node_bytes;
            current_bytes += new_node_bytes;
            usage_list.splice(usage_list.begin(), usage_list, it->second);
        } else {
            current_bytes += new_node_bytes;
            usage_list.push_front({filename, content, new_node_bytes});
            cache_map[filename] = usage_list.begin();
        }
        while (current_bytes > max_bytes && !usage_list.empty()) {
            const auto& oldest_file = usage_list.back(); 
            current_bytes -= oldest_file.total_node_bytes;
            cache_map.erase(oldest_file.filename);
            usage_list.pop_back();
        }
    }
};


LockManager lock_manager;
FileCache server_cache(512 * 1024 * 1024);
const fs::path RUTA_BASE = "/my_FS/Archivos";

bool resolve_file_path(const char* file_name, fs::path& resolved_path) {
    if (file_name == nullptr || *file_name == '\0') return false;

    fs::path requested_path(file_name);
    if (requested_path.is_absolute()) return false;
    for (const auto& component : requested_path) {
        if (component == "..") return false;
    }

    std::error_code error;
    fs::path root = fs::canonical(RUTA_BASE, error);
    if (error) return false;
    fs::path candidate = fs::canonical(root / requested_path, error);
    if (error) return false;

    auto root_component = root.begin();
    auto candidate_component = candidate.begin();
    for (; root_component != root.end(); ++root_component, ++candidate_component) {
        if (candidate_component == candidate.end() || *root_component != *candidate_component) {
            return false;
        }
    }

    if (!fs::is_regular_file(candidate, error) || error) return false;
    resolved_path = std::move(candidate);
    return true;
}

extern "C" {
    bool_t acquire_lock_1_svc(LockRequest *argp, int *result, struct svc_req *rqstp) {
        if (argp == nullptr || result == nullptr || argp->lock_name == nullptr ||
            argp->client_id == nullptr) return FALSE;
        *result = lock_manager.acquire(argp->lock_name, argp->client_id);
        return TRUE;
    }

    bool_t check_lock_1_svc(LockRequest *argp, int *result, struct svc_req *rqstp) {
        if (argp == nullptr || result == nullptr || argp->lock_name == nullptr ||
            argp->client_id == nullptr) return FALSE;
        *result = lock_manager.check(argp->lock_name, argp->client_id);
        return TRUE;
    }

    bool_t release_lock_1_svc(LockRequest *argp, int *result, struct svc_req *rqstp) {
        if (argp == nullptr || result == nullptr || argp->lock_name == nullptr ||
            argp->client_id == nullptr) return FALSE;
        *result = lock_manager.release(argp->lock_name, argp->client_id);
        return TRUE;
    }

    bool_t cancel_lock_1_svc(LockRequest *argp, int *result, struct svc_req *rqstp) {
        if (argp == nullptr || result == nullptr || argp->lock_name == nullptr ||
            argp->client_id == nullptr) return FALSE;
        *result = lock_manager.cancel(argp->lock_name, argp->client_id);
        return TRUE;
    }

    bool_t get_server_status_1_svc(void *argp, char **result, struct svc_req *rqstp) {
        if (result == nullptr) return FALSE;
        *result = nullptr;
        string status = lock_manager.get_status();
        *result = strdup(status.c_str());
        return *result != nullptr;
    }

    bool_t get_file_1_svc(FileRequest *argp, FileResponse *result, struct svc_req *rqstp) {
        if (argp == nullptr || result == nullptr) return FALSE;
        result->status = FILE_ERROR;
        result->data.data_val = nullptr;
        result->data.data_len = 0;

        fs::path file_path;
        if (!resolve_file_path(argp->file_name, file_path)) {
            printf("[RPC] Archivo no encontrado o ruta no permitida.\n");
            return TRUE;
        }

        string ruta_archivo = file_path.string();
        string contenido;
        
        bool enCache = server_cache.obtenerArchivo(ruta_archivo, contenido);
        if (enCache) {
            printf("[RPC] Cache Hit: '%s'.\n", ruta_archivo.c_str());
        } else {
            printf("[RPC] Cache Miss: Buscando '%s'...\n", ruta_archivo.c_str());
            ifstream archivo(ruta_archivo, ios::binary | ios::ate);
            if (!archivo.is_open()) {
                return TRUE;
            }
            streamsize tam = archivo.tellg();
            if (tam < 0 || static_cast<unsigned long long>(tam) >
                               numeric_limits<u_int>::max()) {
                return TRUE;
            }
            archivo.seekg(0, ios::beg);
            contenido.resize(static_cast<size_t>(tam));
            if (tam > 0) {
                archivo.read(contenido.data(), tam);
                if (!archivo) return TRUE;
            }

            server_cache.insertarArchivo(ruta_archivo, contenido);
        }

        if (!contenido.empty()) {
            result->data.data_val = static_cast<char*>(malloc(contenido.size()));
            if (result->data.data_val == nullptr) return FALSE;
            memcpy(result->data.data_val, contenido.data(), contenido.size());
        }
        result->data.data_len = static_cast<u_int>(contenido.size());
        result->status = FILE_OK;
        return TRUE;
    }

    // Callback necesario para liberar la memoria asignada dinámicamente en RPC multihilo
    int myfs_prog_1_freeresult(SVCXPRT *transp, xdrproc_t xdr_result, caddr_t result) {
        xdr_free(xdr_result, result);
        return 1;
    }
}