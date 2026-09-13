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

    int socketfd;
    struct sockaddr_in destino;

    socketfd=socket(AF_INET, SOCK_STREAM,0);

    memset(&destino, 0, sizeof(destino));
    

    destino.sin_family=AF_INET;
    destino.sin_port=htons(8000);
    destino.sin_addr.s_addr=inet_addr("127.0.0.1");

    int estado_Conexion = connect(socketfd, (struct sockaddr *)&destino,sizeof(struct sockaddr));   
    
    if(estado_Conexion == -1){
        printf("error en la Conexion\n");
        exit(-1);
    }
    else{
    
        

        char* msg= "";
        int len_msg, input;
        len_msg = strlen(msg);

        int estado_Envio = send(socketfd,msg,len_msg,0);
        bool flag=true;

        if(estado_Envio == -1){
            printf("error en el Envio\n");
            exit(-1);
        }
        else{
            while(flag){
                
                char buffer[100];
                int bytes_recibidos;
                
                bytes_recibidos = recv(socketfd, buffer, sizeof(buffer)-1, 0);
                
                if(bytes_recibidos == -1){
                    printf("error en Recepcion\n");
                    exit(-1);
                }
                else if (bytes_recibidos == 0) {
                printf("El servidor ha cerrado la conexión.\n");
                }
                else{
                    buffer[bytes_recibidos] = '\0';
                    printf(buffer);
                    printf("Ingrese el archivo que desea obtener");
                    scanf("%d ",&input);
                    if(input==0){
                        flag=false;
                    }
                }
            }
        }
    }
    close(socketfd);
    return(0);
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