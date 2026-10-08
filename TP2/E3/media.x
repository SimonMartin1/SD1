const MAX_SIZE = 100;

/* Estructura para enviar el arreglo de longitud variable */
struct arreglo_enteros {
    int numeros<MAX_SIZE>;
};

/* Estructura para la respuesta */
struct respuesta_media {
    float media;
    int status; /* 0 = OK, 1 = Arreglo vacío */
};

/* Definición del programa */
program MEDIAPROG {
    version MEDIAVERS {
        respuesta_media CALCULAR_MEDIA(arreglo_enteros) = 1;
    } = 1;
} = 0x20000002;