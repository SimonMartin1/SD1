
FROM gcc:latest

WORKDIR /Prueba-Cliente-Servidor

COPY servidor.cpp cliente.cpp ./

RUN g++ servidor.cpp -o servidor
RUN g++ cliente.cpp -o cliente

CMD ["bash"]