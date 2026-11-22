#ifndef COURSE_WORK_PARALLEL_COMPUTING_SEARCHSYSTEM_H
#define COURSE_WORK_PARALLEL_COMPUTING_SEARCHSYSTEM_H

#include "InvertedIndex.h"
#include "Ollama.h"
#include <shared_mutex>
#include <nlohmann/json.hpp>
#include "ThreadPool.h"
#include "ixwebsocket/IXWebSocket.h"

using json = nlohmann::json;
using namespace std;

class SearchSystem {
private:
    shared_mutex mtx;
public:

    Ollama ollama;
    InvertedIndex invertedIndex;
    InvertedIndex::StatsData statics;

    json search(string query);
    InvertedIndex::StatsData index_dataset(int threads_num);
    InvertedIndex::StatsData get_stats();
    json search_stream(string query, ix::WebSocket& client);

};

#endif