#include "SearchSystem.h"
#include <vector>
#include <chrono>
#include <nlohmann/json.hpp>
#include "Ollama.h"
#include <filesystem>
#include <fstream>
#include "InvertedIndex.h"
#include <algorithm>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

using json = nlohmann::json;
using namespace std::filesystem;
using namespace std::chrono;

SearchSystem::SearchSystem(Ollama& ollama_instance) : ollama(ollama_instance){}

InvertedIndex::stats_data SearchSystem::get_stats() {
    shared_lock<shared_mutex> lock(mtx);
    return invertedIndex.statics;
}


bool create_snippet(string doc_id, const unordered_set<string>& terms, string& snippet) {
    bool found = false;
    snippet = "";
    ifstream file("data/train/" + doc_id + ".txt");

    if (!file.is_open()) return found;
    string word;

    while (file >> word) {
        if (!found && terms.count(standardize_word(word))) found = true;
        
        if (found){
            if(!snippet.empty()) snippet += " ";
            size_t remaining = 200 - snippet.size();
            snippet += word.substr(0, remaining);

            if (snippet.size() >= 200) break;
        }
    }
    if(file.bad()) found = false;
    return found;
}


json SearchSystem::search(string query){
    auto start_time = steady_clock::now();
    unordered_set<string> terms;
    string term;
    stringstream query_stream(query);

    while (query_stream >> term) {
        if (standardize_word(term) != "") terms.insert(standardize_word(term));
    }
    if(terms.empty()) return json{{"error", "No valid search terms"}};

    vector<string> expanded_terms = ollama.expand_query(query);
    if (expanded_terms.empty()) {
        return json{{"error", "No expanded search terms"}};
    }
    for (const string& term : expanded_terms) {
        stringstream ss(term);
        string sub_term;
        while (ss >> sub_term) {
            if (standardize_word(sub_term) != "") terms.insert(standardize_word(sub_term));
        }
    }

    unordered_map<string, int> scores;
    json results = json::array();

    {
        shared_lock<shared_mutex> lock(mtx);
        for (const string& term : terms) {
            vector<string> docs = invertedIndex.find_documents(term);

            for (const string& doc_id : docs) scores[doc_id]++;
        }
        
        vector<pair<string, int>> sorted_results(scores.begin(), scores.end());
        sort(sorted_results.begin(), sorted_results.end(), [](const auto& a, const auto& b) {
                 return a.second > b.second;
        });

        for (const auto& result : sorted_results) {
            string doc_id = result.first;
            string snippet;

            if (!create_snippet(doc_id, terms, snippet))
                return {{"error", "Cannot create document snippet"}};

            double score = static_cast<double>(result.second) / terms.size();

            results.push_back({{"doc_id", doc_id}, {"score", score},{"snippet", snippet}});
        }
    }
    auto end_time = steady_clock::now();
    auto search_time_ms = duration_cast<milliseconds>(end_time - start_time).count();

    return {{"results", results}, {"time_ms", search_time_ms}};
}


InvertedIndex::stats_data SearchSystem::index_dataset(int threads_num) {
    unique_lock<shared_mutex> lock(mtx);

    auto start_time = steady_clock::now();
    atomic<int> docs_indexed{0};
    
    if(threads_num > 0)
    {
        ThreadPool index_pool(threads_num);
        vector<path> files_to_index;
        invertedIndex.clear();

        for (const auto& entry : directory_iterator("data/train")) {
            if (entry.is_regular_file() && entry.path().extension() == ".txt") {
                files_to_index.push_back(entry.path());
            }
        }

        for (size_t i = 0; i < files_to_index.size(); i++) {
            const auto& file_path = files_to_index[i];
            string doc_id = file_path.stem().string();

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
