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

// Función para enviar un mensaje a un usuario específico o a todos
void enviar_mensaje(int sock, const char *mensaje, const char *destinatario) {
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "accion", "enviar_mensaje");

    if (destinatario) {
        cJSON_AddStringToObject(json, "destinatario", destinatario);
    }
    cJSON_AddStringToObject(json, "mensaje", mensaje);
    char *json_str = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);

    // Enviar mensaje
    if (send(sock, json_str, strlen(json_str), 0) < 0) {
        perror("Error al enviar mensaje");
        free(json_str);
        return;
    }

    free(json_str);
}

// Función para cambiar el estado del cliente
void cambiar_estado(int sock, const char *nuevo_estado) {
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "accion", "cambiar_estado");
    cJSON_AddStringToObject(json, "estado", nuevo_estado);
    char *json_str = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);

    // Enviar solicitud para cambiar el estado
    if (send(sock, json_str, strlen(json_str), 0) < 0) {
        perror("Error al cambiar estado");
        free(json_str);
        return;
    }

    free(json_str);
    printf("Estado cambiado a: %s\n", nuevo_estado);
}

// Hilo para recibir mensajes en tiempo real
void *recibir_mensajes(void *arg) {
    char response[BUFFER_SIZE];

    while (1) {
        int len = recv(sock, response, sizeof(response) - 1, 0);
        if (len > 0) {
            response[len] = '\0';
            printf("\n\nMensaje recibido: %s\n", response);
            printf("\nSelecciona una opción: ");
            fflush(stdout);
        }
    }
    return NULL;
}

// Función para listar usuarios conectados
void listar_usuarios(int sock) {
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "accion", "listar_usuarios");
    char *json_str = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);

    if (send(sock, json_str, strlen(json_str), 0) < 0) {
        perror("Error al solicitar listado de usuarios");
        free(json_str);
        return;
    }
    free(json_str);

    char response[BUFFER_SIZE];
    int len = recv(sock, response, sizeof(response) - 1, 0);
    if (len > 0) {
        response[len] = '\0';
        printf("Usuarios conectados:\n%s\n", response);
    } else {
        printf("Error al recibir la lista de usuarios\n");
    }
}

// Función para consultar información de un usuario
void consultar_info_usuario(int sock) {
    printf("Introduce el nombre del usuario: ");
    char usuario_info[BUFFER_SIZE];
    fgets(usuario_info, BUFFER_SIZE, stdin);
    usuario_info[strcspn(usuario_info, "\n")] = 0;

    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "accion", "info_usuario");
    cJSON_AddStringToObject(json, "usuario", usuario_info);
    char *json_str = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);

    if (send(sock, json_str, strlen(json_str), 0) < 0) {
        perror("Error al consultar información de usuario");
        free(json_str);
        return;
    }
    free(json_str);

    char response[BUFFER_SIZE];
    int len = recv(sock, response, sizeof(response) - 1, 0);
    if (len > 0) {
        response[len] = '\0';
        printf("Información del usuario: %s\n", response);
    } else {
        printf("Error al recibir la información del usuario\n");
    }
}

int main() {
    struct sockaddr_in server;
    char message[BUFFER_SIZE];
    char username[BUFFER_SIZE];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("No se pudo crear el socket");
        return 1;
    }

    server.sin_addr.s_addr = inet_addr("192.168.229.71");
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Error al conectar al servidor");
        return 1;
    }

    printf("Introduce tu nombre de usuario: ");
    fgets(username, BUFFER_SIZE, stdin);
    username[strcspn(username, "\n")] = 0;

    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "accion", "REGISTRO");
    cJSON_AddStringToObject(json, "usuario", username);
    char *json_str = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);

    if (send(sock, json_str, strlen(json_str), 0) < 0) {
        perror("Error al registrar usuario");
        free(json_str);
        close(sock);
        return 1;
    }
    free(json_str);

    printf("Usuario '%s' registrado correctamente.\n", username);

    // Crear hilo para recibir mensajes
    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, recibir_mensajes, NULL) < 0) {
        perror("Error al crear el hilo de recepción");
        return 1;
    }

    while (1) {
        printf("\nOpciones:\n");
        printf("1. Chatear con todos (broadcast)\n");
        printf("2. Chatear con un usuario (mensaje privado)\n");
        printf("3. Cambiar estado (ACTIVO, OCUPADO, INACTIVO)\n");
        printf("4. Listar usuarios conectados\n");
        printf("5. Consultar información de un usuario\n");
        printf("6. Salir\n");

        printf("Selecciona una opción: ");
        int opcion;
        scanf("%d", &opcion);
        getchar();

        switch (opcion) {
            case 1:
                printf("Escribe tu mensaje: ");
                fgets(message, BUFFER_SIZE, stdin);
                message[strcspn(message, "\n")] = 0;
                enviar_mensaje(sock, message, NULL);
                break;

            case 2: {
                printf("A quién deseas enviar el mensaje? ");
                char destinatario[BUFFER_SIZE];
                fgets(destinatario, BUFFER_SIZE, stdin);
                destinatario[strcspn(destinatario, "\n")] = 0;
                printf("Escribe tu mensaje: ");
                fgets(message, BUFFER_SIZE, stdin);
                message[strcspn(message, "\n")] = 0;
                enviar_mensaje(sock, message, destinatario);
                break;
            }

            case 3:
                printf("Selecciona un estado: 1. ACTIVO 2. OCUPADO 3. INACTIVO\n");
                int estado_opcion;
                scanf("%d", &estado_opcion);
                getchar();

                if (estado_opcion == 1) {
                    cambiar_estado(sock, "ACTIVO");
                } else if (estado_opcion == 2) {
                    cambiar_estado(sock, "OCUPADO");
                } else if (estado_opcion == 3) {
                    cambiar_estado(sock, "INACTIVO");
                } else {
                    printf("Opción inválida\n");
                }
                break;

            case 4:
                listar_usuarios(sock);
                break;

            case 5:
                consultar_info_usuario(sock);
                break;

            case 6:
                printf("Saliendo...\n");
                json = cJSON_CreateObject();
                cJSON_AddStringToObject(json, "accion", "salir");
                cJSON_AddStringToObject(json, "usuario", username);
                json_str = cJSON_PrintUnformatted(json);
                cJSON_Delete(json);

                if (send(sock, json_str, strlen(json_str), 0) < 0) {
                    perror("Error al notificar salida");
                }
                free(json_str);

                close(sock);
                return 0;

            default:
                printf("Opción inválida\n");
                break;
        }
    }

    close(sock);
    return 0;
}
