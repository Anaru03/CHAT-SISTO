#include <stdio.h>    
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <stdlib.h>
    #include <string.h>
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <pthread.h>
#endif

#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024

// Estructura para almacenar la información de cada cliente
typedef struct {
    int socket;
    char username[50];
    char ip[16];
    char status[10];  // "ACTIVO", "OCUPADO", "INACTIVO"
} Client;

Client clients[MAX_CLIENTS];
int client_count = 0;
#ifdef _WIN32
    HANDLE clients_mutex = NULL;
#else
    pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

void add_client(Client client) {
#ifdef _WIN32
    WaitForSingleObject(clients_mutex, INFINITE);
#else
    pthread_mutex_lock(&clients_mutex);
#endif
    if (client_count < MAX_CLIENTS) {
        clients[client_count++] = client;
    } else {
        fprintf(stderr, "Se alcanzó el máximo de clientes.\n");
    }
#ifdef _WIN32
    ReleaseMutex(clients_mutex);
#else
    pthread_mutex_unlock(&clients_mutex);
#endif
}

void remove_client(int sock) {
#ifdef _WIN32
    WaitForSingleObject(clients_mutex, INFINITE);
#else
    pthread_mutex_lock(&clients_mutex);
#endif
    for (int i = 0; i < client_count; i++) {
        if (clients[i].socket == sock) {
            clients[i] = clients[client_count - 1];
            client_count--;
            break;
        }
    }
#ifdef _WIN32
    ReleaseMutex(clients_mutex);
#else
    pthread_mutex_unlock(&clients_mutex);
#endif
}

void enviar_mensaje(int sock, const char *mensaje) {
    send(sock, mensaje, strlen(mensaje), 0);
}

#ifdef _WIN32
DWORD WINAPI handle_client(LPVOID arg) {
#else
void *handle_client(void *arg) {
#endif
    int client_socket = *(int*)arg;
    free(arg);
    char buffer[BUFFER_SIZE];
    int bytes_read;

    while ((bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytes_read] = '\0';
        printf("Mensaje recibido: %s\n", buffer);
    }

    remove_client(client_socket);
#ifdef _WIN32
    closesocket(client_socket);
    return 0;
#else
    close(client_socket);
    return NULL;
#endif
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <puertodelservidor>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]);

#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Error al inicializar Winsock\n");
        return 1;
    }
    clients_mutex = CreateMutex(NULL, FALSE, NULL);
#endif

    int server_socket, new_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error al crear socket");
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

    printf("Servidor ejecutandose en:%d\n", port);

    while ((new_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len)) >= 0) {
        printf("Nueva conexión: %s\n", inet_ntoa(client_addr.sin_addr));

#ifdef _WIN32
        HANDLE thread;
        int *new_sock = malloc(sizeof(int));
        *new_sock = new_socket;
        thread = CreateThread(NULL, 0, handle_client, (void*)new_sock, 0, NULL);
#else
        pthread_t thread;
        int *new_sock = malloc(sizeof(int));
        *new_sock = new_socket;
        if (pthread_create(&thread, NULL, handle_client, (void*)new_sock) < 0) {
            perror("Error al crear thread");
            close(new_socket);
        }
#endif
    }

#ifdef _WIN32
    closesocket(server_socket);
    WSACleanup();
#else
    close(server_socket);
#endif

    return 0;
}
