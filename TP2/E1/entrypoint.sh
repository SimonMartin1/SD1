#!/bin/bash

# Llamamos a rpcbind directamente en lugar de usar "service"
rpcbind

# Ejecutar el comando pasado al contenedor (por defecto será /bin/bash)
exec "$@"