#include <iostream>
#include <cstring>
#include <fstream>
#include <sstream>
#include <queue>
#include <thread>
#include <mutex>
#include <vector>
#include <condition_variable>

#include "ApiHandler.h"
#include "InvertedIndex.h"
#include "Ollama.h"
#include "SearchSystem.h"
#include "ThreadPool.h"
#include "WebSocketSession.h"

// додано ifdef, оскільки моя ос windows, а компіляція й робота сервера буде на ubuntu (докер)
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#define PORT 8080

using namespace std;

void close_socket(int sock) {
#ifdef _WIN32
    closesocket(sock);
#else
    close(sock);
#endif
}

int main() {
    int serverSocket;
    struct sockaddr_in serverAddr;
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        cerr << "Socket creation failed\n";
        return 1;
    }
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) == -1) {
        cerr << "Bind failed\n";
        close_socket(serverSocket);
        return 1;
    }
    if (listen(serverSocket, 5) == -1) {
        cerr << "Listen failed\n";
        close_socket(serverSocket);
        return 1;
    }
    cout << "Server listening on port " << PORT << "...\n";
    Ollama ollama;
    SearchSystem searchSystem(ollama);
    while (true) {
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddr,
                                  &clientAddrLen);
        if (clientSocket == -1) {
            cerr << "Accept failed\n";
            continue;
        }
        cout << "Connection accepted from " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) <<endl;
        //хендлер запитів і відправка
        char buffer[1024] = {0};
        int received = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (received <= 0) {
            close_socket(clientSocket);
            continue;
        }
        ApiHandler apiHandler(searchSystem);
        string request = buffer;
        apiHandler.handle_request(clientSocket, request);
    }

    close_socket(serverSocket);
    return 0;
}
