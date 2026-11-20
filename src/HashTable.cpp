#include "HashTable.h"

HashTable::HashTable(): table(16384), mutexes(128) {}

void HashTable::clear() {
    for (auto& bucket : table) bucket.clear();
}

void HashTable::insert(const string& term, const string& doc_id){

    hash<string> hasher;
    size_t bucket_index = hasher(term) % table.size();
    size_t buckets_per_mutex = table.size() / mutexes.size();
    size_t mutex_index = bucket_index / buckets_per_mutex; 

    lock_guard<mutex> lock(mutexes[mutex_index]);
    for (Row& raw : table[bucket_index]) {
        if (raw.term == term) {
            raw.doc_ids.push_back(doc_id);
            return;
        }
    }

    table[bucket_index].push_back({term, {doc_id}});
}


vector<string> HashTable::find(const string& term) {
    hash<string> hasher;
    size_t bucket_index = hasher(term) % table.size();
    size_t buckets_per_mutex = table.size() / mutexes.size();
    size_t mutex_index = bucket_index / buckets_per_mutex;
    
    lock_guard<mutex> lock(mutexes[mutex_index]);

    for (const Row& row : table[bucket_index])
        if (row.term == term) return row.doc_ids;

    return {};
}

void HashTable::print(ostream& output) {
    size_t buckets_per_mutex = table.size() / mutexes.size();

    for (size_t i = 0; i < table.size(); i++) {
        lock_guard<mutex> lock(mutexes[i / buckets_per_mutex]);

        for (const Row& row : table[i]) {
            output << row.term << ": ";
            for (const string& doc_id : row.doc_ids)
                output << doc_id << " ";
            output << endl;
        }
    }
}