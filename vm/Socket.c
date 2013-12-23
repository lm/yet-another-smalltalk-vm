#include "Socket.h"
#include "Assert.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>


int socketConnect(uint32_t ip, uint16_t port)
{
	int descriptor = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in address;

	if (descriptor < 0) {
		return -1;
	}

	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	memcpy(&address.sin_addr, &ip, sizeof(ip));

	if (connect(descriptor, (struct sockaddr *) &address, sizeof(address)) != 0) {
		close(descriptor);
		return -1;
	} else {
		return descriptor;
	}
}


int socketBind(uint32_t ip, uint16_t port, int backlog)
{
	int descriptor = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in address;

	if (descriptor < 0) {
		return -1;
	}

	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	memcpy(&address.sin_addr, &ip, sizeof(ip));
	//address.sin_addr.s_addr = INADDR_ANY;

	if (bind(descriptor, (struct sockaddr *) &address, sizeof(address)) != 0) {
		close(descriptor);
		return -1;
	}
	if (listen(descriptor, backlog) != 0) {
		close(descriptor);
		return -1;
	}
	return descriptor;
}


int socketAccept(int descriptor)
{
	return accept(descriptor, NULL, 0);
}


uint32_t socketHostLookup(char *host, const char **error)
{
	struct addrinfo hints;
	struct addrinfo *addr;

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	int result = getaddrinfo(host, NULL, &hints, &addr);
	if (result == 0) {
		ASSERT(addr->ai_addr->sa_family == AF_INET);
		uint32_t ip;
		memcpy(&ip, &((struct sockaddr_in *) addr->ai_addr)->sin_addr, sizeof(ip));
		*error = NULL;
		freeaddrinfo(addr);
		return ip;
	} else {
		*error = gai_strerror(result);
		return 0;
	}
}
