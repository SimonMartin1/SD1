
Importante -- Los comandos son para powershell

## 1. Crear la red Docker

```powershell
docker network create rpcnet
```

Crea la red `rpcnet`, a la que se conectarán el servidor y los contenedores
cliente.

## 2. Construir la imagen

```powershell
docker build --no-cache -t tp3-rpc .
```

Construye la imagen `tp3-rpc` con los fuentes y archivos del proyecto. La opción
`--no-cache` fuerza la reconstrucción de los pasos de la imagen.

## 3. Iniciar el servidor

En una terminal, ejecutá:

```powershell
docker run --rm --name tp3-servidor-prueba `
  --network rpcnet --network-alias servidor `
  tp3-rpc
```

Inicia el servidor conectado a `rpcnet` y le asigna el alias de red `servidor`.
Dejá esta terminal abierta mientras probás los clientes.

## 4. Descargar un archivo

En otra terminal, ejecutá:

```powershell
docker run --rm -it --name cliente1 --network rpcnet `
  --entrypoint /usr/local/bin/cliente `
  tp3-rpc servidor Cliente1 get receta.txt /
```

Ejecuta el cliente `Cliente1`, que solicita y descarga `receta.txt`. La dirección
`servidor` es el alias de red del contenedor que está ejecutando el servidor.
El archivo se guarda en `/` dentro del contenedor cliente, que se elimina al
terminar por la opción `--rm`.

## 5. Consultar el estado de los locks

```powershell
docker run --rm -it --name cliente4 --network rpcnet `
  --entrypoint /usr/local/bin/cliente `
  tp3-rpc servidor Admin status
```

Ejecuta una consulta al servidor. La respuesta muestra los locks conocidos, su
estado, quién posee cada lock y qué clientes esperan.

## 6. Probar la espera y liberación de un lock

`La funcion test_lock permite a los clientes solicitar un lock para un determinado recurso por un tiempo ingresado por comando para probar el funcionamiento`

En una terminal, ejecutá el primer cliente:

```powershell
docker run --rm -it --name cliente1 --network rpcnet `
  --entrypoint /usr/local/bin/cliente `
  tp3-rpc servidor Cliente1 test_lock receta.txt 60
```

`Cliente1` solicita el lock `receta.txt` y, si lo obtiene, lo mantiene durante
60 segundos.

Mientras el primer comando sigue ejecutándose, abrí otra terminal y ejecutá:

```powershell
docker run --rm -it --name cliente2 --network rpcnet `
  --entrypoint /usr/local/bin/cliente `
  tp3-rpc servidor Cliente2 test_lock receta.txt 30
```

`Cliente2` solicita el mismo lock. Debe quedar en espera hasta que `Cliente1` lo
libere; luego lo obtiene, lo mantiene durante 30 segundos y finalmente lo
libera.


## 7. Probar el acceso a un recurso en paralelo

Repetir el paso anterior pero esta vez con un `Cliente3` para acceder a otro recurso mientras el `Cliente1` y `Cliente2` hacen uso y esperar respectivamente por el recurso receta.txt

docker exec -it mi-servidor-rpc cliente 127.0.0.1 Cliente3 test_lock informe.txt 30