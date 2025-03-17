#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "cJSON.h"

#define PORT 50213
#define BUFFER_SIZE 1024

int main() {
    int sock;
    struct sockaddr_in server;
    char message[BUFFER_SIZE];

    // Crear socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("No se pudo crear el socket");
        return 1;
    }

    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    // Conectar al servidor
    int estado = connect(sock, (struct sockaddr *)&server, sizeof(server));
    if (estado < 0) {
        perror("Error al conectar al servidor");
        printf("%d\n", estado);
        return 1;
    }
    
    printf("Conectado al servidor en 127.0.0.1:%d\n", PORT);

    while (1) {
        printf("Escribe un mensaje (o 'salir' para terminar): ");
        fgets(message, BUFFER_SIZE, stdin);
        message[strcspn(message, "\n")] = 0; // Eliminar salto de línea

        if (strcmp(message, "salir") == 0) {
            break;
        }

        // Crear JSON
        cJSON *json = cJSON_CreateObject();
        cJSON_AddStringToObject(json, "mensaje", message);
        char *json_str = cJSON_PrintUnformatted(json);
        cJSON_Delete(json);

        // Enviar JSON
        if (send(sock, json_str, strlen(json_str), 0) < 0) {
            perror("Error al enviar mensaje");
            free(json_str);
            break;
        }

        free(json_str);
    }

    close(sock);
    return 0;
}
