#include "Ollama.h"
#include <cstring>
#include <mutex>
#include <iostream>
#include <sstream>
#include <httplib.h>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#endif

inline void close_socket_ollama(int sock) {
#ifdef _WIN32
    closesocket(sock);
#else
    close(sock);
#endif
}

using namespace std;

bool Ollama::is_working() {
    httplib::Client cli(api_url, 11434);
    cli.set_connection_timeout(5, 0);
    cli.set_read_timeout(5, 0);
    auto response = cli.Get("/api/version");
    if (!response || response->status != 200) return false;
    return true;
}

string Ollama::send_request(string prompt) {
    httplib::Client cli(api_url, 11434);
    cli.set_connection_timeout(5, 0);
    cli.set_read_timeout(90, 0);
    json body = {{"model", model_name}, {"prompt", prompt}, {"stream", false}};

    auto response = cli.Post("/api/generate", body.dump(), "application/json");
    if (!response || response->status != 200) {
        cerr << "Error: Failed to get response from Ollama API" << endl;
        return "";
    }
    return response->body;
} 

vector<string> Ollama::process_answer(string answer){
    vector<string> terms;
    auto result = json::parse(answer, nullptr, false);

    if(result.is_discarded() || !result.contains("response") || !result["response"].is_string())
        return terms;

    string response = result["response"].get<string>();
    stringstream stream(response);
    string term;

    while(getline(stream, term, ',')) {
        size_t start = term.find_first_not_of(" \t\n\r");
        size_t end = term.find_last_not_of(" \t\n\r");
        
        if (start != string::npos)
            terms.push_back(term.substr(start, end - start + 1));
    }

    if(terms.empty()) 
        cerr << "Warning: No terms extracted from response: " << response << endl;

    return terms;
}

vector<string> Ollama::expand_query(string query){
    lock_guard<mutex> lock(mtx);
    string prompt =
    "Extract the main concepts from the search query and add relevant synonyms. "
    "Return only English search terms separated by commas on one line. "
    "Do not include explanations, headings, numbering, or quotation marks. "
    "Example output: stars, astronomy, cosmos, galaxies\n"
    "Search query: " + query;

    string response = send_request(prompt);
    if (response.empty()) {
        cerr << "Warning: Empty response for query: " << query << endl;
        return {};
    }
    return process_answer(response);
}