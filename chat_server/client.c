#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "cJSON.h"

#define PORT 50213
#define BUFFER_SIZE 1024

// Función para imprimir la lista de usuarios
void listar_usuarios(int sock) {
    // Crear JSON de solicitud para listar usuarios
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "accion", "listar_usuarios");
    char *json_str = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);

    // Enviar solicitud
    if (send(sock, json_str, strlen(json_str), 0) < 0) {
        perror("Error al solicitar listado de usuarios");
        free(json_str);
        return;
    }

    free(json_str);

    // Esperar respuesta (asumimos que el servidor devuelve el listado de usuarios)
    char response[BUFFER_SIZE];
    int len = recv(sock, response, sizeof(response) - 1, 0);
    if (len > 0) {
        response[len] = '\0';  // Asegurarse de que la cadena termine correctamente
        printf("Usuarios conectados:\n%s\n", response);
    } else {
        printf("Error al recibir la lista de usuarios\n");
    }
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

int main() {
    int sock;
    struct sockaddr_in server;
    char message[BUFFER_SIZE];
    char username[BUFFER_SIZE];

    // Crear socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("No se pudo crear el socket");
        return 1;
    }

    server.sin_addr.s_addr = inet_addr("192.168.229.71");  // Dirección IP del servidor
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    // Conectar al servidor
    int estado = connect(sock, (struct sockaddr *)&server, sizeof(server));
    if (estado < 0) {
        perror("Error al conectar al servidor");
        return 1;
    }

    // Solicitar nombre de usuario
    printf("Introduce tu nombre de usuario: ");
    fgets(username, BUFFER_SIZE, stdin);
    username[strcspn(username, "\n")] = 0;  // Eliminar salto de línea

    // Crear JSON para registrar usuario
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "accion", "REGISTRO");
    cJSON_AddStringToObject(json, "usuario", username);
    char *json_str = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);

    // Enviar nombre de usuario al servidor
    if (send(sock, json_str, strlen(json_str), 0) < 0) {
        perror("Error al registrar usuario");
        free(json_str);
        close(sock);
        return 1;
    }

    free(json_str);

    // Confirmación de registro
    printf("Usuario '%s' registrado correctamente.\n", username);

    while (1) {
        // Mostrar menú de opciones
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
        getchar();  // Limpiar el buffer del salto de línea

        switch (opcion) {
            case 1:
                // Enviar mensaje a todos
                printf("Escribe tu mensaje: ");
                fgets(message, BUFFER_SIZE, stdin);
                message[strcspn(message, "\n")] = 0;  // Eliminar salto de línea
                enviar_mensaje(sock, message, NULL);  // NULL para enviar al chat general
                break;

            case 2:
                // Enviar mensaje privado
                printf("A quién deseas enviar el mensaje? ");
                char destinatario[BUFFER_SIZE];
                fgets(destinatario, BUFFER_SIZE, stdin);
                destinatario[strcspn(destinatario, "\n")] = 0;  // Eliminar salto de línea
                printf("Escribe tu mensaje: ");
                fgets(message, BUFFER_SIZE, stdin);
                message[strcspn(message, "\n")] = 0;  // Eliminar salto de línea
                enviar_mensaje(sock, message, destinatario);
                break;

            case 3:
                // Cambiar estado
                printf("Selecciona un estado: 1. ACTIVO 2. OCUPADO 3. INACTIVO\n");
                int estado_opcion;
                scanf("%d", &estado_opcion);
                getchar();  // Limpiar el buffer del salto de línea

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
                // Listar usuarios conectados
                listar_usuarios(sock);
                break;

            case 5:
                // Consultar información de un usuario
                printf("Introduce el nombre del usuario: ");
                char usuario_info[BUFFER_SIZE];
                fgets(usuario_info, BUFFER_SIZE, stdin);
                usuario_info[strcspn(usuario_info, "\n")] = 0;  // Eliminar salto de línea

                // Crear JSON para consulta de usuario
                json = cJSON_CreateObject();
                cJSON_AddStringToObject(json, "accion", "info_usuario");
                cJSON_AddStringToObject(json, "usuario", usuario_info);
                json_str = cJSON_PrintUnformatted(json);
                cJSON_Delete(json);

                // Enviar solicitud
                if (send(sock, json_str, strlen(json_str), 0) < 0) {
                    perror("Error al consultar información de usuario");
                    free(json_str);
                    break;
                }

                free(json_str);

                // Recibir respuesta con la información del usuario
                char response[BUFFER_SIZE];
                int len = recv(sock, response, sizeof(response) - 1, 0);
                if (len > 0) {
                    response[len] = '\0';  // Asegurarse de que la cadena termine correctamente
                    printf("Información del usuario: %s\n", response);
                } else {
                    printf("Error al recibir la información del usuario\n");
                }
                break;

            case 6:
                // Salir
                printf("Saliendo...\n");

                // Crear JSON para notificar salida
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
