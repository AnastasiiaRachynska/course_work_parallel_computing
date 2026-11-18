#include "InvertedIndex.h"
#include <string>
#include <unordered_set>
using namespace std;



string standardize_word(const string& raw_word) {
    string result = "";
    for (unsigned char c : raw_word)
        if (isalnum(c)) result += static_cast<char>(tolower(c));
    return result;
}

InvertedIndex::InvertedIndex(): index(16384), index_mutexes(128) {}

void InvertedIndex::clear() {
    for (auto& bucket : index) bucket.clear();
    statics.docs_num = 0;
    statics.build_time_ms = 0;
    statics.threads_num = 0;
}

void InvertedIndex::add_document(const unordered_set<string>& terms, string doc_id){

    for (const string& term : terms) {

        hash<string> hasher;
        size_t bucket_index = hasher(term) % index.size();
        size_t buckets_per_mutex = index.size() / index_mutexes.size();
        size_t mutex_index = bucket_index / buckets_per_mutex; 

        lock_guard<mutex> lock(index_mutexes[mutex_index]);
        bool found = false;
        for (Raw& raw : index[bucket_index]) {
            if (raw.term == term) {
                raw.doc_ids.push_back(doc_id);
                found = true;
                break;
            }
        }

        if (!found) index[bucket_index].push_back({term, {doc_id}});
    }
}


vector<string> InvertedIndex::find_documents(string term) {
    term = standardize_word(term);
    if (term == "") return {};

    hash<string> hasher;
    size_t bucket_index = hasher(term) % index.size();
    size_t buckets_per_mutex = index.size() / index_mutexes.size();
    size_t mutex_index = bucket_index / buckets_per_mutex;
    
    lock_guard<mutex> lock(index_mutexes[mutex_index]);

    for (Raw& raw : index[bucket_index])
        if (raw.term == term) return raw.doc_ids;

    return {};
}
    