#include <iostream>
#include "D:\GIT\WEB2.0\course_work_parallel_computing\src\SearchSystem.h"
#include "D:\GIT\WEB2.0\course_work_parallel_computing\src\Ollama.h"
#include <vector>
#include <thread>

using namespace std;

int main() {
    Ollama ollama;
    SearchSystem searchSystem(ollama);
    vector<int> threads_nums = {1, 2, 4, 8};
    for (int threads : threads_nums) {
        InvertedIndex::stats_data stats = searchSystem.index_dataset(threads);
        cout << threads << " - " << stats.docs_num << " - " << stats.build_time_ms<< endl;
    }
    return 0;
}