
#ifndef COURSE_WORK_PARALLEL_COMPUTING_OLLAMA_H
#define COURSE_WORK_PARALLEL_COMPUTING_OLLAMA_H
#include <string>
#include <mutex>
#include <vector>
using namespace std;

class Ollama {
public:
    string api_url;
    string model_name = "llama3";
    mutex gate;

    vector<string> expand_query(string query);

    bool is_working();

}; // Ollama

#endif //COURSE_WORK_PARALLEL_COMPUTING_OLLAMA_H
