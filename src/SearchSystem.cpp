//
// Created by User on 15.10.2026.
//

#include "SearchSystem.h"
#include <vector>
#include <nlohmann/json.hpp>
#include "Ollama.h"
using json = nlohmann::json;
#include "InvertedIndex.h"

SearchSystem::SearchSystem(Ollama& ollama_instance) : ollama(ollama_instance){}

stats_data SearchSystem::get_stats() {
    return invertedIndex.statics;
}

json SearchSystem::search(string query){}


stats_data SearchSystem::index_dataset(int threads_num) {

}
