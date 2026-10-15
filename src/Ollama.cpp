#include "Ollama.h"
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

inline void close_socket_ollama(int sock) {
#ifdef _WIN32
    closesocket(sock);
#else
    close(sock);
#endif
}

bool Ollama::is_working() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) return false;

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(11434);

    struct hostent* host = gethostbyname("ollama");
    if (host == nullptr) {
        close_socket_ollama(sock);
        return false;
    }

    memcpy(&addr.sin_addr, host->h_addr_list[0], host->h_length);

    if (connect(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        close_socket_ollama(sock);
        return false;
    }
    close_socket_ollama(sock);
    return true;
}
