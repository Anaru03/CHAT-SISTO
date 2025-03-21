#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>             
#include <pthread.h>
#include <arpa/inet.h>          
#include <sys/socket.h>
#include <cJSON.h>

#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024

typedef struct {
    int socket;
    char username[50];
    char ip[16];      
    char status[10];
} Client;

Client clients[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void add_client(Client client) {
    pthread_mutex_lock(&clients_mutex);
    if (client_count < MAX_CLIENTS) {
        clients[client_count++] = client;
    } else {
        fprintf(stderr, "Se alcanzó el máximo de clientes.\n");
    }
    pthread_mutex_unlock(&clients_mutex);
}

void remove_client(int sock) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i].socket == sock) {
            clients[i] = clients[client_count - 1];
            client_count--;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

int usuario_existe(const char *username, const char *ip) {
    int existe = 0;
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (strcmp(clients[i].username, username) == 0 ||
            strcmp(clients[i].ip, ip) == 0) {
            existe = 1;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    return existe;
}

void enviar_mensaje(int sock, cJSON *mensaje) {
    char *string = cJSON_PrintUnformatted(mensaje);
    if (string != NULL) {
        send(sock, string, strlen(string), 0);
        free(string);
    }
}

void *handle_client(void *arg) {
    int client_socket = *(int*)arg;
    free(arg);
    char buffer[BUFFER_SIZE];
    int bytes_read;

    Client client_local;
    client_local.socket = client_socket;

    if ((bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytes_read] = '\0';
        cJSON *json = cJSON_Parse(buffer);
        if (json != NULL) {
            cJSON *tipo = cJSON_GetObjectItem(json, "tipo");
            if (tipo && strcmp(tipo->valuestring, "REGISTRO") == 0) {
                cJSON *usuario = cJSON_GetObjectItem(json, "usuario");
                cJSON *direccionIP = cJSON_GetObjectItem(json, "direccionIP");

                if (usuario && direccionIP) {
                    if (usuario_existe(usuario->valuestring, direccionIP->valuestring)) {
                        cJSON *respuesta = cJSON_CreateObject();
                        cJSON_AddStringToObject(respuesta, "respuesta", "ERROR");
                        cJSON_AddStringToObject(respuesta, "razon", "Nombre o dirección duplicado");
                        enviar_mensaje(client_socket, respuesta);
                        cJSON_Delete(respuesta);
                        cJSON_Delete(json);
                        close(client_socket);
                        pthread_exit(NULL);
                    } else {
                        strncpy(client_local.username, usuario->valuestring, sizeof(client_local.username) - 1);
                        strncpy(client_local.ip, direccionIP->valuestring, sizeof(client_local.ip) - 1);
                        strcpy(client_local.status, "ACTIVO");

                        add_client(client_local);

                        cJSON *respuesta = cJSON_CreateObject();
                        cJSON_AddStringToObject(respuesta, "response", "OK");
                        enviar_mensaje(client_socket, respuesta);
                        cJSON_Delete(respuesta);
                    }
                }
            }
            cJSON_Delete(json);
        }
    } else {
        close(client_socket);
        pthread_exit(NULL);
    }

    while ((bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytes_read] = '\0';
        cJSON *json = cJSON_Parse(buffer);
        if (json == NULL) {
            fprintf(stderr, "Error al parsear JSON del cliente.\n");
            continue;
        }
        
        cJSON *tipo = cJSON_GetObjectItem(json, "tipo");
        if (tipo && strcmp(tipo->valuestring, "EXIT") == 0) {
            remove_client(client_socket);
            cJSON_Delete(json);
            break;
        }
        
        cJSON *accion = cJSON_GetObjectItem(json, "accion");
        if (accion && strcmp(accion->valuestring, "LISTA") == 0) {
            cJSON *respuesta = cJSON_CreateObject();
            cJSON_AddStringToObject(respuesta, "accion", "LISTA");
            cJSON *usuarios = cJSON_CreateArray();
            pthread_mutex_lock(&clients_mutex);
            for (int i = 0; i < client_count; i++) {
                cJSON_AddItemToArray(usuarios, cJSON_CreateString(clients[i].username));
            }
            pthread_mutex_unlock(&clients_mutex);
            cJSON_AddItemToObject(respuesta, "usuarios", usuarios);
            enviar_mensaje(client_socket, respuesta);
            cJSON_Delete(respuesta);
        }
        
        cJSON_Delete(json);
    }

    remove_client(client_socket);
    close(client_socket);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <puertodelservidor>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]);

    int server_socket, new_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error al crear socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Error en setsockopt");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error en bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 10) < 0) {
        perror("Error en listen");
        exit(EXIT_FAILURE);
    }

    printf("Servidor ejecutándose en *:%d (accesible desde cualquier dispositivo en la red)\n", port);

    while ((new_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len)) >= 0) {
        printf("Nueva conexión desde %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        pthread_t tid;
        int *pclient = malloc(sizeof(int));
        *pclient = new_socket;
        if (pthread_create(&tid, NULL, &handle_client, pclient) != 0) {
            perror("Error al crear thread");
            free(pclient);
        }
        pthread_detach(tid);
    }

    close(server_socket);
    return 0;
}
