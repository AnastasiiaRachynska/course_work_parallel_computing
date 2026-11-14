#include "SearchSystem.h"
#include <vector>
#include <chrono>
#include <nlohmann/json.hpp>
#include "Ollama.h"
#include <filesystem>
#include <fstream>
#include "InvertedIndex.h"

using json = nlohmann::json;
using namespace std::filesystem;
using namespace std::chrono;

SearchSystem::SearchSystem(Ollama& ollama_instance) : ollama(ollama_instance){}

InvertedIndex::stats_data SearchSystem::get_stats() {
    return invertedIndex.statics;
}

json SearchSystem::search(string query){}


InvertedIndex::stats_data SearchSystem::index_dataset(int threads_num) {
    auto start_time = steady_clock::now();
    atomic<int> docs_indexed{0};
    
    if(threads_num > 0)
    {
        ThreadPool index_pool(threads_num);
        vector<path> files_to_index;
        invertedIndex.clear();

        for (const auto& entry : directory_iterator("data/train")) {
            if (entry.is_regular_file()) {
                files_to_index.push_back(entry.path());
            }
        }

        for (size_t i = 0; i < files_to_index.size(); i++) {
            const auto& file_path = files_to_index[i];
            int doc_id = i;

            index_pool.addTask([this, file_path, doc_id, &docs_indexed] {
                vector<string> terms ;
                string term;
                ifstream file(file_path);

                while (file >> term) terms.push_back(term);

                invertedIndex.add_document(terms, doc_id);
                docs_indexed++;
            });
        }
        index_pool.wait_all();
    }

    auto end_time = steady_clock::now();
    invertedIndex.statics.build_time_ms = duration_cast<milliseconds>(end_time - start_time).count();
    invertedIndex.statics.threads_num = threads_num;
    invertedIndex.statics.docs_num = docs_indexed;
    return invertedIndex.statics;
}
