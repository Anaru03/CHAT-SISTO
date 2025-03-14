#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"  //Cambiarlo para las pruebas
#define SERVER_PORT 27523 
#define BUFFER_SIZE 1024

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Error al crear el socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error al conectar al servidor");
        close(sock);
        exit(1);
    }

    printf("Conectado al servidor en %s:%d\n", SERVER_IP, SERVER_PORT);

    while (1) {
        printf("Escribe un mensaje (o 'salir' para terminar): ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0;  // Elimina el salto de línea

        if (send(sock, buffer, strlen(buffer), 0) == -1) {
            perror("Error al enviar mensaje");
            break;
        }

        if (strcmp(buffer, "salir") == 0) {
            printf("Desconectando...\n");
            break;
        }

        memset(buffer, 0, BUFFER_SIZE);
        if (recv(sock, buffer, BUFFER_SIZE, 0) <= 0) {
            perror("Error al recibir respuesta");
            break;
        }
        printf("Servidor: %s\n", buffer);
    }

    close(sock);
    return 0;
}
