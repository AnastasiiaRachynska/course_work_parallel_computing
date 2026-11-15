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

ApiHandler::ApiHandler(SearchSystem& search_system) : engine(search_system) {}

string extract_body(string& request) {
    size_t position = request.find("\r\n\r\n");
    if (position != string::npos)
        return request.substr(position + 4);
    return "";
}

void ApiHandler::handle_request(int client_socket, string request) {
    if (request.find("GET /api/health") != string::npos) {
        send_response(client_socket, handle_health());
    }
    if (request.find("POST /api/index") != string::npos) {
        send_response(client_socket, handle_index(extract_body(request)));
    }
    if (request.find("POST /api/search") != string::npos) {
        send_response(client_socket, handle_search(extract_body(request)));
    }
    if (request.find("GET /api/stats") != string::npos) {
        send_response(client_socket, handle_stats());
    }
}

void ApiHandler::send_response(int client_socket, json json_response) {
    string response = "HTTP/1.1 200 OK\r\nContent-Type: application/json; charset=UTF-8\r\nContent-Length: " + to_string(json_response.dump().length()) + "\r\nConnection: close\r\n\r\n" + json_response.dump();
    send(client_socket, response.c_str(), response.length(), 0);
}

json ApiHandler::handle_health() {
    json response;
    response["status"] = "OK";
    response["ollama"] = engine.ollama.is_working();
    return response;
}

json ApiHandler::handle_index(string threads) {
    int threads_num = json::parse(threads).value("threads", 2);
    json response;
    InvertedIndex::stats_data raw_results = engine.index_dataset(threads_num);
    response["status"] = "OK";
    response["documents_indexed"] = raw_results.docs_num;
    response["time_ms"] = raw_results.build_time_ms;
    return response;
}

json ApiHandler::handle_search(string query) {
    json request = json::parse(query, nullptr, false);
    
    if (request.is_discarded()) return json{{"error", "Invalid JSON"}};
    
    if (!request.is_object() || !request.contains("query") || !request["query"].is_string())
        return {{"error", "Field 'query' must be a string"}};

    return engine.search(request["query"].get<string>());
}

json ApiHandler::handle_stats() {
    InvertedIndex::stats_data stats = engine.get_stats();
    json response;
    response["documents"] = stats.docs_num;
    response["index_build_time_ms"] = stats.build_time_ms;
    response["threads_used"] = stats.threads_num;
    return response;
}