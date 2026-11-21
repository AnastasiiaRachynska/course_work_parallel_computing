#ifndef COURSE_WORK_PARALLEL_COMPUTING_APIHANDLER_H
#define COURSE_WORK_PARALLEL_COMPUTING_APIHANDLER_H

#include "SearchSystem.h"
using json = nlohmann::json;
using namespace std;


class ApiHandler {
public:
    SearchSystem& engine;
    string status = "200 OK";

    ApiHandler(SearchSystem& engine);

    void handle_request(int client_socket, string request);

    void send_response(int client_socket, json json_response);

    json handle_health();

    json handle_index(string threads);

    json handle_search(string query);

    json handle_stats();


};


#endif
