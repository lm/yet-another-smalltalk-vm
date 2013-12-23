#ifndef SOCKET_H
#define SOCKET_H

#include "Object.h"
#include <stdint.h>

typedef struct {
	OBJECT_HEADER;
	Value descriptor;
} RawServerSocket;
OBJECT_HANDLE(ServerSocket);

typedef struct {
	OBJECT_HEADER;
	Value address;
} RawInternetAddress;
OBJECT_HANDLE(InternetAddress);

int socketConnect(uint32_t ip, uint16_t port);
int socketBind(uint32_t ip, uint16_t port, int backlog);
int socketAccept(int descriptor);
uint32_t socketHostLookup(char *host, const char **error);

#endif
