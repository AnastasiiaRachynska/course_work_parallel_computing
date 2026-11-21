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
#include <locale>
#include <iostream>
#include <atomic>
#include <cctype>

using json = nlohmann::json;
using namespace std::filesystem;
using namespace std::chrono;

InvertedIndex::stats_data SearchSystem::get_stats() {
    shared_lock<shared_mutex> lock(mtx);
    return invertedIndex.statics;
}


bool create_snippet(const string& doc_id, const unordered_set<string>& terms, string& snippet) {
    snippet = "";

    ifstream file("data/train/" + doc_id + ".txt");
    if (!file.is_open())return false;

    string word;
    bool found = false;
    while (file >> word) {
        if (!found && terms.count(standardize_word(word))) found = true;

        if (found) {
            if (!snippet.empty()) snippet += " ";

            size_t remaining = 200 - snippet.size();
            snippet += word.substr(0, remaining);

            if (snippet.size() >= 200)break;
        }
    }

    return found && !file.bad();
}

json SearchSystem::search(string query){
    auto start_time = steady_clock::now();
    unordered_set<string> terms;

    if(query.empty()) return json{{"error", "No valid search terms"}};

    vector<string> expanded_terms = ollama.expand_query(query);
    
    if (expanded_terms.empty()) return json{{"error", "No expanded search terms"}};
    
    for (const string& term : expanded_terms) {
        stringstream ss(term);
        string sub_term;
        while (ss >> sub_term)
            if (standardize_word(sub_term) != "") terms.insert(standardize_word(sub_term));
    }

    unordered_map<string, int> scores;
    json results = json::array();

    {
        shared_lock<shared_mutex> lock(mtx);

        for (const string& term : terms) {
            vector<string> docs = invertedIndex.find_documents(term);
            for (const string& doc_id : docs)scores[doc_id]++;
        }

        vector<pair<string, int>> sorted_results(scores.begin(), scores.end());
        sort(sorted_results.begin(), sorted_results.end(), [](const auto& a, const auto& b) {
            return a.second > b.second;});

        for (const auto& result : sorted_results) {
            const string& doc_id = result.first;
            string snippet;

            if (!create_snippet(doc_id, terms, snippet))
                return {{"error", "Cannot create document snippet"}};

            double score = static_cast<double>(result.second) / terms.size();

            results.push_back({{"doc_id", doc_id},{"score", score},{"snippet", snippet}});
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
    atomic<bool> index_failed{false};
    
    if(threads_num > 0)
    {
        ThreadPool index_pool(threads_num);
        vector<path> files_to_index;
        invertedIndex.clear();
        invertedIndex.statics.success = true;

        for (const auto& entry : directory_iterator("data/train")) {
            if (entry.is_regular_file() && entry.path().extension() == ".txt")
                files_to_index.push_back(entry.path());
        }


        for (const auto& file_path : files_to_index) {
            string doc_id = file_path.stem().string();

            index_pool.add_task([this, file_path, doc_id, &docs_indexed, &index_failed] {
                unordered_set<string> terms;

                ifstream file(file_path);
                string line, word;

                while (getline(file, line)) {

                    for (unsigned char c : line) {
                        if (isspace(c)) {
                            if (!word.empty())terms.insert(word);
                            word.clear();
                        } else if (isalnum(c))
                            word += static_cast<char>(tolower(c));
                    }

                    if (!word.empty()) {
                        terms.insert(word);
                        word.clear();
                    }

                }

                if (!file.eof() || file.bad()) {
                    cout << "Cannot read file: " << file_path << endl;
                    index_failed = true;
                    return;
                }

                invertedIndex.add_document(terms, doc_id);
                docs_indexed++;
            });
        }
        index_pool.wait_all();
        if (index_failed) {
            invertedIndex.clear();
            invertedIndex.statics.success = false;
            return invertedIndex.statics;
        }
    }

    auto end_time = steady_clock::now();
    invertedIndex.statics.build_time_ms = duration_cast<milliseconds>(end_time - start_time).count();
    invertedIndex.statics.threads_num = threads_num;
    invertedIndex.statics.docs_num = docs_indexed;
    return invertedIndex.statics;
}


json SearchSystem::search_stream(string query, ix::WebSocket& client){
    auto start_time = steady_clock::now();
    unordered_set<string> terms;

    if(query.empty()) return json{{"error", "No valid search terms"}};

    vector<string> expanded_terms = ollama.expand_query(query);

    if (expanded_terms.empty()) return json{{"error", "No expanded search terms"}};

    json message = {{"stage", "query_expansion"}, 
        {"data", {{"original", query}, {"expanded_terms", expanded_terms}}}};
    auto send_result = client.send(message.dump());

    if(!send_result.success)
        return json{{"error", "Failed to send query expansion message"}};

    for (const string& term : expanded_terms) {
        stringstream ss(term);
        string sub_term;
        while (ss >> sub_term)
            if (standardize_word(sub_term) != "") terms.insert(standardize_word(sub_term));
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
            return a.second > b.second;});

        for (const auto& result : sorted_results) {
            const string& doc_id = result.first;
            string snippet;

            if (!create_snippet(doc_id, terms, snippet))
                return {{"error", "Cannot create document snippet"}};

            double score = static_cast<double>(result.second) / terms.size();

            results.push_back({{"doc_id", doc_id},{"score", score},{"snippet", snippet}});
        }
    }

 
    auto end_time = steady_clock::now();
    auto search_time_ms = duration_cast<milliseconds>(end_time - start_time).count();
    
    message = {{"stage", "keyword_search"}, 
        {"data", {{"results", results}, {"time_ms", search_time_ms}}}};
    auto send_result2 = client.send(message.dump());
    
    if(!send_result2.success)
        return json{{"error", "Failed to send keyword search message"}};
    
    message = {{"stage", "done"}};
    auto send_result3 = client.send(message.dump());
    if(!send_result3.success) return json{{"error", "Failed to send done message"}};
    
    return {{"results", results}, {"time_ms", search_time_ms}};
}



// auto ollama_start = steady_clock::now();
    // vector<string> expanded_terms = ollama.expand_query(query);
    // auto ollama_end = steady_clock::now();
    // auto ollama_time_ms = duration_cast<milliseconds>(ollama_end - ollama_start).count();
    // std::cout << "Ollama time: " << ollama_time_ms << " ms" << std::endl;

  // auto keyword_start = steady_clock::now();

// auto keyword_end = steady_clock::now();
    // auto keyword_time_ms = duration_cast<milliseconds>(keyword_end - keyword_start).count();
    // std::cout << "Keyword search time: " << keyword_time_ms << " ms" << std::endl;