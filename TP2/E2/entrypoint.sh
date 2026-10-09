#!/bin/bash
# Inicia rpcbind (portmapper) y luego ejecuta el comando pedido (por defecto, bash)
rpcbind
sleep 1
exec "$@"
