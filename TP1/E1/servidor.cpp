#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h> // Funciones principales de sockets
#include <netinet/in.h> // Estructuras de direcciones de internet
#include <arpa/inet.h>  // Operaciones de conversión (ej. inet_pton)
#include <unistd.h>     // Para usar close(), read(), write()
#include <cstring>

int main(void) {

    int socketfd; // file descriptor del socket
    int cliente; // file descriptor del cliente
    
    struct sockaddr_in nos;
    struct sockaddr_in ellos;

    socketfd=socket(AF_INET, SOCK_STREAM,0);

    memset(&nos, 0, sizeof(nos));

    nos.sin_family=AF_INET; // IPv4
    nos.sin_port=htons(8000); // puerto
    nos.sin_addr.s_addr=INADDR_ANY; // cualquier direccion

    if (socketfd==-1){
        printf("error en socket\n");
        std::exit(-1);
    }

    bind(socketfd, (struct sockaddr *)&nos, sizeof(nos));

    listen(socketfd, 10);

    socklen_t addr_len = sizeof(ellos);
    cliente = accept(socketfd, (struct sockaddr *)&ellos, &addr_len); // descriptor por el que podesos enviar y recibir 

    if(cliente == -1){
        printf("error en accept\n");
        std::exit(-1);
    }

    //recibimos el mensaje del cliente
    char buffer[1024];
    int len, bytes_recibidos;


    bytes_recibidos = recv(cliente, buffer,sizeof(buffer)-1, 0);

    if(bytes_recibidos == -1){
        printf("Error en Recepcion del Serve\n");
        std::exit(-1);
    }
    else if (bytes_recibidos == 0) {
        printf("El cliente ha cerrado la conexión.\n");
    }
    else{
        buffer[bytes_recibidos] = '\0';
        printf("Mensaje recibido: %s\n", buffer);
    }

    //enviamos el mensaje al cliente
    int bytes_enviados;

    bytes_enviados = send(cliente, buffer, sizeof(buffer)-1, 0);

    if(bytes_enviados == -1){
        printf("error en el Envio\n");
        std::exit(-1);
    }else if(bytes_enviados == 0){
        printf("El cliente ha cerrado la conexión.\n");
    }
    else{
        printf("Mensaje enviado: %s\n", buffer);
    }


    close(cliente);
    close(socketfd);

    return 0;
}