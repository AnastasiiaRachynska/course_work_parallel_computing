#include <iostream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "../src/SearchSystem.h"
using namespace std;


int main() {
    SearchSystem searchSystem;

    InvertedIndex::StatsData stats = searchSystem.index_dataset(2);
    cout << "Indexed " << stats.docs_num << " documents in " << stats.build_time_ms << " ms using " << stats.threads_num << " threads." << std::endl;
    
    searchSystem.invertedIndex.index.print(cout);
    return 0;
}