#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>  
#include <unistd.h>     
#include <cstring>
#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>
using namespace std;
namespace fs = filesystem;

void recibirArchivo(int socket_fd, const string& ruta_destino, const string& nombre_archivo) {
    // 1. Recibir el tamaño del archivo primero (o código de error)
    streamsize leng = 0;
    int resp = recv(socket_fd, reinterpret_cast<char*>(&leng), sizeof(leng), 0);

    if (resp <= 0) {
        printf("Error: No se pudo recibir respuesta del servidor o la conexión se cerró\n");
        return;
    }

    // Si el servidor envía -1, significa que el archivo no fue encontrado
    if (leng < 0) {
        printf("Error: El archivo '%s' no se encuentra en el servidor\n", nombre_archivo.c_str());
        return;
    }

    // 2. Determinar la ruta de destino (soporta directorio o ruta con nombre de archivo)
    fs::path p(ruta_destino);
    if (fs::is_directory(p) || (!ruta_destino.empty() && (ruta_destino.back() == '/' || ruta_destino.back() == '\\'))) {
        p /= fs::path(nombre_archivo).filename();
    }

    if (p.has_parent_path()) {
        fs::create_directories(p.parent_path());
    }
    
    ofstream archivo(p, ios::binary);
    if (!archivo.is_open()) {
        printf("Error al crear el archivo de destino: %s\n", p.string().c_str());
        return;
    }

    // 3. Recibir los bloques de datos
    char buffer[1024];
    streamsize bytes_totales_recibidos = 0;

    while (bytes_totales_recibidos < leng) {
        size_t bytes_a_recibir = min(sizeof(buffer), static_cast<size_t>(leng - bytes_totales_recibidos));
        int bytes_recibidos = recv(socket_fd, buffer, bytes_a_recibir, 0);
        
        if (bytes_recibidos == -1) {
            printf("Error en la recepción de datos\n");
            break;
        }
        else if (bytes_recibidos == 0) {
            printf("El servidor ha cerrado la conexión antes de completar la transferencia\n");
            break;
        }

        archivo.write(buffer, bytes_recibidos);
        bytes_totales_recibidos += bytes_recibidos;
    }
    if (bytes_totales_recibidos == leng) {
        printf("Archivo recibido correctamente: %s (%ld bytes)\n", p.string().c_str(), static_cast<long>(leng));
    } else {
        printf("La transferencia falló o se cortó\n");
    }
    archivo.close();
}


int main(int argc, char* argv[]){

    const char* ip_destino = argv[1],*archivo_solicitado = argv[3],*directorio_donde_almacenar = argv[4];
    int puerto_destino = atoi(argv[2]);
    int socketfd;
    struct sockaddr_in destino;

    socketfd=socket(AF_INET, SOCK_STREAM,0);

    memset(&destino, 0, sizeof(destino));
    

    destino.sin_family=AF_INET;
    destino.sin_port=htons(puerto_destino);
    destino.sin_addr.s_addr=inet_addr(ip_destino);

    int estado_Conexion = connect(socketfd, (struct sockaddr *)&destino,sizeof(struct sockaddr));   
    
    if (estado_Conexion == -1) {
        printf("Error en la conexión\n");
        exit(-1);
    }
    else {
        int estado_Envio = send(socketfd, archivo_solicitado, strlen(archivo_solicitado), 0);

        if (estado_Envio == -1) {
            printf("Error en el envío\n");
            exit(-1);
        }
        else {
            string url(directorio_donde_almacenar);
            recibirArchivo(socketfd, url, archivo_solicitado);
        }
    }
    close(socketfd);
    return 0;
}

//Construir la Imagen
//docker build -t sockets-cpp-test .


//Arrancar Contenedor
//docker run -it --name mis-sockets sockets-cpp-test

// Corre y borra al finalizar 
//Version Linux
//docker run -it --rm --name mis-sockets sockets-cpp-test && docker rmi sockets-cpp-test

//Version Windows
//docker run -it --rm --name mis-sockets sockets-cpp-test; docker rmi sockets-cpp-test


//Entrar al servidor en la misma consola
//    ./servidor 


//Entrar al cliente
// docker exec -it mis-sockets bash
// ./cliente

//sudo docker system prune