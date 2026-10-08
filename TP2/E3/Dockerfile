FROM debian:bookworm-slim

# Evitar prompts interactivos durante la instalación
ENV DEBIAN_FRONTEND=noninteractive

# Instalar herramientas de compilación C/C++ y dependencias de RPC
RUN apt-get update && apt-get install -y \
    build-essential \
    rpcsvc-proto \
    libtirpc-dev \
    rpcbind \
    && rm -rf /var/lib/apt/lists/*

# Configurar el directorio de trabajo
WORKDIR /app

# Exponer el puerto estándar del portmapper (rpcbind)
EXPOSE 111/tcp 111/udp

# Copiar el script de inicio
COPY entrypoint.sh /entrypoint.sh
RUN sed -i 's/\r$//' /entrypoint.sh && chmod +x /entrypoint.sh

# Configurar el entrypoint para asegurar que rpcbind inicie siempre
ENTRYPOINT ["/entrypoint.sh"]

# Por defecto, abrir una terminal bash
CMD ["/bin/bash"]