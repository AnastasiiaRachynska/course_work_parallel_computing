#include <iostream>
#include "../src/SearchSystem.h"
#include "../src/Ollama.h"
#include <vector>

using namespace std;

int main() {
    SearchSystem searchSystem;
    vector<int> threads_nums = {8, 16, 32, 64};
    for (int threads : threads_nums) {
        cout << "Indexing with " << threads << " threads..." << endl;
        InvertedIndex::stats_data stats = searchSystem.index_dataset(threads);
        cout << threads << " - " << stats.docs_num << " - " << stats.build_time_ms<< endl;
    }
    return 0;
}