FROM debian:bookworm-slim

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    rpcsvc-proto \
    libtirpc-dev \
    rpcbind \
    ca-certificates \
    netbase \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /my_FS
COPY myfs.x servidor.cpp cliente.cpp ./
COPY receta.txt ./Archivos/

RUN rpcgen -C -M -h myfs.x -o myfs.h && \
    rpcgen -C -M -c myfs.x -o myfs_xdr.c && \
    rpcgen -C -M -l myfs.x -o myfs_clnt.c && \
    gcc -I/usr/include/tirpc -I. -c myfs_xdr.c -o /tmp/myfs_xdr.o && \
    gcc -I/usr/include/tirpc -I. -c myfs_clnt.c -o /tmp/myfs_clnt.o && \
    g++ -std=c++17 -pthread -I/usr/include/tirpc -I. -o /usr/local/bin/servidor servidor.cpp /tmp/myfs_xdr.o -ltirpc && \
    g++ -std=c++17 -pthread -I/usr/include/tirpc -I. -o /usr/local/bin/cliente cliente.cpp /tmp/myfs_clnt.o /tmp/myfs_xdr.o -ltirpc

EXPOSE 111/tcp 111/udp

COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]
CMD ["/usr/local/bin/servidor"]


# docker build -t tp3-rpc .
# docker run -d --rm --name mi-servidor-rpc tp3-rpc
# Ver el estado
# docker exec -it mi-servidor-rpc cliente 127.0.0.1 Cliente1 status
# Probar el lock con dos clientes
# docker exec -it mi-servidor-rpc cliente 127.0.0.1 Cliente1 test_lock receta.txt 15
# docker exec -it mi-servidor-rpc cliente 127.0.0.1 Cliente2 test_lock receta.txt 5
#Descargar el archivo solicitado
#docker exec -it mi-servidor-rpc cliente 127.0.0.1 Cliente3 get receta.txt /