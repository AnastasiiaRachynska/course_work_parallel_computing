#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include <sstream>
using json = nlohmann::json;

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


int main(int argc, char* argv[]) {

    string method, path, body_str;
    if(argc == 3 && string(argv[1]) == "--index") {
        stringstream ss(argv[2]);
        int threads;
        if(!(ss >> threads) || threads <= 0) {
            cerr << "Invalid number of threads" << endl;
            return 1;
        }
        method = "POST";
        path = "/api/index";
        json body = json{{"threads", threads}};
        body_str = body.dump();
    }
    else if(argc == 2 && string(argv[1]) == "--stats") {
        method = "GET";
        path = "/api/stats";
    }
    else if(argc == 2 && string(argv[1]) == "--health") {
        method = "GET";
        path = "/api/health";
    }
    else if(argc == 2 && !string(argv[1]).empty() &&
             string(argv[1]).find("--") != 0) {
        method = "POST";
        path = "/api/search";
        json body = json{{"query", string(argv[1])}};
        body_str = body.dump();
    }
    else {
        cerr << "Invalid arguments" << endl;
        return 1;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0) {
        cerr << "Failed to create socket" << endl;
        return 1;
    }

    do{
        struct sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        server_addr.sin_port = htons(PORT);
        if(connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            cerr << "Failed to connect to server" << endl;
            close_socket(sock);
            return 1;
        }

        string request = method + " " + path + " HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: " + to_string(body_str.size()) + "\r\nConnection: close\r\n\r\n" + body_str;

        int result = send(sock, request.c_str(), request.size(), 0);
        if(result < 0) {
            cerr << "Failed to send request" << endl;
            close_socket(sock);
            return 1;
        }

        string response;
        char buffer[1024];
        int bytes_received;
        while((bytes_received = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
            response.append(buffer, bytes_received);
        }
        if(bytes_received < 0) {
            cerr << "Failed to receive response" << endl;
            close_socket(sock);
            return 1;
        }
        
        cout << response << endl;
            

    } while(0);
    close_socket(sock);
    return 0;
}