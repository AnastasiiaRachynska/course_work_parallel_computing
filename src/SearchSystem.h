//
// Created by User on 15.10.2026.
//

#ifndef COURSE_WORK_PARALLEL_COMPUTING_SEARCHSYSTEM_H
#define COURSE_WORK_PARALLEL_COMPUTING_SEARCHSYSTEM_H

#include "InvertedIndex.h"
#include "Ollama.h"
#include <shared_mutex>
#include <nlohmann/json.hpp>
#include "ThreadPool.h"

using json = nlohmann::json;
using namespace std;

class SearchSystem {
private:
    shared_mutex mtx;
public:

    Ollama& ollama;
    InvertedIndex invertedIndex;
    InvertedIndex::stats_data statics;
    

    SearchSystem(Ollama& ollama);


    json search(string query);
    InvertedIndex::stats_data index_dataset(int threads_num);
    InvertedIndex::stats_data get_stats();

}; // SearchSystem

#endif //COURSE_WORK_PARALLEL_COMPUTING_SEARCHSYSTEM_H