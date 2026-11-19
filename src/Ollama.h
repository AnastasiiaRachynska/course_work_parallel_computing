
#ifndef COURSE_WORK_PARALLEL_COMPUTING_OLLAMA_H
#define COURSE_WORK_PARALLEL_COMPUTING_OLLAMA_H
#include <string>
#include <mutex>
#include <vector>
using namespace std;

class Ollama {
private:
    string send_request(std::string prompt);
    vector<string> process_answer(std::string answer);
public:
    string model_name = "llama3";
    mutex mtx;
    string api_url = "ollama";

    vector<string> expand_query(std::string query);

    bool is_working();

};

#endif
