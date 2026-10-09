CC      = gcc
CFLAGS  = -Wall -I/usr/include/tirpc
LDLIBS  = -ltirpc

all: servidor cliente

proceso.h proceso_clnt.c proceso_svc.c proceso_xdr.c: proceso.x
	rpcgen -C proceso.x

servidor: servidor.c proceso_svc.c proceso_xdr.c proceso.h
	$(CC) $(CFLAGS) -o $@ servidor.c proceso_svc.c proceso_xdr.c $(LDLIBS)

cliente: cliente.c proceso_clnt.c proceso_xdr.c proceso.h
	$(CC) $(CFLAGS) -o $@ cliente.c proceso_clnt.c proceso_xdr.c $(LDLIBS)

clean:
	rm -f servidor cliente proceso.h proceso_clnt.c proceso_svc.c proceso_xdr.c

.PHONY: all clean
