#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>  
#include <unistd.h>     
#include <cstring>

int main(void) {

    int socketfd; 
    int cliente; 

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

    printf("Servidor escuchando en el puerto 8000...\n");

    socklen_t addr_len = sizeof(ellos);
    cliente = accept(socketfd, (struct sockaddr *)&ellos, &addr_len); 

    if(cliente == -1){
        printf("error en accept\n");
        std::exit(-1);
    }

    printf("Cliente conectado: %s\n", inet_ntoa(ellos.sin_addr));

    //recibimos el mensaje del cliente
    char buffer[1024];
    int bytes_recibidos;

    bytes_recibidos = recv(cliente, buffer, sizeof(buffer)-1, 0);

    if(bytes_recibidos == -1){
        printf("Error en Recepcion del Servidor\n");
        close(cliente);
        close(socketfd);
        std::exit(-1);
    }
    else if (bytes_recibidos == 0) {
        printf("El cliente ha cerrado la conexion.\n");
    }
    else{
        buffer[bytes_recibidos] = '\0';
        printf("Mensaje recibido: %s\n", buffer);

        //enviamos el eco al cliente (solo si realmente llego algo)
        int bytes_enviados;
        bytes_enviados = send(cliente, buffer, bytes_recibidos, 0);

        if(bytes_enviados == -1){
            printf("error en el Envio\n");
            std::exit(-1);
        }else if(bytes_enviados == 0){
            printf("El cliente ha cerrado la conexion.\n");
        }
        else{
            printf("Mensaje enviado: %s\n", buffer);
        }
    }

    close(cliente);
    close(socketfd);

    return 0;
}