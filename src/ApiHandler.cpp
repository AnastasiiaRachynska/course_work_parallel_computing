#include "ApiHandler.h"
#include <string>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
#endif

void ApiHandler::handle_request(int client_socket, string request) {
    if (request.find("GET /api/health") != string::npos) {
        send_response(client_socket, handle_health());
    }
}

void ApiHandler::send_response(int client_socket, json json_response) {
    string response = "HTTP/1.1 200 OK\r\nContent-Type: application/json; charset=UTF-8\r\nContent-Length: " + to_string(json_response.dump().length()) + "\r\nConnection: close\r\n\r\n" + json_response.dump();
    send(client_socket, response.c_str(), response.length(), 0);
}

json ApiHandler::handle_health() {
    json response;
    response["status"] = "OK";
    response["ollama"] = true;
    return response;
}