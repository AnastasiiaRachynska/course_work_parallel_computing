#include "ApiHandler.h"
#include <string>



ApiHandler::ApiHandler(SearchSystem& search_system) : engine(search_system) {}

string extract_body(string& request) {
    size_t position = request.find("\r\n\r\n");
    if (position != string::npos)
        return request.substr(position + 4);
    return "";
}

void ApiHandler::handle_request(int client_socket, string request) {
    if (request.find("OPTIONS") == 0) {
        send_response(client_socket, json::object());
        return;
    }
    
    if (request.find("GET /api/health") != string::npos)
        send_response(client_socket, handle_health());
    
    else if (request.find("POST /api/index") != string::npos)
        send_response(client_socket, handle_index(extract_body(request)));
    
    else if (request.find("POST /api/search") != string::npos) 
        send_response(client_socket, handle_search(extract_body(request)));
    
    else if (request.find("GET /api/stats") != string::npos) 
        send_response(client_socket, handle_stats());
    
}

void ApiHandler::send_response(int client_socket, json json_response) {
    string body = json_response.dump(-1, ' ', false, json::error_handler_t::replace);

    string response = "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json; charset=UTF-8\r\n"
        "Access-Control-Allow-Origin: http://localhost:8000\r\n"
        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Content-Length: " + to_string(body.length()) + 
        "\r\nConnection: close\r\n\r\n" + body;

    send(client_socket, response.c_str(), response.length(), 0);
}

json ApiHandler::handle_health() {
    return {{"status", "OK"}, {"ollama", engine.ollama.is_working()}};
}

json ApiHandler::handle_index(string threads) {
    int threads_num = json::parse(threads).value("threads", 8);
   
    InvertedIndex::stats_data raw_results = engine.index_dataset(threads_num);
    if (!raw_results.success)
        return {{"error", "Failed to read a document during indexing"}};

    return {{"status", "OK"}, {"documents_indexed", raw_results.docs_num}, {"time_ms", raw_results.build_time_ms}};
}

json ApiHandler::handle_search(string query) {
    json request = json::parse(query, nullptr, false);
    
    if (request.is_discarded()) return json{{"error", "Invalid JSON"}};
    
    if (!request.is_object() || !request.contains("query") || !request["query"].is_string())
        return {{"error", "Field 'query' must be a string"}};

    if (!request.contains("mode") || request["mode"] != "keyword")
        return {{"error", "Field 'mode' must be a keyword"}};

    string search_query = request["query"];
    return engine.search(search_query);
}

json ApiHandler::handle_stats() {
    InvertedIndex::stats_data stats = engine.get_stats();

    return {{"status", "OK"}, {"documents", stats.docs_num}, {"index_build_time_ms", stats.build_time_ms}, {"threads_used", stats.threads_num}};
}