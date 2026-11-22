#include <iostream>
#include "../src/SearchSystem.h"
#include "../src/Ollama.h"
#include <vector>
#include <numeric>

using namespace std;

int main() {
    vector<int> threads_nums = {1, 2, 4, 8, 12, 16, 22, 32, 64, 128, 256};
    for (int threads : threads_nums) {
        vector<long> results;
        for (int i = 0; i < 3; ++i) {
            SearchSystem searchSystem;
            cout << "Indexing with " << threads << " threads..." << endl;
            InvertedIndex::StatsData stats = searchSystem.index_dataset(threads);
            cout << threads << " - " << stats.docs_num << " - " << stats.build_time_ms<< endl;
            results.push_back(stats.build_time_ms);
        }
        cout << "Results for " << threads << " threads: " << accumulate(results.begin(), results.end(), 0L) / results.size() << endl;
    }

    return 0;
}