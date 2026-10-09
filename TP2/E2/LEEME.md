# Ejercicio 2 – Finalización de procesos mediante RPC

Archivos escritos por el alumno: `proceso.x`, `servidor.c`, `cliente.c`, `Makefile`
Generados por rpcgen: `proceso.h`, `proceso_clnt.c`, `proceso_svc.c`, `proceso_xdr.c`

## Compilar
    make

## Ejecutar (dentro del contenedor, con rpcbind activo)
Terminal 1 – servidor (como usuario NO root, para poder probar "sin permiso"):
    setpriv --reuid=65534 --regid=65534 --clear-groups ./servidor

Terminal 2 – cliente:
    ./cliente localhost pid <PID>
    ./cliente localhost nombre <nombre>

## Pruebas obligatorias
1. Éxito:        setpriv --reuid=65534 --regid=65534 --clear-groups sleep 300 &   →  ./cliente localhost pid <PID>
2. Inexistente:  ./cliente localhost pid 999999
3. No finalizable (proceso de root, servidor sin privilegios):  sleep 300 &  →  ./cliente localhost pid <PID>
