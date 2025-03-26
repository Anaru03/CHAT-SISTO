#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "cJSON.h"

#define PORT 50213
#define BUFFER_SIZE 1024

int sock;  // Socket global para uso en el hilo de recepción
char username[BUFFER_SIZE];  // Nombre de usuario global

// Función para enviar un mensaje (broadcast o directo)
void enviar_mensaje(const char *mensaje, const char *destinatario) {
    cJSON *json = cJSON_CreateObject();

    if (destinatario == NULL || strlen(destinatario) == 0) {
        // Mensaje de broadcast
        cJSON_AddStringToObject(json, "accion", "BROADCAST");
        cJSON_AddStringToObject(json, "nombre_emisor", username);
        cJSON_AddStringToObject(json, "mensaje", mensaje);
    } else {
        // Mensaje directo (DM)
        cJSON_AddStringToObject(json, "accion", "DM");
        cJSON_AddStringToObject(json, "nombre_emisor", username);
        cJSON_AddStringToObject(json, "nombre_destinatario", destinatario);
        cJSON_AddStringToObject(json, "mensaje", mensaje);
    }

    char *json_str = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);

    if (send(sock, json_str, strlen(json_str), 0) < 0) {
        perror("Error al enviar mensaje");
    }

    free(json_str);
}

// Función para cambiar el estado del cliente
void cambiar_estado(const char *nuevo_estado) {
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "tipo", "ESTADO");
    cJSON_AddStringToObject(json, "usuario", username);
    cJSON_AddStringToObject(json, "estado", nuevo_estado);
    char *json_str = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);

    if (send(sock, json_str, strlen(json_str), 0) < 0) {
        perror("Error al cambiar estado");
    } else {
        printf("Estado cambiado a: %s\n", nuevo_estado);
    }

    free(json_str);
}

// Hilo para recibir mensajes
void *recibir_mensajes(void *arg) {
    char response[BUFFER_SIZE];

    while (1) {
        int len = recv(sock, response, sizeof(response) - 1, 0);
        if (len > 0) {
            response[len] = '\0';

            cJSON *json_response = cJSON_Parse(response);
            if (!json_response) {
                printf("\nMensaje recibido (formato incorrecto): %s\n", response);
            } else {
                // Se pueden recibir mensajes con diferentes claves, se muestra lo recibido
                cJSON *accion = cJSON_GetObjectItem(json_response, "accion");
                cJSON *mensaje = cJSON_GetObjectItem(json_response, "mensaje");
                cJSON *response_field = cJSON_GetObjectItem(json_response, "response");
                cJSON *respuesta = cJSON_GetObjectItem(json_response, "respuesta");

                if (accion && cJSON_IsString(accion) && mensaje && cJSON_IsString(mensaje)) {
                    printf("\n[%s] %s\n", accion->valuestring, mensaje->valuestring);
                } else if (response_field && cJSON_IsString(response_field)) {
                    printf("\nRespuesta del servidor: %s\n", response_field->valuestring);
                } else if (respuesta && cJSON_IsString(respuesta)) {
                    printf("\nRespuesta del servidor: %s\n", respuesta->valuestring);
                } else {
                    printf("\nMensaje recibido: %s\n", response);
                }
                cJSON_Delete(json_response);
            }
            printf("\nSelecciona una opción: ");
            fflush(stdout);
        }
    }
    return NULL;
}

// Función para listar usuarios conectados
void listar_usuarios() {
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "accion", "LISTA");
    cJSON_AddStringToObject(json, "nombre_usuario", username);
    char *json_str = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);

    if (send(sock, json_str, strlen(json_str), 0) < 0) {
        perror("Error al solicitar listado de usuarios");
    }
    free(json_str);
}

// Función para consultar información de un usuario
void consultar_info_usuario() {
    printf("Introduce el nombre del usuario a consultar: ");
    char usuario_info[BUFFER_SIZE];
    fgets(usuario_info, BUFFER_SIZE, stdin);
    usuario_info[strcspn(usuario_info, "\n")] = 0;

    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "tipo", "MOSTRAR");
    cJSON_AddStringToObject(json, "usuario", usuario_info);
    char *json_str = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);

    if (send(sock, json_str, strlen(json_str), 0) < 0) {
        perror("Error al consultar información de usuario");
    }
    free(json_str);
}

int main() {
    struct sockaddr_in server;
    char direccion[BUFFER_SIZE];

    // Crear socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("No se pudo crear el socket");
        return 1;
    }

    server.sin_addr.s_addr = inet_addr("192.168.229.71");  // IP del servidor
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    // Conectar con el servidor
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Error al conectar al servidor");
        return 1;
    }

    // Registro del usuario
    printf("Introduce tu nombre de usuario: ");
    fgets(username, BUFFER_SIZE, stdin);
    username[strcspn(username, "\n")] = 0;

    printf("Introduce tu dirección IP: ");
    fgets(direccion, BUFFER_SIZE, stdin);
    direccion[strcspn(direccion, "\n")] = 0;

    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "tipo", "REGISTRO");
    cJSON_AddStringToObject(json, "usuario", username);
    cJSON_AddStringToObject(json, "direccionIP", direccion);
    char *json_str = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);

    if (send(sock, json_str, strlen(json_str), 0) < 0) {
        perror("Error al registrar usuario");
    } else {
        printf("Usuario '%s' registrado correctamente.\n", username);
    }
    free(json_str);

    // Crear hilo para recibir mensajes
    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, recibir_mensajes, NULL) < 0) {
        perror("Error al crear el hilo de recepción");
        return 1;
    }

    // Menú interactivo
    while (1) {
        printf("\nOpciones:\n");
        printf("1. Chatear con todos\n");
        printf("2. Chatear con un usuario (DM)\n");
        printf("3. Cambiar estado\n");
        printf("4. Listar usuarios\n");
        printf("5. Consultar información de un usuario\n");
        printf("6. Salir\n");
        printf("Selecciona una opción: ");
        int opcion;
        if (scanf("%d", &opcion) != 1) {
            printf("Entrada inválida.\n");
            while (getchar() != '\n'); // limpiar buffer
            continue;
        }
        getchar(); // limpiar salto de línea

        switch (opcion) {
            case 1: {
                printf("Escribe tu mensaje: ");
                char mensaje[BUFFER_SIZE];
                fgets(mensaje, BUFFER_SIZE, stdin);
                mensaje[strcspn(mensaje, "\n")] = 0;
                enviar_mensaje(mensaje, NULL);
                break;
            }
            case 2: {
                printf("Introduce el destinatario: ");
                char destinatario[BUFFER_SIZE];
                fgets(destinatario, BUFFER_SIZE, stdin);
                destinatario[strcspn(destinatario, "\n")] = 0;

                printf("Escribe tu mensaje: ");
                char mensaje[BUFFER_SIZE];
                fgets(mensaje, BUFFER_SIZE, stdin);
                mensaje[strcspn(mensaje, "\n")] = 0;
                enviar_mensaje(mensaje, destinatario);
                break;
            }
            case 3: {
                printf("Introduce tu nuevo estado (ACTIVO, OCUPADO, INACTIVO): ");
                char estado[BUFFER_SIZE];
                fgets(estado, BUFFER_SIZE, stdin);
                estado[strcspn(estado, "\n")] = 0;
                cambiar_estado(estado);
                break;
            }
            case 4:
                listar_usuarios();
                break;
            case 5:
                consultar_info_usuario();
                break;
            case 6: {
                // Enviar mensaje de salida
                cJSON *json_exit = cJSON_CreateObject();
                cJSON_AddStringToObject(json_exit, "tipo", "EXIT");
                cJSON_AddStringToObject(json_exit, "usuario", username);
                cJSON_AddStringToObject(json_exit, "estado", "");
                char *json_str_exit = cJSON_PrintUnformatted(json_exit);
                cJSON_Delete(json_exit);
                send(sock, json_str_exit, strlen(json_str_exit), 0);
                free(json_str_exit);

                printf("Saliendo...\n");
                close(sock);
                pthread_cancel(thread_id);
                return 0;
            }
            default:
                printf("Opción inválida\n");
                break;
        }
    }

    close(sock);
    return 0;
}
