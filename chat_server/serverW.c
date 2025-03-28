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

#include "cJSON.h" 

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

// Declaración adelantada para evitar el warning en timer_thread
Client* find_client_by_username(const char* username);

// Función timer que espera 1 minuto y cambia el estado del cliente a "INACTIVO"
#ifdef _WIN32
DWORD WINAPI timer_thread(LPVOID arg) {
    char* username = (char*) arg;
    Sleep(60000);  // Espera 60 segundos
    Client *cl = find_client_by_username(username);
    if(cl) {
         strncpy(cl->status, "INACTIVO", sizeof(cl->status)-1);
    }
    free(username);
    return 0;
}
#else
void *timer_thread(void *arg) {
    char* username = (char*) arg;
    sleep(60);  // Espera 60 segundos
    Client *cl = find_client_by_username(username);
    if(cl) {
         strncpy(cl->status, "INACTIVO", sizeof(cl->status)-1);
    }
    free(username);
    return NULL;
}
#endif

// Funciones de manejo de clientes

void add_client(Client client) {
#ifdef _WIN32
    WaitForSingleObject(clients_mutex, INFINITE);
#else
    pthread_mutex_lock(&clients_mutex);
#endif
    if (client_count < MAX_CLIENTS) {
        clients[client_count++] = client;
    } else {
        fprintf(stderr, "Se alcanzo el maximo de clientes.\n");
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

Client* find_client_by_username(const char* username) {
#ifdef _WIN32
    WaitForSingleObject(clients_mutex, INFINITE);
#else
    pthread_mutex_lock(&clients_mutex);
#endif
    Client* result = NULL;
    for (int i = 0; i < client_count; i++) {
        if (strcmp(clients[i].username, username) == 0) {
            result = &clients[i];
            break;
        }
    }
#ifdef _WIN32
    ReleaseMutex(clients_mutex);
#else
    pthread_mutex_unlock(&clients_mutex);
#endif
    return result;
}

// Envía un mensaje (cadena) a un socket dado.
void enviar_mensaje(int sock, const char *mensaje) {
    send(sock, mensaje, strlen(mensaje), 0);
}

// Función para enviar mensaje JSON al cliente
void enviar_JSON(int sock, cJSON *json) {
    char *mensaje = cJSON_PrintUnformatted(json);
    if(mensaje) {
        enviar_mensaje(sock, mensaje);
        free(mensaje);
    }
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
    int registered = 0;  // Bandera para ver si ya se registró el cliente
    Client client_info;
    memset(&client_info, 0, sizeof(Client));
    client_info.socket = client_socket;
    // Se asume que la IP se puede obtener del socket, pero en registro se recibe también

    while ((bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytes_read] = '\0';
        printf("Mensaje recibido: %s\n", buffer);

        cJSON *json = cJSON_Parse(buffer);
        if (!json) {
            fprintf(stderr, "Error al parsear JSON\n");
            continue;
        }

        // Se verifica si viene el campo "tipo"
        cJSON *tipo = cJSON_GetObjectItemCaseSensitive(json, "tipo");
        // Si no, se chequea "accion"
        cJSON *accion = NULL;
        if (!tipo) {
            accion = cJSON_GetObjectItemCaseSensitive(json, "accion");
        }

        // REGISTRO
        if (tipo && cJSON_IsString(tipo) && strcmp(tipo->valuestring, "REGISTRO") == 0) {
            if (registered) {
                // Ya registrado, no se procesa de nuevo
                cJSON_Delete(json);
                continue;
            }
            cJSON *usuario = cJSON_GetObjectItemCaseSensitive(json, "usuario");
            cJSON *direccionIP = cJSON_GetObjectItemCaseSensitive(json, "direccionIP");
            if (!cJSON_IsString(usuario) || !cJSON_IsString(direccionIP)) {
                cJSON *resp = cJSON_CreateObject();
                cJSON_AddStringToObject(resp, "respuesta", "ERROR");
                cJSON_AddStringToObject(resp, "razon", "Formato inválido en REGISTRO");
                enviar_JSON(client_socket, resp);
                cJSON_Delete(resp);
            } else {
                // Validar duplicados en username o dirección IP
                int duplicado = 0;
#ifdef _WIN32
                WaitForSingleObject(clients_mutex, INFINITE);
#else
                pthread_mutex_lock(&clients_mutex);
#endif
                for (int i = 0; i < client_count; i++) {
                    if (strcmp(clients[i].username, usuario->valuestring) == 0 ||
                        strcmp(clients[i].ip, direccionIP->valuestring) == 0) {
                        duplicado = 1;
                        break;
                    }
                }
#ifdef _WIN32
                ReleaseMutex(clients_mutex);
#else
                pthread_mutex_unlock(&clients_mutex);
#endif
                if (duplicado) {
                    cJSON *resp = cJSON_CreateObject();
                    cJSON_AddStringToObject(resp, "respuesta", "ERROR");
                    cJSON_AddStringToObject(resp, "razon", "Nombre o dirección duplicado");
                    enviar_JSON(client_socket, resp);
                    cJSON_Delete(resp);
                } else {
                    // Registro exitoso: se asigna el estado inicial "ACTIVO"
                    strncpy(client_info.username, usuario->valuestring, sizeof(client_info.username)-1);
                    strncpy(client_info.ip, direccionIP->valuestring, sizeof(client_info.ip)-1);
                    strncpy(client_info.status, "ACTIVO", sizeof(client_info.status)-1);
                    add_client(client_info);
                    registered = 1;
                    cJSON *resp = cJSON_CreateObject();
                    cJSON_AddStringToObject(resp, "response", "OK");
                    enviar_JSON(client_socket, resp);
                    cJSON_Delete(resp);
                    
                    // Agregar timer de 1 minuto para cambiar estado a INACTIVO
#ifdef _WIN32
                    DWORD threadId;
                    char *userForTimer = _strdup(client_info.username);
                    HANDLE hTimer = CreateThread(NULL, 0, timer_thread, userForTimer, 0, &threadId);
#else
                    pthread_t timer_tid;
                    char *userForTimer = strdup(client_info.username);
                    pthread_create(&timer_tid, NULL, timer_thread, userForTimer);
#endif
                }
            }
        }
        // Procesar mensajes basados en "tipo"
        else if (tipo && cJSON_IsString(tipo)) {
            if (strcmp(tipo->valuestring, "EXIT") == 0) {
                // Liberar usuario y salir
                remove_client(client_socket);
                cJSON_Delete(json);
                break;
            }
            else if (strcmp(tipo->valuestring, "ESTADO") == 0) {
                cJSON *usuario = cJSON_GetObjectItemCaseSensitive(json, "usuario");
                cJSON *estado = cJSON_GetObjectItemCaseSensitive(json, "estado");
                if (!cJSON_IsString(usuario) || !cJSON_IsString(estado)) {
                    cJSON *resp = cJSON_CreateObject();
                    cJSON_AddStringToObject(resp, "respuesta", "ERROR");
                    cJSON_AddStringToObject(resp, "razon", "Formato inválido en ESTADO");
                    enviar_JSON(client_socket, resp);
                    cJSON_Delete(resp);
                } else {
                    // Validar que el estado sea uno de los permitidos: "ACTIVO", "OCUPADO" o "INACTIVO"
                    if (strcmp(estado->valuestring, "ACTIVO") != 0 &&
                        strcmp(estado->valuestring, "OCUPADO") != 0 &&
                        strcmp(estado->valuestring, "INACTIVO") != 0) {
                        cJSON *resp = cJSON_CreateObject();
                        cJSON_AddStringToObject(resp, "respuesta", "ERROR");
                        cJSON_AddStringToObject(resp, "razon", "Estado inválido");
                        enviar_JSON(client_socket, resp);
                        cJSON_Delete(resp);
                    } else {
                        // Si el usuario coincide con el cliente registrado, actualizar globalmente
                        if (strcmp(usuario->valuestring, client_info.username) == 0) {
                            Client *cl = find_client_by_username(client_info.username);
                            if (cl) {
                                if (strcmp(cl->status, estado->valuestring) == 0) {
                                    cJSON *resp = cJSON_CreateObject();
                                    cJSON_AddStringToObject(resp, "respuesta", "ERROR");
                                    cJSON_AddStringToObject(resp, "razon", "ESTADO_YA_SELECCIONADO");
                                    enviar_JSON(client_socket, resp);
                                    cJSON_Delete(resp);
                                } else {
                                    strncpy(cl->status, estado->valuestring, sizeof(cl->status)-1);
                                    // También actualizamos la copia local
                                    strncpy(client_info.status, estado->valuestring, sizeof(client_info.status)-1);
                                    cJSON *resp = cJSON_CreateObject();
                                    cJSON_AddStringToObject(resp, "respuesta", "OK");
                                    enviar_JSON(client_socket, resp);
                                    cJSON_Delete(resp);
                                }
                            }
                        }
                    }
                }
            }
            else if (strcmp(tipo->valuestring, "MOSTRAR") == 0) {
                cJSON *usuario = cJSON_GetObjectItemCaseSensitive(json, "usuario");
                if (!cJSON_IsString(usuario)) {
                    cJSON *resp = cJSON_CreateObject();
                    cJSON_AddStringToObject(resp, "respuesta", "ERROR");
                    cJSON_AddStringToObject(resp, "razon", "Formato inválido en MOSTRAR");
                    enviar_JSON(client_socket, resp);
                    cJSON_Delete(resp);
                } else {
                    Client *dest = find_client_by_username(usuario->valuestring);
                    if (dest) {
                        cJSON *resp = cJSON_CreateObject();
                        cJSON_AddStringToObject(resp, "tipo", "MOSTRAR");
                        cJSON_AddStringToObject(resp, "usuario", dest->username);
                        cJSON_AddStringToObject(resp, "estado", dest->status);
                        cJSON_AddStringToObject(resp, "ip", dest->ip);
                        enviar_JSON(client_socket, resp);
                        cJSON_Delete(resp);
                    } else {
                        cJSON *resp = cJSON_CreateObject();
                        cJSON_AddStringToObject(resp, "respuesta", "ERROR");
                        cJSON_AddStringToObject(resp, "razon", "USUARIO_NO_ENCONTRADO");
                        enviar_JSON(client_socket, resp);
                        cJSON_Delete(resp);
                    }
                }
            }
        }
        // Procesar mensajes basados en "accion"
        else if (accion && cJSON_IsString(accion)) {
            if (strcmp(accion->valuestring, "BROADCAST") == 0) {
                // Se espera que el JSON contenga: nombre_emisor y mensaje
                cJSON *nombre_emisor = cJSON_GetObjectItemCaseSensitive(json, "nombre_emisor");
                cJSON *mensaje = cJSON_GetObjectItemCaseSensitive(json, "mensaje");
                if (cJSON_IsString(nombre_emisor) && cJSON_IsString(mensaje)) {
#ifdef _WIN32
                    WaitForSingleObject(clients_mutex, INFINITE);
#else
                    pthread_mutex_lock(&clients_mutex);
#endif
                    // Enviar a todos los clientes conectados
                    for (int i = 0; i < client_count; i++) {
                        enviar_mensaje(clients[i].socket, buffer);
                    }
#ifdef _WIN32
                    ReleaseMutex(clients_mutex);
#else
                    pthread_mutex_unlock(&clients_mutex);
#endif
                }
            }
            else if (strcmp(accion->valuestring, "DM") == 0) {
                // Mensaje directo: se esperan: nombre_emisor, nombre_destinatario y mensaje
                cJSON *nombre_emisor = cJSON_GetObjectItemCaseSensitive(json, "nombre_emisor");
                cJSON *nombre_destinatario = cJSON_GetObjectItemCaseSensitive(json, "nombre_destinatario");
                cJSON *mensaje = cJSON_GetObjectItemCaseSensitive(json, "mensaje");
                if (cJSON_IsString(nombre_emisor) && cJSON_IsString(nombre_destinatario) && cJSON_IsString(mensaje)) {
                    Client *dest = find_client_by_username(nombre_destinatario->valuestring);
                    if (dest) {
                        enviar_mensaje(dest->socket, buffer);
                    } else {
                        cJSON *resp = cJSON_CreateObject();
                        cJSON_AddStringToObject(resp, "respuesta", "ERROR");
                        cJSON_AddStringToObject(resp, "razon", "USUARIO_NO_ENCONTRADO");
                        enviar_JSON(client_socket, resp);
                        cJSON_Delete(resp);
                    }
                }
            }
            else if (strcmp(accion->valuestring, "LISTA") == 0) {
                // Se envía un listado de usuarios conectados con su IP y status
#ifdef _WIN32
                    WaitForSingleObject(clients_mutex, INFINITE);
#else
                    pthread_mutex_lock(&clients_mutex);
#endif
                cJSON *lista = cJSON_CreateArray();
                for (int i = 0; i < client_count; i++) {
                    cJSON *user_obj = cJSON_CreateObject();
                    cJSON_AddStringToObject(user_obj, "usuarios", clients[i].username);
                    cJSON_AddStringToObject(user_obj, "ip", clients[i].ip);
                    cJSON_AddStringToObject(user_obj, "status", clients[i].status);
                    cJSON_AddItemToArray(lista, user_obj);
                }
#ifdef _WIN32
                    ReleaseMutex(clients_mutex);
#else
                    pthread_mutex_unlock(&clients_mutex);
#endif
                cJSON *resp = cJSON_CreateObject();
                cJSON_AddStringToObject(resp, "accion", "LISTA");
                cJSON_AddItemToObject(resp, "usuarios", lista);
                enviar_JSON(client_socket, resp);
                cJSON_Delete(resp);
            }
        }
        cJSON_Delete(json);
    }

    // Al salir del loop se cierra la conexión
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

    printf("Servidor ejecutandose en el puerto: %d\n", port);

    while ((new_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len)) >= 0) {
        printf("Nueva conexion desde: %s\n", inet_ntoa(client_addr.sin_addr));
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
