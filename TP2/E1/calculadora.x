/* calculadora.x */

/* Estructura para enviar los dos números */
struct operandos {
    int a;
    int b;
};

/* Definición del programa RPC */
program CALCULADORA_PROG {
    version CALCULADORA_VERS {
        int SUMA(operandos) = 1;   /* Procedimiento 1 */
        int RESTA(operandos) = 2;  /* Procedimiento 2 */
    } = 1; /* Versión 1 */
} = 0x20000001; /* Número de programa único (entre 0x20000000 y 0x3fffffff para usuarios) */