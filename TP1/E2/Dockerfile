
FROM gcc:latest

WORKDIR /my_FS


# Añadimos los archivos 
COPY receta.txt ./Archivos/

COPY servidor.cpp cliente.cpp ./

RUN g++ servidor.cpp -o servidor
RUN g++ cliente.cpp -o cliente

CMD ["bash"]