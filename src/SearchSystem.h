//
// Created by User on 15.10.2026.
//

#ifndef COURSE_WORK_PARALLEL_COMPUTING_SEARCHSYSTEM_H
#define COURSE_WORK_PARALLEL_COMPUTING_SEARCHSYSTEM_H

#include "InvertedIndex.h"
#include "Ollama.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;
using namespace std;

class SearchSystem {
public:

    Ollama& ollama;
    InvertedIndex invertedIndex;
    SearchSystem(Ollama& ollama);

    json search(string query);
    stats_data index_dataset(int threads_num);
    stats_data get_stats();

}; // SearchSystem

#endif //COURSE_WORK_PARALLEL_COMPUTING_SEARCHSYSTEM_H