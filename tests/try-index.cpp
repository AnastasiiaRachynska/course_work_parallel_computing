#include <iostream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "../src/SearchSystem.h"
using namespace std;


int main() {
    SearchSystem searchSystem;

    InvertedIndex::stats_data stats = searchSystem.index_dataset(4);
    cout << "Indexed " << stats.docs_num << " documents in " << stats.build_time_ms << " ms using " << stats.threads_num << " threads." << std::endl;
    
    // cout << searchSystem.invertedIndex.index << endl;

    for (const auto& bucket : searchSystem.invertedIndex.index) {
        for (const Raw& entry : bucket){
            cout << entry.term << ": ";
            for (const auto& doc_id : entry.doc_ids) cout << doc_id << " ";
            cout << endl;
        }
    }
    return 0;
}