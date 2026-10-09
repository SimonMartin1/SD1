/* proceso.x - Interfaz RPC del Ejercicio 2: finalizacion de procesos */

const MAX_MSG = 256;
const MAX_NOMBRE = 128;

/* Codigos de resultado */
enum estado {
    OK = 0,             /* proceso(s) finalizado(s) */
    NO_EXISTE = 1,      /* no existe el proceso solicitado */
    SIN_PERMISO = 2,    /* el servidor no tiene permiso (EPERM) */
    NO_FINALIZO = 3,    /* se envio la senal pero el proceso sigue vivo */
    PROHIBIDO = 4,      /* pid/nombre protegido (PID 1, el propio servidor) */
    ERROR = 5           /* otro error */
};

struct respuesta {
    estado codigo;
    int cantidad;               /* procesos finalizados */
    string mensaje<MAX_MSG>;
};

typedef string nombre_proc<MAX_NOMBRE>;

program TERMINAR_PROG {
    version TERMINAR_VERS {
        respuesta TERMINAR_POR_PID(int) = 1;
        respuesta TERMINAR_POR_NOMBRE(nombre_proc) = 2;
    } = 1;
} = 0x31234567;
