#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>  
#include <unistd.h>     
#include <cstring>
#include <iostream>
#include <fstream> 
#include <filesystem>
#include <string>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <list>
#include <utility>
#include <mutex>

using namespace std;
namespace fs = filesystem;


class FileCache {
private:
    // Estructura interna necesaria para aplicar la política de reemplazo LRU
    struct CacheNode {
        string filename;     // Nombre del archivo (Clave)
        string content;      // Contenido del archivo (Valor)
        size_t total_node_bytes;  // Peso total del nodo para el control de la caché
    };

    list<CacheNode> usage_list;
    unordered_map<string, list<CacheNode>::iterator> cache_map;
    
    const size_t max_bytes;
    size_t current_bytes;
    mutable mutex cache_mutex;

    // Calcula el tamaño real en memoria de un string (Stack + Heap)
    size_t estimate_string_mem(const string& str) const {
        return sizeof(str) + str.capacity();
    }

    // Calcula cuánta memoria consume este nodo sumando el overhead de las estructuras de C++
    size_t calcularEspacioEnMemoria(const string& filename, const string& content) {
        size_t raw_payload = estimate_string_mem(filename) + estimate_string_mem(content);
        
        // Overhead estimado de punteros de list y unordered_map (arquitectura 64-bits)
        size_t cpp_overhead = 48; 
        
        return raw_payload + cpp_overhead;
    }

public:
    // Constructor donde definimos el límite en bytes
    explicit FileCache(size_t max_bytes_capacity) 
        : max_bytes(max_bytes_capacity), current_bytes(0) {}

    
    bool obtenerArchivo(const string& filename, string& out_content) {
        lock_guard<mutex> lock(cache_mutex);
        
        auto it = cache_map.find(filename);
        if (it == cache_map.end()) {
            return false; // Archivo no está en caché
        }
        
        // POLÍTICA LRU: Se usó el archivo, lo movemos al principio de la lista
        usage_list.splice(usage_list.begin(), usage_list, it->second);
        
        out_content = it->second->content;
        return true; // Cache Hit
    }

    // Insertar o actualizar un archivo en la caché
    void insertarArchivo(const string& filename, const string& content) {
        lock_guard<mutex> lock(cache_mutex);
        
        size_t new_node_bytes = calcularEspacioEnMemoria(filename, content);
        
        // Si un solo archivo es más grande que toda la caché (512MB), no se guarda
        if (new_node_bytes > max_bytes) {
            return; 
        }

        auto it = cache_map.find(filename);
        if (it != cache_map.end()) {
            // El archivo ya existía: actualizamos tamaño global y contenido
            current_bytes -= it->second->total_node_bytes;
            
            it->second->content = content;
            it->second->total_node_bytes = new_node_bytes;
            current_bytes += new_node_bytes;
            
            // Lo marcamos como recientemente usado
            usage_list.splice(usage_list.begin(), usage_list, it->second);
        } else {
            // Archivo nuevo: sumamos su tamaño e insertamos al inicio
            current_bytes += new_node_bytes;
            usage_list.push_front({filename, content, new_node_bytes});
            cache_map[filename] = usage_list.begin();
        }

        // POLÍTICA DE REEMPLAZO: Mientras superemos los 512MB, expulsamos el menos usado
        while (current_bytes > max_bytes && !usage_list.empty()) {
            const auto& oldest_file = usage_list.back(); // El último es el menos usado
            
            current_bytes -= oldest_file.total_node_bytes;
            cache_map.erase(oldest_file.filename);
            usage_list.pop_back(); // Desalojo físico
        }
    }

    // Método para verificar cuánta memoria real consume la caché actualmente
    double obtenerEspacioOcupado() const {
        lock_guard<mutex> lock(cache_mutex);
        return current_bytes / (1024.0 * 1024.0);
    }
};




void enviarArchivo(int socket_fd, const string& ruta_archivo, FileCache& cache) {
    string contenido;
    
    
    bool enCache = cache.obtenerArchivo(ruta_archivo, contenido);
    
    if (enCache) {
        printf("\nCache Hit: Leyendo '%s' desde la memoria RAM.\n", ruta_archivo.c_str());
    } else {
        //printf("\n\n", ruta_archivo);
        printf("\nCache Miss: Buscando '%s' en el directorio...\n", ruta_archivo.c_str());
        
        // Verificar si el archivo existe físicamente y es un archivo regular
        if (!fs::exists(ruta_archivo) || !fs::is_regular_file(ruta_archivo)) {
            printf("Error: El archivo '%s' no existe en el servidor.\n", ruta_archivo.c_str());
            streamsize leng_error = -1;
            send(socket_fd, reinterpret_cast<char*>(&leng_error), sizeof(leng_error), 0);
            return;
        }
        
        // Abrir el archivo original en modo binario
        ifstream archivo(ruta_archivo, ios::binary | ios::ate);
        if (!archivo.is_open()) {
            printf("Error: No se pudo abrir el archivo '%s' desde el disco.\n", ruta_archivo.c_str());
            streamsize leng_error = -1;
            send(socket_fd, reinterpret_cast<char*>(&leng_error), sizeof(leng_error), 0);
            return;
        }
        
        // Obtener el tamaño y leer todo su contenido
        streamsize tamaño_disco = archivo.tellg();
        archivo.seekg(0, ios::beg);
        
        contenido.resize(tamaño_disco);
        archivo.read(&contenido[0], tamaño_disco);
        archivo.close();
        
        // Guardar en la caché para las próximas peticiones
        cache.insertarArchivo(ruta_archivo, contenido);
    }

    // 2. Enviar el tamaño total del archivo (sea de caché o de disco)
    streamsize leng = contenido.size();
    send(socket_fd, reinterpret_cast<char*>(&leng), sizeof(leng), 0);
    
    // 3. Enviar el archivo en bloques (Buffer de 1024 bytes) desde el string en RAM
    size_t bytes_enviados_totales = 0;
    char buffer[1024];
    
    while (bytes_enviados_totales < leng) {
        // Calcular cuántos bytes faltan por enviar en este bloque (máximo 1024)
        size_t bytes_a_enviar = min(sizeof(buffer), leng - bytes_enviados_totales);
        
        // Copiar los datos del string al buffer temporal
        contenido.copy(buffer, bytes_a_enviar, bytes_enviados_totales);
        
        int bytes_enviados = send(socket_fd, buffer, bytes_a_enviar, 0);
        
        if (bytes_enviados == -1) {
            printf("error en el Envio\n");
            exit(-1);
        } else if (bytes_enviados <= 0) {
            printf("El cliente ha cerrado la conexion.\n");
            return;
        }
        
        bytes_enviados_totales += bytes_enviados;
    }
    
    printf("Archivo enviado con exito: %s\n", ruta_archivo.c_str());
}

void manejarCliente(string ruta,int cliente, FileCache& server_cache) {
    
    char buffer[1024];
    int bytes_recibidos;
                
                
    bytes_recibidos = recv(cliente, buffer, sizeof(buffer) - 1, 0);
    if (bytes_recibidos <= 0) {
        printf("Error en la solicitud o el cliente se desconectó.\n");
        close(cliente);
        return;
    }
                
    string archivo(buffer, bytes_recibidos);
                
    enviarArchivo(cliente,ruta.append(archivo),server_cache);
    
    close(cliente);
    
    printf("Cliente desconectado \n");
}




int main(void) {

    int server_Id; 
    int cliente_Id; 
    
    struct sockaddr_in socket_Servidor;
    struct sockaddr_in socket_Cliente;

    server_Id=socket(AF_INET, SOCK_STREAM,0);

    memset(&socket_Servidor, 0, sizeof(socket_Servidor));

    socket_Servidor.sin_family=AF_INET; 
    socket_Servidor.sin_port=htons(8000); 
    socket_Servidor.sin_addr.s_addr=INADDR_ANY;


    string ruta = "/my_FS/Archivos/";
    
    const size_t tamanioEnMemoria = 512 * 1024 * 1024;
    FileCache server_cache(tamanioEnMemoria);

    if (server_Id==-1){
        printf("Error en socket\n");
        exit(-1);
    }

    else{
    
        bind(server_Id, (struct sockaddr *)&socket_Servidor, sizeof(socket_Servidor));
        
        printf("Escuchando en el Puerto: %d\nDirectorio en Uso: %s\n", ntohs(socket_Servidor.sin_port), fs::absolute(ruta).c_str());
        
        string listado_archivos;
            
            try {
                for (const auto& entrada : fs::directory_iterator(ruta)) {
                    // ¿Es un archivo?
                    if (entrada.is_regular_file()) {
                        listado_archivos.append(entrada.path().filename().string());
                        listado_archivos.append("\n");
                    } 
                }
            } catch (const fs::filesystem_error& e) {
                listado_archivos.append("Error al acceder al directorio");
            }
        
        printf("\nArchivos Disponibles %s \n", listado_archivos.c_str());
        
        listen(server_Id, 10);
        
        while (true) {
            socklen_t addr_len = sizeof(socket_Cliente);
            cliente_Id = accept(server_Id, (struct sockaddr *)&socket_Cliente, &addr_len); 
            
            if(cliente_Id == -1){
                printf("error en accept\n");
                exit(-1);
            }
            else{
                
                printf("\nIP del Cliente: '%s'", inet_ntoa(socket_Cliente.sin_addr));
                
                thread hilo_cliente(manejarCliente, ruta, cliente_Id, ref(server_cache));
                
                hilo_cliente.detach();
                
            }
        }
        close(server_Id);
    }
    return 0;
}