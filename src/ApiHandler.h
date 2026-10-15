#ifndef COURSE_WORK_PARALLEL_COMPUTING_APIHANDLER_H
#define COURSE_WORK_PARALLEL_COMPUTING_APIHANDLER_H

#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
using json = nlohmann::json;
using namespace std;


class ApiHandler {
public:
    //engine : SearchSystem&
    //ollama : Ollama&
    void handle_request(int client_socket, string request);

    void send_response(int client_socket, json json_response);

    json handle_health();


};


#endif
