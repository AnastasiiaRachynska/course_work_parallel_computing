
#ifndef COURSE_WORK_PARALLEL_COMPUTING_OLLAMA_H
#define COURSE_WORK_PARALLEL_COMPUTING_OLLAMA_H
#include <string>
#include <mutex>
#include <vector>
// using namespace std;

class Ollama {
private:
    std::string send_request(std::string prompt);
    std::vector<std::string> process_answer(std::string answer);
public:
    std::string api_url;
    std::string model_name = "llama3";
    std::mutex mtx;

    std::vector<std::string> expand_query(std::string query);

    bool is_working();

}; // Ollama

#endif //COURSE_WORK_PARALLEL_COMPUTING_OLLAMA_H
