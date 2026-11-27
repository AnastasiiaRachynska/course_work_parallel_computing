#include <iostream>
#include <cstring>
#include <fstream>
#include <sstream>
#include <thread>
#include <condition_variable>

#include "ApiHandler.h"
#include "InvertedIndex.h"
#include "Ollama.h"
#include "SearchSystem.h"
#include "ThreadPool.h"
#include "WebSocketSession.h"
#include <ixwebsocket/IXWebSocketServer.h>


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
#define WS_PORT 8081

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
        cerr << "Bind failed" << endl;
        close_socket(serverSocket);
        return 1;
    }
    if (listen(serverSocket, 5) == -1) {
        cerr << "Listen failed\n";
        close_socket(serverSocket);
        return 1;
    }
    cout << "Server listening on port " << PORT << "...\n";
    
    
    SearchSystem searchSystem;
    ThreadPool threadPool(16); 

    ix::WebSocketServer server(WS_PORT, "0.0.0.0");
    if(!WebSocketSession::start(server, searchSystem, threadPool)){
        close_socket(serverSocket);
        return 1;
    }

    while (true) {
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddr, &clientAddrLen);
        if (clientSocket == -1) {
            cerr << "Accept failed\n";
            continue;
        }
        cout << "Connection accepted from " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) <<endl;
        
        //хендлер запитів і відправка
        threadPool.add_task([clientSocket, &searchSystem](){
            char buffer[1024];
            string request;
            size_t headerEnd = request.find("\r\n\r\n");
            while(headerEnd == string::npos) {
                int received = recv(clientSocket, buffer, sizeof(buffer), 0);
                if (received <= 0){
                    cerr << "Failed to receive data\n";
                    break;
                } 
                request.append(buffer, received);
                headerEnd = request.find("\r\n\r\n");
            }
            

            if (headerEnd != string::npos) {
                size_t contentLength = 0;
                istringstream requestStream(request.substr(0, headerEnd));
                string line;
                while (getline(requestStream, line)){
                    transform(line.begin(), line.end(), line.begin(),
                        [](unsigned char c) { return tolower(c); });
                if (line.find("content-length:") == 0) {
                    contentLength = stoi(line.substr(15));
                    break;
                }
            }
                size_t bodyStart = headerEnd + 4;
                bool complete = true;
                while (request.size() - bodyStart < contentLength) {
                    int received = recv(clientSocket, buffer, sizeof(buffer), 0);
                    if (received <= 0) {
                        cerr << "Failed to receive body data\n";
                        complete = false;
                        break;
                    }
                    request.append(buffer, received);
                }
                if (complete) {
                    ApiHandler apiHandler(searchSystem);
                    apiHandler.handle_request(clientSocket, request);
                }
            }
            close_socket(clientSocket);
        });
    }

    close_socket(serverSocket);
    return 0;
}
