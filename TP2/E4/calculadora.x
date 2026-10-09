/* calculadora.x */

/* Estructura para enviar los dos números */
typedef string cadena<>; /*

/* Definición del programa RPC */
program CALCULADORA_PROG {
    version CALCULADORA_VERS {
        int cant_carac_e(cadena) = 1;   /* Procedimiento 1 */
        int cant_carac_se(cadena) = 2;  /* Procedimiento 2 */
        int cant_palabras(cadena) = 3;  /* Procedimiento 3 */
    } = 1; /* Versión 1 */
} = 0x20000001; /* Número de programa único (entre 0x20000000 y 0x3fffffff para usuarios) */