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
using namespace std;

int main(int argc, char* argv[]){

    if (argc < 3) {
        printf("Uso: %s <ip_servidor> <puerto>\n", argv[0]);
        exit(-1);
    }

    const char* ip_destino = argv[1];
    int puerto_destino = atoi(argv[2]);

    int socketfd;
    struct sockaddr_in destino;

    socketfd=socket(AF_INET, SOCK_STREAM,0);

    memset(&destino, 0, sizeof(destino));

    destino.sin_family=AF_INET;
    destino.sin_port=htons(puerto_destino);
    destino.sin_addr.s_addr=inet_addr(ip_destino);

    int estado_Conexion = connect(socketfd, (struct sockaddr *)&destino,sizeof(struct sockaddr));

    if(estado_Conexion == -1){
        printf("error en la Conexion\n");
        exit(-1);
    }

    string mensaje;
    cout << "Ingrese el mensaje a enviar: ";
    getline(cin, mensaje);

    int estado_Envio = send(socketfd, mensaje.c_str(), mensaje.size(), 0);

    if(estado_Envio == -1){
        printf("error en el Envio\n");
        exit(-1);
    }

    printf("Mensaje enviado: %s\n", mensaje.c_str());

    char buffer[1024];
    int bytes_recibidos;

    bytes_recibidos = recv(socketfd, buffer, sizeof(buffer)-1, 0);

    if(bytes_recibidos == -1){
        printf("error en Recepcion\n");
        exit(-1);
    }
    else if (bytes_recibidos == 0) {
        printf("El servidor ha cerrado la conexion.\n");
    }
    else{
        buffer[bytes_recibidos] = '\0';
        printf("Respuesta del servidor: %s\n", buffer);
    }

    close(socketfd);
    return(0);
}