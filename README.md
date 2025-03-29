# Proyecto de Chat en C

## Universidad del Valle de Guatemala  
**Curso:** Sistemas Operativos  
**Docentes:** Juan Luis García, Sebastián Galindo, Juan Carlos Canteo  

### Descripción
Este proyecto es una implementación de un sistema de chat en C, basado en el trabajo original de Bob Dugan y Erik Véliz (2006). El objetivo es reforzar conocimientos sobre procesos, threads, concurrencia y comunicación entre procesos mediante sockets en Linux.

El chat se compone de dos partes:
- **Servidor:** Mantiene una lista de usuarios conectados y gestiona la comunicación entre ellos.
- **Cliente:** Permite a los usuarios conectarse, enviar y recibir mensajes, y gestionar su estado.

## Funcionalidades
### Servidor
- Registro de usuarios con validación de nombres únicos.
- Gestión de conexiones concurrentes mediante multithreading.
- Listado de usuarios conectados.
- Información de usuarios (IP de clientes conectados).
- Cambio de estado de usuarios (ACTIVO, OCUPADO, INACTIVO).
- Manejo de mensajes directos y broadcasting.

### Cliente
- Conexión al servidor con registro de usuario.
- Envío y recepción de mensajes (chat general y privado).
- Cambio de estado.
- Consulta de usuarios conectados e información de un usuario.
- Comando de ayuda y salida.

## Instalación y Ejecución

### Compilación y Ejecución del Servidor
```sh
$ gcc serverW.c ../cJSON/cJSON.c -I "../cJSON" -o serverW.exe -lws2_32
$ ./serverW.exe 50213  # 50213 es el puerto del servidor
```

### Compilación y Ejecución del Cliente
```sh
$ gcc -Wall -pthread -I/mnt/c/SISTO/cjson client.c -o client -L/mnt/c/SISTO/cjson -lcjson
$ ./client
```
**Nota:** Ajustar la ruta de `cjson` según la ubicación en el sistema.

## Requisitos
- Sistema operativo Linux.
- Compilador `gcc`.
- Librería `pthread`.
- Librería `cJSON` para manejo de JSON.


## Notas Adicionales
- Se debe seguir el [protocolo](https://drive.google.com/file/d/1enn02xiu3lMSY2CubSH6e8PHvrm3cz4-/view?usp=sharing) de comunicación acordado en clase para garantizar la interoperabilidad entre servidores y clientes de distintos grupos.

**Autores:** 
- Isabella Miralles - [Isabella-22293](https://github.com/Isabella-22293) 
- Ruth de León - [Anaru03](https://github.com/Anaru03) 
